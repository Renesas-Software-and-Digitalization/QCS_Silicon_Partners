#!/usr/bin/env python

import os
from os.path import join as path_join
from os.path import exists as path_exists
from os.path import sep as path_sep
from glob import glob
import sys
from pathlib import Path
from functools import reduce
from iterfzf import iterfzf
import re
from ruamel.yaml import YAML

PT = '^[\s]{4}'
T_FOLDER = '.xml_tests'

PATHS = {
    # kits, os
    'MCU_KIT': path_join('**', 'mcu_kits', '%s', 'templates', '%s', T_FOLDER, '%s.y*ml'),
    # kits, mod type, func type, mod name, os, app (option)
    'RENESAS_COMP': path_join('**', 'mcu_kits', '%s', '%s', '%s', '%s', T_FOLDER, '%s.y*ml'),
    # mod type, func type, mod name, kit, os
    'PARTNER_COMP': path_join('**', '%s', '%s', '%s', '%s', '%s', T_FOLDER, '%s.y*ml')
}

yaml = YAML(typ='rt')


@yaml.register_class
class Flatten(list):
    yaml_tag = '!flatten'

    def __init__(self, *args):
        self.items = args

    @classmethod
    def from_yaml(cls, constructor, node):
        x = cls(*constructor.construct_sequence(node, deep=True))
        return x

    @classmethod
    def to_yaml(cls, representer, node):
        return representer.represent_sequence(cls.yaml_tag, node)

    def __iter__(self):
        for item in self.items:
            if isinstance(item, ToFlatten):
                for nested_item in item:
                    yield nested_item
            else:
                yield item


@yaml.register_class
class ToFlatten(list):
    yaml_tag = '!toflatten'

    @classmethod
    def from_yaml(cls, constructor, node):
        x = cls(constructor.construct_sequence(node, deep=True))
        return x

    @classmethod
    def to_yaml(cls, representer, node):
        return representer.represent_sequence(cls.yaml_tag, node)


def strip_python_tags(s):
    result = []
    for line in s.splitlines():
        idx = line.find("!flatten")
        if idx > -1:
            line = line[:idx]
        result.append(line)
    return '\n'.join(result)


def _lookup_templates(name, base_path, files, lookups, multi=False):
    resolved = None
    if len(files) > 0:
        resolved = list(filter(
            path_exists,
            [_f if _f.startswith(path_sep) else path_join(base_path, _f) for _f in files.split(',')]
        ))
        resolved = resolved if multi or len(resolved) == 0 else resolved[0]

    if not resolved:
        resolved = iterfzf(
            reduce(
                lambda reduced, item: reduced + glob(path_join(base_path, item), recursive=True),
                lookups if isinstance(lookups, list) else [lookups],
                []
            ),
            prompt="Please select %s: " % name.lower(),
            multi=multi
        )
    return resolved


def _combine_modules(mcu_file, sensor_files, connectivity_files, debug=False):
    if debug:
        print('\t\t[DEBUG] loading all selected module files...')

    pj_template = ''
    for m_type, m_files in dict({'sensors': sensor_files, 'connectivity': connectivity_files}).items():
        pj_template += '%s: &%s !toflatten\n' % (m_type, m_type)
        for m_file in m_files:
            with open(m_file) as mf:
                pj_template += ''.join(
                    list(filter(
                        lambda l: len(l.strip()) > 0,
                        [re.sub(PT, '', l) for l in mf.readlines()]
                    ))
                )

    with open(mcu_file) as mf:
        pj_template += '\nmcu_kits:\n'
        pj_template += reduce(
            lambda merged, line: merged + (
                line.strip('\n') + ' !flatten\n' if re.match('[\s]+sequence:', line) else line) + '',
            mf.readlines()
        )

    if debug:
        print('\t\t[DEBUG] merged project template anchors:\n\n----\n%s\n----\n\n' % pj_template)

    pj_yaml = yaml.load(pj_template)
    if 'mcu_kits' in pj_yaml:
        return pj_yaml['mcu_kits']
    else:
        print('There seems something wrong in the MCU templates. Skip the process.')
        return []


