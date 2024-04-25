#!/usr/bin/env python

import os
from os.path import exists as path_exists
from os.path import join as path_join
from functools import reduce

from ruamel.yaml import YAML
from jinja2 import Environment, FileSystemLoader

yaml = YAML(typ='rt')


def _load_manifest(manifest_path):
    if path_exists(manifest_path):
        with open(manifest_path, 'r') as ym:
            return yaml.load(ym)
    return None


def _load_pmod(base, name):
    return _load_manifest(path_join(base, 'modules', 'pmod', name.split('/')[0], 'manifest.yml'))


def _load_mcu(base, name):
    return _load_manifest(path_join(base, 'mcu_kits', name, 'manifest.yml'))


def generate_yaml(base, write_to, project_list):
    # Set up Jinja2 environment
    env = Environment(loader=FileSystemLoader(base))

    # Load the main template
    template = env.get_template('applications/project_template.yml.j2')

    # Render the template with provided parameters
    rendered_template = template.render(**project_list)

    out_file = path_join(write_to, f"{project_list['app_name']}.yml")  # default output file name
    if '.' in write_to:
        ext = write_to.split('.')[-1]
        if 'YML' == ext.upper() or 'YAML' == ext.upper():
            out_file = write_to

    os.makedirs(os.path.dirname(out_file), exist_ok=True)
    with open(out_file, 'w') as out:
        out.write(rendered_template)

    print(f"YAML file '{out_file}' generated successfully.\n")


def transform_json(base, mcu, platform, pmods, debug=0):
    proj_kit = mcu.upper().replace('_', '-')
    if platform == "freertos":
        proj_os = "FreeRTOS"
        proj_template = "FreeRTOS - Minimal - Static Allocation"
    else:
        proj_os = 'No RTOS'
        proj_template = 'Bare Metal - Minimal'

    mcu_def = _load_mcu(base, mcu)
    output_json = {
        "project": {
            "name": f"{mcu}_{platform}",
            "os": proj_os,
            "template": proj_template,
            "kit": proj_kit
        },
        "mcu_kit": mcu,
        "rtos_name": platform,
        "board": mcu_def['interfaces']
    }
    apps = reduce(
        lambda reduced, elm: [*reduced, elm.split('/')[-1]] if '/' in elm else reduced,
        pmods,
        [],
    )
    for app in apps:
        if debug:
            print(f'\tverifying [{app}] compatibility with {mcu}/{platform}...')
        if not app in mcu_def['compatibility']['applications'].keys() or \
                not platform in mcu_def['compatibility']['applications'][app]:
            print(f'Validation Error: [{app}] is incompatible with {mcu}/{platform}!')
            exit(2)

    _used = []
    for pmod in pmods:
        if debug:
            print(f'\tresolving {pmod}...')
        if '/' in pmod:
            (p_name, p_app) = pmod.split('/')
            output_json['app_name'] = p_app
        else:
            p_name = pmod

        pmod_def = _load_pmod(base, pmod)
        pmod_app_check_def = pmod_def['compatibility']['applications']
        if pmod_def:
            if debug:
                print(f'\tverifying [{app}] compatibility with {p_name}...')
            if not app in pmod_app_check_def.keys() or \
                    not platform in pmod_app_check_def[app]:
                print(f'Validation Error: [{app}] is incompatible with {p_name}!')
                exit(2)

            p_interface = pmod_def['interface_type']

            mcu_pmod_check_def = mcu_def['compatibility']['modules']['pmod']
            _mod_onboard = None
            if not 'exclude' in mcu_pmod_check_def.keys() or \
                    not p_interface in mcu_pmod_check_def['exclude'] or \
                    not p_name in mcu_pmod_check_def['exclude']:
                for name, interfaces in mcu_pmod_check_def.items():
                    if 'exclude' == name:
                        continue
                    if debug:
                        print(f'\t\tchecking {p_interface}/{p_name} with {interfaces}...')
                    if (p_interface in interfaces or p_name in interfaces) and not name in _used:
                        if debug:
                            print(f'\t\t\t... matched')
                        _used.append(name)
                        _mod_onboard = name
                        break

            if _mod_onboard:
                typ = pmod_def.get('type', '')
                if debug:
                    print(f'\t\tappending {typ}:{p_name} to {_mod_onboard}...')
                mod_name = 'sensor_module' if 'sensor' == typ else 'conn_module'
                output_json[mod_name] = {'name': p_name, 'interface': _mod_onboard}
                output_json[mod_name].update(pmod_def)
            else:
                print(f'Validation Error: no more available pmod obn {mcu}/{platform} for [{p_name}]!')
                exit(2)
    if debug:
        print('\n\ttransformed json --\n')
        print(output_json)
        print('\n')
    return output_json


if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(
        description="Project template brewery.",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter
    )
    parser.add_argument(
        "-b",
        dest="base",
        help="Whereabouts of the brewing materials",
        metavar="BASE",
        required=False,
        default=os.environ.get('CI_PROJECT_DIR', '.')
    )
    parser.add_argument(
        "-p",
        dest="project",
        help="The brewing recipe all at once; either a pointer to the file or the content",
        metavar="PROJECT",
        required=False,
        default=os.environ.get('ENV_YML_PROJECT', '.')
    )
    parser.add_argument(
        "-m",
        dest="mcu",
        help="The MCU kit",
        metavar="MCU",
        required=False,
        default=os.environ.get('ENV_YML_MCU', '')
    )
    parser.add_argument(
        "-s",
        dest="sensors",
        help="The Sensor Pmod",
        metavar="SENSORS",
        required=False,
        default=os.environ.get('ENV_YML_SENSORS', '')
    )
    parser.add_argument(
        "-c",
        dest="connectivity",
        help="The Connectivity Pmod",
        metavar="CONNECTIVITY",
        required=False,
        default=os.environ.get('ENV_YML_CONNECTIVITY', '')
    )
    parser.add_argument(
        "-o",
        dest="output",
        help="Where to store the brew result",
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

    base = arguments.base
    mcu = None
    platform = None
    pmods = None
    write_to = arguments.output if arguments.output else path_join(base, 'Build')

    if arguments.project:
        _stream = None
        if path_exists(arguments.project):
            _stream = open(arguments.project, 'r')
        elif path_exists(path_join(arguments.base, arguments.project)):
            _stream = open(path_join(arguments.base, arguments.project), 'r')
        else:
            _stream = arguments.project
        project = yaml.load(_stream)
        mcu = project['mcu_kits']
        platform = project['os']
        pmods = project['pmods']
    else:
        (mcu, platform) = arguments.mcu.split('/')
        pmods = []
        for comps in [arguments.sensors, arguments.connectivity]:
            if ',' in comps:
                pmods.extend(arguments.comps.split(','))
            elif comps:
                pmods.append(comps)

    project_vars = transform_json(base, mcu, platform, pmods, debug=arguments.debug)

    generate_yaml(base, write_to, project_vars)
