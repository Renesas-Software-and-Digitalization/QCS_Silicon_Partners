# This file will generate 2 files:
# install_info.json
#   This file has information about each FSP version installed in e2 studio.
# default_projects.json
#   This file will give a list of projects to be built that should represent all default projects that can be created for each version of FSP installed.

loadModule("/FSP/ProjectGen")

import json
import os
import yaml
import xml.etree.ElementTree as ET

CONST_FSP_FAMILY = 'ra'

# Output dictionary
od = {}

tcs = getAvailableToolchains()
od['toolchains'] = []
for tc in tcs:
    tc_vers = getAvailableToolchainVersions(tc)
    # IAR does not have a version list. The version is in the toolchain name instead
    if len(tc_vers) == 0:
        od['toolchains'].append({ "name": tc, "version": "" })
    for tc_ver in tc_vers:
        od['toolchains'].append({ "name": tc, "version": tc_ver })

fsp_versions = getAvailableFspVersions(CONST_FSP_FAMILY)
od['fsps'] = []
for version in fsp_versions:
    # Start new dict to load up
    od_fsp = { "name": version }
    boards = getAvailableBoards(version, CONST_FSP_FAMILY)
    od_fsp['boards'] = []
    for board in boards:
        od_fsp['boards'].append({ "name": board.getDisplay() })
    mcus = getAvailableDevices(version, CONST_FSP_FAMILY)
    od_fsp['mcus'] = []
    for mcu in mcus:
        od_fsp['mcus'].append({ "name":  mcu.getShortName()})
    rtoses = getAvailableRtoses(version, CONST_FSP_FAMILY)
    od_fsp['rtoses'] = []
    for rtos in rtoses:
        od_fsp['rtoses'].append({ "name": rtos.getRtosOption().getNonVersionedDisplay()})
    templates = getAvailableTemplates(version, CONST_FSP_FAMILY)
    od_fsp['templates'] = []
    for template in templates:
        od_fsp['templates'].append({ "name": template })
    od['fsps'].append(od_fsp)

# Currently, EASE does not tell us which templates apply to which RTOSs or boards. For example, you cannot use
# "Bare Metal - Minimal" with FreeRTOS. Also, you cannot use "Blinky" projects with Custom User Board projects.
# This mapping is doing this part manually for now.
mapping_rtos_to_templates = []
# "No RTOS" should run on "Bare Metal" templates
mapping_rtos_to_templates.append({ "rtos" : "No RTOS", "template_match" : "Bare Metal" })
# "FreeRTOS" should run on "FreeRTOS" templates
mapping_rtos_to_templates.append({ "rtos" : "FreeRTOS", "template_match" : "FreeRTOS" })
# Azure RTOS should run on "Azure RTOS" templates
mapping_rtos_to_templates.append({ "rtos" : "Azure", "template_match" : "Azure" })

# e2 studio does not allow many characters for project names. Replace characters that are a part of unique name that don't meet e2 requirements.
def getSafeName(input):
    return input.replace("-", "dash").replace(" ","space").replace("+","plus").replace(".","dot").replace("(","op").replace(")","cp")

def createProject(pd, fsp_version, board_or_device, toolchain, toolchain_version, rtos_name, template_name):

    global iar_supported_mcus

    if "iar" in toolchain.lower():
        short_tc_name = "iar"

        # See if IAR supports this MCU. For board comparison, this will use the list as provided. We will also change to part number style
        # (eg RA2A1 to R7FA2A1) for checking against part numbers. We could just remove the first 'r' and check that against board and
        # part number. The only reason I have not done this is in case there is ever a case where the rest of the MCU name may match
        # the ending part of a different part number (eg: match the package type, memory size).
        iar_support = False

        for mcu in iar_supported_mcus:
            # Check against board name first. eg: ra6m3 in ek-ra6m3
            if mcu.lower() in board_or_device.lower():
                iar_support = True
            # Check against MCU name. eg: r7fa6m3 in r7fa6m3ah
            if mcu.lower().replace('ra', 'r7fa') in board_or_device.lower():
                iar_support = True

        if iar_support == False:
            # IAR does not currently support this MCU
            return
    elif ("ac6" in toolchain.lower()) or ("arm compiler" in toolchain.lower()):
        short_tc_name = "ac6"
    else:
        short_tc_name = "gcc"

    # Get a path name which can be used for artifacts so everything is not at 1 level
    path_name = fsp_version + "_" + short_tc_name + "_" + rtos_name + "_" + template + "_" + board_or_device

    # Default sequence is to add a thread if an RTOS is used and then build. If a thread is not added then e2 studio will not generate all content needed.
    full_sequence = []

    if 'no rtos' not in rtos_name.lower():
        create_thread_sequence = { 'op': 'create_thread', 'name': 't0' }
        full_sequence.append(create_thread_sequence)

    build_sequence = { 'op': 'build', 'completed': 1 }
    full_sequence.append(build_sequence)

    # Convert to names safe for filename use
    project_name = getSafeName(path_name)
    pd.append({ 'name': project_name,
                'fsp_version': fsp_version,
                'board_or_device': board_or_device,
                'toolchain': toolchain,
                'toolchain_version': toolchain_version,
                'rtos': rtos_name,
                'template': template_name,
                'path': path_name,
                'sequence': full_sequence })


with open(os.path.join("/", "e2_studio", "input", "ease_info.json"), 'r') as json_file:
    # Read project dictionary from JSON file
    ease_info = json.load(json_file)

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

# Project dictionary that will be output
pd = []

for fsp_version in fsp_versions:
    for rtos in rtoses:

        # Get non-versioned RTOS name
        # eg: 'Azure RTOS ThreadX' instead of 'Azure RTOS ThreadX (v6.1.8+fsp.3.5.0.beta1.20211112.2cc3a458)'
        rtos = rtos.getRtosOption().getNonVersionedDisplay()

        for mapping in mapping_rtos_to_templates:
            # See if current RTOS matches one we want to test from mapping_rtos_to_templates[]
            if mapping['rtos'] not in rtos:
                continue

            for template in templates:
                # Build board projects (iterate over Boards)
                for board in boards:
                    board = board.getDisplay()

                    # Skip custom board. That will be tested in next loop.
                    if ("Custom" in board):
                        continue

                    # See comment above mapping_rtos_to_templates[]
                    if (mapping['template_match'] not in template):
                        continue

                    for tc in od['toolchains']:
                        tc_chosen = tc['name']
                        tc_chosen_version = tc['version']

                        # Convert to names safe for filename use
                        createProject(pd, fsp_version, board, tc_chosen, tc_chosen_version, rtos, template)

                # Build Custom User Board projects (iterate over MCUs)
                for mcu in mcus:
                    mcu = mcu.getShortName()

                    # Custom User Boards cannot create blinky projects because they do not have LEDs
                    if ("Blinky" in template):
                        continue

                    # See comment above mapping_rtos_to_templates[]
                    if (mapping['template_match'] not in template):
                        continue

                    for tc in od['toolchains']:
                        tc_chosen = tc['name']
                        tc_chosen_version = tc['version']

                        # Convert to names safe for filename use
                        createProject(pd, fsp_version, mcu, tc_chosen, tc_chosen_version, rtos, template)


# This file is currently just for reference about what is available in current e2 studio installation
with open(ease_info['install_info'], 'w') as f:
    f.write(json.dumps(od, indent=2))

# This file will give a list of projects that can be built. The expectation is that this file will be iterated over and projects creatd and built.
with open(ease_info['default_projects'], 'w') as f:
    f.write(json.dumps(pd, indent=2))