def _ensure_outfile(outfile, templates, outdir='.'):
    _out = path_join(
        outdir,
        ((templates[0] if isinstance(templates, list) else templates)['name']) + '.yml'
    ) if len(outfile) == 0 else outfile
    Path(_out).parent.absolute().mkdir(parents=True, exist_ok=True)
    return _out


def _resolve_mcu(kit, platform):
    found = glob(PATHS['MCU_KIT'] % (kit, platform, 'project_template'))
    return found[0] if len(found) else None


def _resolve_template(typ, kit, platform, component):
    comp_elms = component.split(path_sep)  # split module and app, if specified
    comp_path = path_join(
        comp_elms[0], platform, *(comp_elms[1:] if len(comp_elms) > 1 else [])
    )  # format as `module/os/app`
    comp_file = comp_elms[-1]
    # resolve in Renesas folder first
    found = glob(PATHS['RENESAS_COMP'] % (kit, typ, '**', comp_path, comp_file + '*'))
    # resolve in other folders otherwise
    found = found if len(found) else \
        glob(PATHS['PARTNER_COMP'] % (typ, '**', component, kit, platform, comp_file + '*'))
    if len(found):
        temp_path = found[0]
        temp_type = temp_path.split(typ)[-1].split(path_sep)[1].upper()
        return temp_type, temp_path
    else:
        return None, None


