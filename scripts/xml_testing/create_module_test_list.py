#!/bin/python3

import yaml
import os
from jinja2 import Template
import xml.etree.ElementTree as ET

# Default toolchains to build.
default_toolchain_list = [
    'gcc',
    'iar',
    'ac6'
]

# This will parse a pack creator XML and return the boards that are created
def get_boards (path):
    pack_data_location = os.path.dirname(path)

    board_list = []

    tree = ET.parse(path)
    root = tree.getroot()
    for packs in root.findall('packs'):
        for pack in packs.findall('pack'):
            # Read board XML to get configuration.xml
            if 'board' in pack.text:
                board_xml_tree = ET.parse(os.path.join(pack_data_location, pack.text))
                board_xml_root = board_xml_tree.getroot()
                board_cfg = board_xml_root.find('.//config/modules/module/cfg_xml_file').text
                # Read board cfg xml to get board name shown in e2studio
                # The path in pack_board_<board>.xml is from root of repo
                board_cfg_tree = ET.parse(os.path.join(os.getcwd(), board_cfg))
                board_cfg_root = board_cfg_tree.getroot()
                board_name = board_cfg_root.find('.//board').get('display')

                # Ignore 'Custom User Board (Any Device)'
                if 'custom user' in board_name.lower():
                    continue

                board_list.append(board_name)

    return board_list

def parse (input, output, tagged):

    pack_data_location = os.path.realpath(os.path.join(os.getcwd(), 'data', 'pack_definition_files'))
    # Read internal_packs.xml for information about what MCUs and boards are internal
    internal_board_list = get_boards(os.path.join(pack_data_location, 'internal_packs.xml'))
    # Default board list. This is used when templates don't have a board list.
    # Read release_packs.xml for information about what MCUs and boards are public
    public_board_list = get_boards(os.path.join(pack_data_location, 'release_packs.xml'))

    # Get list of MCUs that IAR supports
    iar_supported_mcus = []
    # First get IAR install path
    tree = ET.parse(os.path.realpath(os.path.join(os.getcwd(), 'test_files', 'shared', 'tools_cfg_ref.xml')))
    root = tree.getroot()
    iar_dir = root.find('.//iar_install').text
    # Read device support files from IAR install
    from pathlib import Path
    for f in Path(os.path.join(iar_dir, 'config', 'devices', 'Renesas')).glob('**/R7FA*'):
        device_name = 'R' + os.path.basename(f)[3:7]
        if device_name not in iar_supported_mcus:
            iar_supported_mcus.append(device_name)

    # Manually created input file
    with open(input, 'r') as f:
        input_template_dict = yaml.load(f, Loader=yaml.CLoader)

    # Clear output file to begin. Will append moving forward.
    with open(output, 'w') as f:
        f.write("")

    # Create permutations of RTOSs and Boards
    for template in input_template_dict['templates']:

        projects = []

        with open(template['template']) as temp:
            t = Template(temp.read())

        # If no toolchain list is found, then use default
        if 'toolchains' not in template:
            template['toolchains'] = default_toolchain_list

        for tc in template['toolchains']:

            for rtos in template['rtoses']:

                # If no board list is found, then use default
                if 'boards' not in template:
                    template['boards'] = public_board_list
                    # If this is not a tagged release, then include the internal boards too
                    if tagged == False:
                        template['boards'] += internal_board_list

                for board in template['boards']:

                    if 'iar' in tc.lower():
                        # See if IAR supports this MCU. For board comparison, this will use the list as provided. We will also change to part number style
                        # (eg RA2A1 to R7FA2A1) for checking against part numbers. We could just remove the first 'r' and check that against board and
                        # part number. The only reason I have not done this is in case there is ever a case where the rest of the MCU name may match
                        # the ending part of a different part number (eg: match the package type, memory size).
                        iar_support = False

                        for mcu in iar_supported_mcus:
                            # Check against board name first. eg: ra6m3 in ek-ra6m3
                            if mcu.lower() in board.lower():
                                iar_support = True
                            # Check against MCU name. eg: r7fa6m3 in r7fa6m3ah
                            if mcu.lower().replace('ra', 'r7fa') in board.lower():
                                iar_support = True

                        if iar_support == False:
                            # IAR does not currently support this MCU
                            continue

                    projects.append({   "rtos" : rtos['name'],
                                        "template" : rtos['template'],
                                        "board" : board,
                                        "warnings" : "0",
                                        "toolchain" : tc,
                                        "template_data" : template['template_data'] })

        with open(output, 'a') as f:
            f.write(t.render(projects=projects))

if __name__ == "__main__":

    import argparse

    # Allow command line arguments
    parser = argparse.ArgumentParser(
        description = "Create YML project list based on templates.",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter
    )

    parser.add_argument("-i",
        dest="input",
        help="File with template data",
        metavar="INPUT_TEMPLATE",
        default=None,
    )

    parser.add_argument("-o",
        dest="output",
        help="Where to store output YAML file",
        metavar="OUTPUT",
        required=True
    )

    parser.add_argument("--tagged",
        dest="tagged",
        help="Whether this is a tagged release. If so, don't include internal boards or MCUs in testing",
        action="store_true",
        required=False
    )

    # Get command line arguments
    arguments = parser.parse_args()

    parse(arguments.input, arguments.output, arguments.tagged)