if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(
        description="Create YML project by combining existing module templates.",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter
    )
    parser.add_argument(
        "-b",
        dest="base",
        help="The base path to load the module template files",
        metavar="BASE",
        required=False,
        default=os.environ.get('CI_PROJECT_DIR', '.')
    )
    parser.add_argument(
        "-p",
        dest="project",
        help="The project template of the selected MCU/Pmods/Onboard_peripheral combination; when set all other YAML relative arguments will be ignored",
        metavar="PROJECT",
        required=False,
        default=os.environ.get('ENV_YML_PROJECT', '.')
    )
    parser.add_argument(
        "-m",
        dest="mcu",
        help="The MCU template to combine with; leave empty to select from project path",
        metavar="MCU",
        required=False,
        default=os.environ.get('ENV_YML_MCU', '')
    )
    parser.add_argument(
        "-s",
        dest="sensors",
        help="The Sensor templates to combine with, seperated by comma if multiple; leave empty to select from project path",
        metavar="SENSORS",
        required=False,
        default=os.environ.get('ENV_YML_SENSORS', '')
    )
    parser.add_argument(
        "-c",
        dest="connectivity",
        help="The Connectivity template to combine with; leave empty to select from project path",
        metavar="CONNECTIVITY",
        required=False,
        default=os.environ.get('ENV_YML_CONNECTIVITY', '')
    )
    parser.add_argument(
        "-o",
        dest="output",
        help="Where to store output YAML file",
        metavar="OUTPUT",
        required=False,
        default=''
    )
    parser.add_argument(
        "-d",
        dest="debug",
        help="Debugging logs",
        required=False,
        action='store_true',
        default=os.environ.get('DEBUG', '0') == '1'
    )
    arguments = parser.parse_args()

    if arguments.debug:
        print('\n'
              'Initiated with following settings:\n'
              ' - BASE:         %s\n'
              ' - PROJECT:      %s\n'
              ' - MCU:          %s\n'
              ' - SENSORS:      %s\n'
              ' - CONNECTIVITY: %s\n'
              ' - OUTPUT:       %s\n'
              '\n' % (
                  arguments.base,
                  arguments.project,
                  arguments.mcu,
                  arguments.sensors,
                  arguments.connectivity,
                  arguments.output
              ))

    mcu = None
    sensors = None
    connectivity = None

    if arguments.project:
        if arguments.debug:
            print('    [DEBUG] project definition provided; to resolve the corresponding templates...')
        _stream = None
        if path_exists(arguments.project):
            _stream = open(arguments.project, 'r')
        elif path_exists(path_join(arguments.base, arguments.project)):
            _stream = open(path_join(arguments.base, arguments.project), 'r')
        else:
            _stream = arguments.project
        project = yaml.load(_stream)
        _kit = project.pop('mcu_kits')
        _os = project.pop('os')
        mcu = _resolve_mcu(_kit, _os)
        if arguments.debug:
            print('    [DEBUG]    resolved mcu [%s]...' % mcu)
        comps = dict([])
        for comp_name, comp_flist in project.items():
            for f in comp_flist if isinstance(comp_flist, list) else [comp_flist]:
                (otype, temp_file) = _resolve_template(comp_name, _kit, _os, f)
                if arguments.debug:
                    print('    [DEBUG]    resolved %s [%s]...' % (otype, temp_file))
                if not otype or not temp_file:
                    continue
                if not otype in comps.keys():
                    comps[otype] = []
                comps[otype].append(temp_file)
        sensors = comps.get('SENSORS', [])
        connectivity = comps.get('CONNECTIVITY', [])
    else:
        if arguments.debug:
            print('    based on the given templates or to look up from base path...')

        mcu = _lookup_templates('MCU', arguments.base, arguments.mcu, PATHS['MCU_KIT'] % ('**', '**', '*'))
        if not mcu:
            print("\nGiven invalid MCU: [%s]" % mcu)
            exit(-1)

        (kit, _other) = mcu.split(path_sep + 'mcu_kits' + path_sep)[1].split(path_sep + 'templates' + path_sep)
        _os = _other.split(T_FOLDER + path_sep)[0]
        _subs = '**' if _other.startswith('.xml_tests') else path_join('**', _os)
        sensors = _lookup_templates(
            'SENSORS',
            arguments.base,
            arguments.sensors,
            [
                PATHS['RENESAS_COMP'] % (kit, 'pmods', 'sensors', _subs, '*'),
                PATHS['PARTNER_COMP'] % ('pmods', 'sensors', '**', kit, _os, '*'),
            ],
            multi=True
        )
        _subs = _subs if _subs.endswith('**') else path_join(_subs, '**')
        connectivity = _lookup_templates(
            'CONNECTIVITY',
            arguments.base,
            arguments.connectivity,
            [
                PATHS['RENESAS_COMP'] % (kit, 'pmods', 'connectivity', _subs, '*'),
                PATHS['PARTNER_COMP'] % ('pmods', 'connectivity', '**', kit, _os, '*'),
                PATHS['RENESAS_COMP'] % (kit, 'onboard_peripherals', 'connectivity', _subs, '*'),
                PATHS['PARTNER_COMP'] % ('onboard_peripherals', 'connectivity', '**', kit, _os, '*'),
            ],
            multi=True
        )

    if mcu and sensors and connectivity:
        print('\nSelected templates --\n\tMCU: {}\n\tSensors: {}\n\tConnectivity: {}\n'.format(
            mcu,
            '\n\t         '.join(sensors),
            '\n\t              '.join(connectivity),
        ))
    else:
        if not sensors:
            print("\nGiven invalid Sensors: [%s]" % arguments.sensors)
        if not connectivity:
            print("\nGiven invalid Connectivity: [%s]" % arguments.connectivity)
        sys.exit(-1)

    project_templates = _combine_modules(mcu, sensors, connectivity, debug=arguments.debug)

    if len(project_templates) > 0:
        yaml_file = _ensure_outfile(arguments.output, project_templates, outdir=os.path.dirname(mcu))
        print('\nWriting to project template: [{}]...'.format(yaml_file))
        yaml.dump(project_templates, open(yaml_file, 'w'), transform=strip_python_tags)
        print('... done!\n')
        if 'templates' in project_templates:
            print('Multiple project found, creating module list...')
            import create_module_test_list

            create_module_test_list.parse(yaml_file, 'tmp_out.yml', False)
    else:
        print('Project template is not defined correctly, skip the process.')
        exit(-1)
