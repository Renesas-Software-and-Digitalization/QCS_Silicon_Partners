# This file will create projects for each project in ${CI_PROJECT_DIR}/e2_studio/input/projects.json

# EASE References:
# https://jira.renesas.eu/confluence/pages/viewpage.action?spaceKey=IDE&title=Headless+Synergy+Configuration
# https://jira.renesas.eu/confluence/display/IDE/RA+Scripted+Project+Generation

# For creating projects
import time

loadModule("/FSP/ProjectGen")
# For modifying threads and stacks
loadModule("/FSP/Configuration")
# Used for building and getting build output
loadModule("/FSP/Build")
# Used for getting project path. Used for getting secure bundle path.
loadModule("/System/Resources")
# Used for setting project configurations
loadModule('/Renesas Build/ManagedBuildSettings')

import json
import os
import logging
import shutil
import re
import threading
import traceback
import py4j

CONST_FSP_FAMILY = 'ra'

logger = logging.getLogger("ease_create_projects.py")
logger.setLevel(logging.DEBUG)
fh = logging.FileHandler('ease.log')
fh.setLevel(logging.DEBUG)
formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
fh.setFormatter(formatter)

# Add output to console to keep track of progress
sh = logging.StreamHandler()
sh.setLevel(logging.DEBUG if os.environ.get('DEBUG', '0') == '1' else logging.INFO)
sh.setFormatter(formatter)

logger.addHandler(fh)
logger.addHandler(sh)

# This logging is added to try and catch the issue reported here:
# https://jira.renesas.eu/browse/IDE-46392
# Putting in separate file so FSP EASE output is still readable
py4j_fh = logging.FileHandler('ease_py4j.log')
py4j_fh.setLevel(logging.DEBUG)
formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
py4j_fh.setFormatter(formatter)

py4jlog = logging.getLogger("py4j")
py4jlog.setLevel(logging.DEBUG)
py4jlog.addHandler(py4j_fh)

# This logging is just for errors. If this error log file gets created then script running this can determine that failure(s) occurred.
# Some problems, such as constraint not working as exepcted, may not show up when building. For that reason if an error like that is found
# then we will write the error out and fail the job after the script has finished.
error_logger = logging.getLogger("ERROR - ease_create_projects.py")
error_logger.setLevel(logging.ERROR)
error_fh = logging.FileHandler('e2_studio/logs/errors.log')
error_fh.setLevel(logging.ERROR)
formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
error_fh.setFormatter(formatter)
error_logger.addHandler(error_fh)


def progressing(checker, msg, waiting=20):
    while not os.path.exists(checker):
        logger.info(msg)
        time.sleep(waiting)


# We have to disable float16 for CMSIS testing projects
# Insert "#define DISABLEFLOAT16" to arm_math_types_f16.h before it is to be used
def fix_fp16_build_error(project_path):
    file = os.path.join(project_path, "ra/arm/CMSIS_5/CMSIS/DSP/Include/arm_math_types_f16.h")
    if os.path.isfile(file):
        lines = ""
        with open(file, 'r') as f:
            for line in f:
                if "__ARM_FEATURE_MVE" in line:
                    lines += "#define DISABLEFLOAT16\n"
                lines += line

        with open(file, 'w') as f:
            f.writelines(lines)


def log_error(msg):
    """
    Helper function for logging error
    :param msg:
    :return:
    """
    global global_error
    logger.error(msg)
    error_logger.error(msg)
    global_error = True


def find_module(current_module, requires_id):
    """
    Recursive function for attempting to find a module in a stack
    :param current_module:
    :param requires_id:
    :return: the module or None if not found
    """
    module = None

    # Get all dependencies of this module. Some may be filled in and others now. Use hasModule() to detect.
    interfaces = current_module.getInterfaces()

    for interface in interfaces:
        logger.debug("Checking interface: " + interface.getId())
        # See if this is the <requires> we are looking for
        if interface.getId() == requires_id:
            logger.debug("Found requires. Check if it is filled in")
            if interface.hasModule():
                logger.debug("Module found.")
                module = interface.getModule()
            else:
                log_error("In attempt to find a module, the <requires> was found but no module was being used.")
            break
        elif interface.hasModule():
            # Recurse down tree and keep looking
            module = find_module(interface.getModule(), requires_id)

            if module:
                break

    return module


def find_pin_component(cfg, cid):
    """
    Find component by ID in PinConfigurations
    :param cfg: the project configuration
    :param cid: the component id
    :return: the pin configuration component object or None if no match
    """
    for pin_cfg in cfg.getPinConfigurations():
        for comp in pin_cfg.getComponents():
            if comp.getId() == cid:
                return comp
    return None


def _iterate_component_attrs(comp):
    for p in comp.getPins():
        yield p
    for p in comp.getProperties():
        yield p
    for c in comp.getConfigs():
        yield c
    try:
        for o in comp.getOptions():
            yield o
    finally:
        pass


def set_component_attr_value(comp, attr_id, attr_value):
    """
    Set value to the attribute (config or property or option) of the comp
    :param comp: the pin configuration component object
    :param attr_id: the attribute name to set value
    :param attr_value: the value to be set to the attribute
    :return: True if successfully set; False otherwise
    """
    cid = comp.getId()
    full_id = "%s.%s" % (cid, attr_id)
    for attr in _iterate_component_attrs(comp):
        if full_id == attr.getId():
            attr_type = attr.getClass().getName().split('.')[-1]
            full_value = '%s.%s' % (cid, attr_value) if \
                'PinConfig' in attr_type or 'PinPin' in attr_type \
                else attr_value
            logger.debug('... try to set %s:[%s] from [%s] to [%s]' %
                         (attr_type, full_id, attr.getValue(), full_value))
            attr.setValue(full_value)
            return True
    return False


OBJ_DICT = dict({
    "EVENT GROUP": "flags",
    "EVENT FLAGS": "flags",
    "MUTEX": "mutex",
    "COUNTING SEMAPHORE": "semaphore",
    "BINARY SEMAPHORE": "binary_semaphore",
    "QUEUE": "queue",
    "STREAM BUFFER": "stream_buffer",
    "MESSAGE BUFFER": "message_buffer",
    "TIMER": "timer"
})


def get_rtos_object_property(obj, name):
    """
    Get the property ID for RTOS object
    :param obj: the RTOS object
    :param name: the name of the property
    :return: the ID of the RTOS property
    """
    o_id = obj.getId()
    p_key = '.'.join(o_id.split('.')[:-1] + [name])
    logger.debug("objId: [%s], resolved propId: [%s]" % (o_id, p_key))
    return p_key


def create_rtos_object(cfg, obj_type, os='awsfreertos'):
    """
    Create a RTOS object
    :param cfg: the configuration object
    :param obj_type: the RTOS object type to be created
    :param os: the OS this object is created for
    :return: the created RTOS object
    """
    obj_id = OBJ_DICT.get(obj_type.upper(), None)
    return create_object(cfg, "rtos.%s.object.%s" % (os, obj_id)) if obj_id else None


def create_object(cfg, id):
    """
    Create an Object by given ID
    :param cfg: the configuration object
    :param id: the object id to be created
    :return: the created object
    """
    if not cfg or not id:
        log_error("Unable to create object due to invalid object ID or null parent config")
        return None
    return cfg.createObject(id)


# Some project options allow "default". We need to fill that in.
default_fsp_version = getAvailableFspVersions(CONST_FSP_FAMILY)[0]

# IAR and AC6 have version in name. The toolchain version will be an empty string
tcs = getAvailableToolchains()
default_iar = None
for tc in tcs:
    if 'iar' in tc.lower():
        default_iar = (tc, "")
    if 'gnu' in tc.lower():
        default_gcc = (tc, getAvailableToolchainVersions(tc)[0])
    if 'arm compiler' in tc.lower():
        default_ac6 = (tc, "")

# RTOS used to have version in name. Now we can reference without version. Still using our own short form (eg: 'Azure' for 'Azure RTOS ThreadX')
available_rtoses = []
xml_rtoses = getAvailableRtoses(default_fsp_version, CONST_FSP_FAMILY)
for rtos in xml_rtoses:
    available_rtoses.append(rtos.getRtosOption().getNonVersionedDisplay())

WK_PATH = os.environ.get('ENV_WORKSPACE_PATH', os.path.join(os.path.pathsep, 'e2_studio', 'workspace'))
logger.info('Process with current setting WORKSPACE: %s' % WK_PATH)

with open('/e2_studio/input/projects.json', 'r') as json_file:
    # Read project dictionary from JSON file
    pd = json.load(json_file)

global_error = False

logger.info("Number of projects to be created: " + str(len(pd)))

project_counter = 0

for project in pd:
    # { 'name': '2dot3dot0_FreeRTOS_FreeRTOSspacedashspaceMinimalspacedashspaceStaticspaceAllocation_R7FA2E1A82DNE',
    #   'fsp_version': '2.3.0',
    #   'board_or_device': 'R7FA2E1A82DNE',
    #   'toolchain': 'GNU ARM Embedded',
    #   'toolchain_version': '9.2.1.20191025',
    #   'rtos': 'FreeRTOS',
    #   'template': 'FreeRTOS - Minimal - Static Allocation'
    # }

    logger.info("Creating project " + str(project_counter) + " - " + project['name'])
    threading.Thread(
        target=progressing,
        args=(
            os.path.join(WK_PATH, project['name'], '.generated'),
            '... project creation in progress ...',
        )
    ).start()
    project_counter += 1

    if project['fsp_version'] == 'default':
        fsp_version = default_fsp_version
    else:
        fsp_version = project['fsp_version']

    logger.debug("  FSP version - " + fsp_version)

    # Make choosing toolchain easier
    proj_tc_lower = project['toolchain'].lower()

    if ('gcc' in proj_tc_lower) or ('gnu' in proj_tc_lower):
        chosen_tc = default_gcc[0]
        if project['toolchain_version'] == 'default':
            chosen_tc_version = default_gcc[1]
        else:
            chosen_tc_version = project['toolchain_version']

    elif 'iar' in proj_tc_lower:
        chosen_tc = default_iar[0]
        if project['toolchain_version'] == 'default':
            chosen_tc_version = default_iar[1]
        else:
            chosen_tc_version = project['toolchain_version']

    elif ('ac6' in proj_tc_lower) or ('arm compiler' in proj_tc_lower):
        chosen_tc = default_ac6[0]
        if project['toolchain_version'] == 'default':
            chosen_tc_version = default_ac6[1]
        else:
            chosen_tc_version = project['toolchain_version']

    else:
        chosen_tc = project['toolchain']
        chosen_tc_version = project['toolchain_version']

    logger.debug("  Toolchain - " + chosen_tc + ". Version -" + chosen_tc_version)

    # TZ support was not in original version. We assume projects are 'flat'
    if 'type' not in project:
        project['type'] = 'flat'

    if 'secure_project' not in project:
        project['secure_project'] = ""

    # Find RTOS name
    rtos = ''
    for current_rtos in available_rtoses:
        if project['rtos'] in current_rtos:
            rtos = current_rtos

    logger.debug("  RTOS: " + rtos)

    logger.debug("  Board or Device - " + project['board_or_device'])

    # eg: generateCExeProject("test_gen3", "3.0.0-alpha.1", "EK-RA6M3", "GNU ARM Embedded", "9.3.1.20200408", "Azure RTOS ThreadX (v6.1.2+fsp.3.0.0.alpha.1)", "Bare Metal - Blinky")
    pg = projectGenerationFor(CONST_FSP_FAMILY, fsp_version)
    pg.projectName(project['name'])
    # Set device or board depending on what is sent in. All RA MCU part numbers start with 'r7f'
    if project['board_or_device'].lower().startswith('r7f'):
        pg.device(project['board_or_device'])
    else:
        pg.board(project['board_or_device'])
    pg.toolchain(chosen_tc, chosen_tc_version)
    pg.projectType(project['type'].upper())
    # For non-secure projects we need to pass in a location for the secure bundle
    if project['type'].lower() == 'nonsecure':
        pg.secureBundleLocation(
            str(getProject(project['secure_project']).getLocation()) + '/Debug/' + project['secure_project'] + '.sbd')
    pg.rtos(rtos)
    pg.template(project['template'])
    pg_error = pg.generate(None)

    # Check for any errors in creating project. For example, if you attempt to create a project with the same name that already exists, you will get an error. Example messages:
    #   Status OK: com.renesas.cdt.ddsc.projectgen code=0 Project generation status null children=[Status OK: unknown code=0 OK null]
    #   Status ERROR: com.renesas.cdt.ddsc.projectgen code=0 Project generation failed null children=[Status ERROR: com.renesas.cdt.ddsc.projectgen code=0 Project "test0" already exists null]
    # We need to check for duplicate projects because the script will continue on and not report an issue otherwise
    if 'status error' in str(pg_error).lower():
        log_error('Error in creating project. Error message: ' + str(pg_error))
        continue

    project_path = os.path.join(WK_PATH, project['name'])

    # Write JSON snippet to project so we don't have to decode the project details from the name
    with open(os.path.join(project_path, 'project_info.json'), 'w') as info:
        json.dump(project, info)

    # If no sequence is defined then we can exit here
    if 'sequence' not in project:
        logger.debug("Using blank template")
        continue

    logger.debug('Opening project - ' + project['name'])

    # Open newly created project
    cfg = openDDSCConfiguration(project['name'])

    # This dictionary will keep track of variables created during sequences. This makes it easier than having
    # to search for an object in every step. For example, creating a stack in one thread and then changing a property
    # in the next step.
    proj_objs = {}

    build_counter = 0

    for seq in project['sequence']:

        logger.debug("Executing sequence - " + seq['op'])

        try:

            # Get HAL/Common thread reference
            # - op: get_hal
            #   name: Name to register HAL thread with
            if seq['op'] == 'get_hal':
                logger.debug("Finding HAL and registering to: " + seq['name'])

                for thread in cfg.getThreads():
                    if thread.isHal() == True:
                        proj_objs[seq['name']] = thread

            # Create new thread
            # - op: create_thread
            #   name: Name to register new thread with
            if seq['op'] == 'create_thread':
                logger.debug("Creating thread: " + seq['name'])
                proj_objs[seq['name']] = cfg.createThread()

            # Create new RTOS object
            # - op: create_rtos_object
            #   name: Name to register new rtos object with
            #   type: The RTOS object type to be created
            #   os: Either *awsfreertos* or *threadx* the OS this object is created for; the prior is the default
            if seq['op'] == 'create_rots_object':
                logger.debug("Creating RTOS object: " + seq['name'])
                proj_objs[seq['name']] = create_rtos_object(cfg, seq['type'], seq.get('os', 'awsfreertos'))

            # Create new module instance
            # - op: add
            #   name: Name to register new module instance with
            #   id: The <module id=""> to select
            #   thread: Which thread to add this module instance too. This should be a previously registered "name"
            if seq['op'] == 'add':
                logger.debug("Adding module instance. " + seq['id'])
                current_thread = proj_objs[seq['thread']]
                s0 = current_thread.createStack(seq['id'])
                proj_objs[seq['name']] = s0

            # Fill in dependency of module instance
            # - op: fill_requires
            #   name: Name to register new module instance with
            #   module: Which module "name" to use. Must have been registered in previous operation (eg: add).
            #   requires_id: Which <requires id=""> to use.
            #   module_id: Which <module id=""> to use for filling the dependency specified by "requires_id"
            #   use: Use an existing stack instead of creating a new one [True/False].
            #   dependant_instance_name: Name of an existing stack.
            if seq['op'] == 'fill_requires':
                # Get object created earlier
                s0 = proj_objs[seq['module']]
                if 'use' in seq and seq['use']:
                    logger.debug(
                        "Filling requires using an existing stack. " + seq['module'] + "-" + seq['requires_id'] + "-" +
                        seq['dependant_instance_name'])
                    s0.getInterface(seq['requires_id']).useModule(proj_objs[seq['dependant_instance_name']])
                else:
                    logger.debug(
                        "Filling requires. " + seq['module'] + "-" + seq['requires_id'] + "-" + seq['module_id'])
                    # Get interface and fill in
                    m0 = s0.getInterface(seq['requires_id']).createModule(seq['module_id'])
                    proj_objs[seq['name']] = m0

            # Gets a dependency and registers it for future use
            # - op: get_dependency
            #   name: Name to register the found module instance with
            #   module: Which module "name" to use for the parent. Must have been registered in previous operation (eg: add).
            #   requires_id: Which <requires id=""> to use.
            if seq['op'] == 'get_dependency':
                logger.debug("Getting dependency. " + seq['module'] + "-" + seq['requires_id'] + "-" + seq['name'])
                # Get object created earlier
                p0 = proj_objs[seq['module']]
                # Get dependent module
                m0 = p0.getInterface(seq['requires_id']).getModule()
                proj_objs[seq['name']] = m0

            # Change an attribute of a PinConfiguration component
            # - op: change_pin
            #   id: the component id to change
            #   attr: the attribute of the component to change/set
            #   value: Value to set.
            #   success: [Optional] "yes" or "no". Default is "yes". Whether this will successfully be set. This is "no" when you want to test a <constraint> inside a <property>. e2 studio will not accept an invalid value.
            if seq['op'] == 'change_pin':
                # Add default option if not set
                if 'success' not in seq:
                    seq['success'] = 'yes'

                set_value_succeeded = False
                comp = find_pin_component(cfg, seq['id'])
                if comp:
                    set_value_succeeded = set_component_attr_value(comp, seq['attr'], seq['value'])
                if (seq['success'] == 'yes') and (not set_value_succeeded):
                    log_error(project['name'] + " - Result of changing " + seq['id'] +
                              " did not match expectation. Expected - " + seq['success'] +
                              ". Actual - " + str(set_value_succeeded))

            # Change a connectivity
            # - op: change_connectivity
            #   id: Pin Configuration component id to set up the connection
            #   pin: the pin of the component to connect
            #   port: the port id for the pin to connect to
            #   success: [Optional] "yes" or "no". Default is "yes". Whether this will successfully be set. This is "no" when you want to test a <constraint> inside a <property>. e2 studio will not accept an invalid value.
            if seq['op'] == 'change_connectivity':
                # Add default option if not set
                if 'success' not in seq:
                    seq['success'] = 'yes'

                set_value_succeeded = False
                peripheral = find_pin_component(cfg, seq['id'])
                if peripheral:
                    set_value_succeeded = set_component_attr_value(
                        peripheral, seq['pin'], '%s.%s' % (seq['pin'], seq['port']))

                if (seq['success'] == 'yes') and (not set_value_succeeded):
                    log_error(project['name'] + " - Result of connecting " + seq['id'] +
                              " did not match expectation. Expected - " + seq['success'] +
                              ". Actual - " + str(set_value_succeeded))

            # Change a property of a module instance or thread
            # - op: change_property
            #   type: "instance" or "common" or "rtos"
            #   module_or_thread: Previously named module or thread. Use 'bsp' for setting BSP property.
            #   id: Which <property id=""> to set.
            #   value: Value to set for <property>. Can be integer or string.
            #   success: [Optional] "yes" or "no". Default is "yes". Whether this will successfully be set. This is "no" when you want to test a <constraint> inside a <property>. e2 studio will not accept an invalid value.
            #   name: [Optional] Name to register this property to (currently only used for debugging)
            if seq['op'] == 'change_property':

                # Add default option if not set
                if 'success' not in seq:
                    seq['success'] = 'yes'

                # Get module to set property for
                if seq['module_or_thread'] == "bsp":
                    s0 = cfg.getBSP()
                    seq['type'] = 'instance'
                else:
                    s0 = proj_objs[seq['module_or_thread']]

                # Get property
                if seq['type'] == 'common':
                    # Common
                    p0 = s0.getCommonProperty(seq['id'])
                else:
                    # Instance specific
                    if seq['type'] == 'rtos':
                        prop_id = get_rtos_object_property(s0, seq['id'])
                    else:
                        prop_id = seq['id']
                    p0 = s0.getProperty(prop_id)

                logger.debug(
                    "Changing property. " + seq['module_or_thread'] + "-" + seq['id'] + "-" +
                    str(seq['value']) + "- Expecting success: " + seq['success'])
                try:
                    # Set new value
                    p0.setValue(str(seq['value']))
                    if 'name' in seq:
                        proj_objs[seq['name']] = p0
                    set_value_succeeded = True
                # If <constraint> catches an invalid option then an exception will be triggered. Catch and continue.
                except py4j.protocol.Py4JError as e:
                    set_value_succeeded = False

                if (((seq['success'] == 'yes') and (set_value_succeeded == False)) or
                        ((seq['success'] == 'no') and (set_value_succeeded == True))):
                    command_descriptor = seq['name'] if 'name' in seq.keys() else seq['id']
                    log_error(project[
                                  'name'] + " - Result of changing property for " + command_descriptor + " did not match expectation. Expected - " +
                              seq['success'] + ". Actual - " + str(set_value_succeeded))

            # Verify a constraint is available with a given string inside of it
            # - op: check_constraints_message
            #   search_text: Will check for a match of this text as part of any of the current constraints
            #   [Optional] Default for 'found' is 'yes'
            #   found: Whether message should be found. 'yes' or 'no'
            if seq['op'] == 'check_constraints_message':

                # Add default option if not set
                if 'found' not in seq:
                    seq['found'] = 'yes'

                expected_found = True if seq['found'] == 'yes' else False

                logger.debug(
                    "Checking for constraint message: " + seq['search_text'] + ". Expectation to find is: " + str(
                        seq['found']))
                problems = cfg.getProblems()
                actual_found = False
                logger.debug("Number of problems: " + str(len(problems)))
                for problem in problems:
                    logger.debug("Problem message: " + problem)
                    if seq['search_text'].strip() in problem:
                        logger.debug("Message found!")
                        actual_found = True
                logger.debug("Problem found: " + str(actual_found))

                if actual_found != expected_found:
                    log_error(project['name'] + " - Constraint message expectation did not match finding: " + seq[
                        'search_text'] + ". Expected: " + str(expected_found) + ". Actual: " + str(actual_found))

            # Verify a certain number constraint messages are currently active
            # - op: check_constraints_number
            #   number: Number of constraints to verify are active
            if seq['op'] == 'check_constraints_number':
                logger.debug("Checking for constraint number: " + str(seq['number']))
                num_problems = len(cfg.getProblems())

                if num_problems != seq['number']:
                    log_error(project['name'] + " - Number of constraints did not match. Expected " + str(
                        seq['number']) + ". Found: " + str(num_problems))

            # Replace a string in file.
            # - op: replace_in_file
            #   src: Path to file inside of project that should be modified.
            #   match: What string to match in the src file.
            #   replace: What to replace the string with. This can be multiline using YML multiline formatting as shown below.
            if seq['op'] == 'replace_in_file':
                logger.debug("Modifying file: " + seq['src'])

                # Read original file
                with open(os.path.join(project_path, seq['src']), 'r') as f:
                    fdata = f.read()

                logger.debug("File read")

                # Replace data
                fdata = fdata.replace(seq['match'], seq['replace'])

                # Write data back
                with open(os.path.join(project_path, seq['src']), 'w') as f:
                    f.write(fdata)

                logger.debug("File written")

            # Insert text into a file.
            # - op: insert_in_file
            #   src: Path to file inside of project that should be modified.
            #   line: <integer> for line number or 'EOF' for end-of-file
            #   text: Text to insert. This can be multiline using YML multiline formatting as shown below.
            if seq['op'] == 'insert_in_file':
                logger.debug(
                    "Inserting text into file: " + seq['src'] + ". Line: " + str(seq['line']) + ". Content" + seq[
                        'text'])

                # Read original file
                with open(os.path.join(project_path, seq['src']), 'r') as f:
                    fdata = f.readlines()

                logger.debug("File read")

                # Insert data
                if seq['line'] == 'EOF':
                    fdata.append(seq['text'])
                else:
                    fdata.insert(seq['line'], seq['text'])

                # Write data back
                with open(os.path.join(project_path, seq['src']), 'w') as f:
                    f.writelines(fdata)

                logger.debug("File written")

            # Add include path to project settings
            # - op: add_include
            #   path: path to be included of the project
            #   absolute: 'yes' to specify the given path should be considered as absolute path; 'no' otherwise.
            #             default as 'no' to refer path under the project folder
            #  - also -
            # Add library to project settings
            # - op: add_library
            #   libs: the names of library (-l)
            #   path: search path of above library to be added to the project (-L)
            #   absolute: 'yes' to specify the given path should be considered as absolute path; 'no' otherwise.
            #             default as 'no' to refer path under the project folder
            if seq["op"] == "add_include" or seq['op'] == "remove_include" or \
                seq["op"] == "add_library" or seq["op"] == "remove_library":

                if "yes" == seq["absolute"]:
                    _target = '"%s"' % seq['path']
                else:
                    _target = '"${workspace_loc:/${ProjName}%s}"' % seq['path']

                _opers = []
                [_action, _setting] = seq["op"].split("_")
                if _action == "add":
                    if _setting == "include":
                        _opers.append(
                            {
                                "op": addIncludePath,
                                "tg": _target
                            },
                        )
                    else:
                        _opers.append(
                            {
                                "op": addLibraryPath,
                                "tg": _target
                            },
                        )
                        for lib in seq["libs"]:
                            _opers.append(
                                {
                                    "op": addLibraryFile,
                                    "tg": lib
                                },
                            )
                else:
                    if _setting == "include":
                        _opers.append(
                            {
                                "op": removeIncludePath,
                                "tg": _target
                            },
                        )
                    else:
                        for lib in seq["libs"]:
                            _opers.append(
                                {
                                    "op": removeLibraryFile,
                                    "tg": lib
                                },
                            )
                        _opers.append(
                            {
                                "op": removeLibraryPath,
                                "tg": _target
                            },
                        )

                # _cfg_names = list(filter(
                #     lambda _cfg: len(_cfg.strip()) > 0,
                #     [bc.getName() for bc in getProject(project['name']).getBuildConfigs()]
                # ))
                for _cfg in ['Debug', 'Release']:
                    for _oper in _opers:
                        logger.info("%s %s %s to [%s - %s]" % (_action, _setting, _oper["tg"], project['name'], _cfg))
                        _oper["op"](project['name'], _cfg, _oper["tg"])

            # Add file to project.
            # - op: add_file
            #   src: Path to file to be copied. Starting path is the root of the peaks repo.
            #   dst: Path of where to copy this file. Starting path is the project directory.
            if seq['op'] == 'add_file' or seq['op'] == 'add_folder':
                _src = seq['src']
                _dst = seq['dst']
                logger.debug("Copying From: %s To: %s" % (_src, _dst))

                _dist_path = os.path.join(project_path, _dst)
                os.makedirs(os.path.dirname(_dist_path), exist_ok=True)
                if os.path.isfile(_src):
                    shutil.copy2(_src, _dist_path)
                else:
                    shutil.copytree(_src, _dist_path, dirs_exist_ok=True)

            # Build project and check against expected results.
            # Base warnings are only on files that have */fsp/* in the path. FreeRTOS warnings are ignored for example.
            # eg for a build that completes with 0 warnings and 0 errors
            # - op: build
            #   expect:
            #     completed: 1
            #     warnings_allowed:
            #       -
            # eg for a build that fails with 4 errors (we're not comparing against a specific number of errors, just that the build failed)
            # - op: build
            #   expect:
            #     completed: 0
            #     warnings: 0
            if seq['op'] == 'build':
                expected_results = "completed-" + str(seq['completed'])
                logger.debug("Building project with expected: " + expected_results)

                # Get unique ID so we can find this build output in log. Putting counter on front so its easier to find builds.
                build_id = str(build_counter)

                # logging.info('... saving project configuration, please wait for a few seconds ...')
                cfg.save()

                # Set file for build output
                build_dir = os.path.join(WK_PATH, project['name'], 'Debug')
                os.makedirs(build_dir, exist_ok=True)

                # Perform incremental build, store output to string and to file
                build_logs = os.path.join(build_dir, project['name'] + '.' + build_id + '.build.stdout.log')
                threading.Thread(target=progressing, args=(build_logs, '... build in progress ...',)).start()
                build_output = buildProject(project['name'], INCREMENTAL_BUILD, build_logs)

                for line in build_output.split('\n'):
                    if len(line) > 0: logger.info(line)

                logger.debug("Build finished - " + build_id)
                logger.info(f"Logs available for later checking at: {build_logs}")

                # Look for build results. eg:
                # 20:38:05 Build Finished. 0 errors, 0 warnings. (took 1s.835ms)
                # 20:39:48 Build Failed. 3 errors, 0 warnings. (took 1s.78ms)
                # Use flags and re.DOTALL to include newlines
                matches = re.match('.* ([0-9]+)\ errors, ([0-9]+)\ warnings', build_output, flags=re.DOTALL)

                try:
                    errors = int(matches.group(1))
                    if errors > 0:
                        completed = 0
                    else:
                        completed = 1
                    warnings = int(matches.group(2))
                    logger.debug("Errors: " + str(errors))
                    logger.debug("Total Warnings: " + str(warnings))

                    # If there are more than 0 warnings, then check if they come from FSP code or not
                    if warnings > 0:
                        # This regex will capture GCC and IAR warnings.
                        # eg GCC: ../ra/aws/amazon-freertos/freertos_kernel/tasks.c:3932:39: warning: unused parameter 'pxTCB' [-Wunused-parameter]
                        # eg IAR: "/e2_studio/workspace/default_crc_iar_FreeRTOS_EKdashRA6M3/ra/aws/amazon-freertos/freertos_kernel/tasks.c",4659  Warning[Pa082]: undefined behavior: the order of volatile accesses is undefined in this statement
                        fsp_warnings = re.findall('\n(.*\/fsp\/.*[Ww]arning[:\[].*)', build_output)
                        logger.debug("FSP Warnings: " + str(len(fsp_warnings)))
                        warning_counter = 0
                        # Output warnings to log
                        for w in fsp_warnings:
                            logger.debug("  Warning " + str(warning_counter) + ": " + w)
                            warning_counter += 1

                        # Check against allowed warnings
                        warnings_allowed = []

                        # Check for no provided exceptions
                        if 'warnings_allowed' not in seq or seq['warnings_allowed'] is None:
                            seq['warnings_allowed'] = []

                        for w_regex in seq['warnings_allowed']:
                            logger.debug("  Regex being checked: " + w_regex)
                            for w in fsp_warnings:
                                # If match is found, then warning is allowed and should be removed from list
                                match = re.match(w_regex, w)
                                if match is not None:
                                    # Delete after this since we're iterating over the list currently
                                    logger.debug(" Warning allowed: " + w)
                                    warnings_allowed.append(w)

                        # Log and remove allowed warnings
                        for w in warnings_allowed:
                            # If the same warning matches 2 patterns, the warning may already be removed from fsp_warnings
                            if w in fsp_warnings:
                                fsp_warnings.remove(w)

                        warnings = len(fsp_warnings)

                        # Print out any warnings that were not supposed to be allowed
                        for w in fsp_warnings:
                            log_error('Uncaught warning: ' + w)

                    if (completed != seq['completed']) or (warnings > 0):
                        actual_results = "completed-" + str(completed) + ". warnings-" + str(warnings)
                        log_error(project[
                                      'name'] + " - Build results for " + build_id + " did not match. Expected: " + expected_results + ". Actual: " + actual_results)

                except AttributeError as e:
                    # This means that build output was not in expected format. This usually means that something went wrong in this script. You can check
                    # the e2 studio logs for an error message.
                    log_error(project['name'] + " - Error in trying to parse build output. Exception: " + str(e))

                build_counter += 1

            # Generate project content
            # - op: generate
            if seq['op'] == 'generate':
                logger.debug("Generating project content")

                cfg.save()
                cfg.generateProjectContent(None)

                if "cmsis" in project['name']:
                    fix_fp16_build_error(project_path)

                with open(os.path.join(WK_PATH, project['name'], '.generated'), 'w') as ts:
                    ts.write(time.strftime('%Y%m%d%H%M%S'))

            # Delete an object.
            # - op: delete_dependency
            #   module: Which parent module "name" to use. Must have been registered in previous operation (eg: add).
            #   requires_id: Which <requires id=""> to use.
            if seq['op'] == 'delete_dependency':
                logger.debug("Deleting dependency: Parent:" + seq['module'] + ". Dependency: " + seq['requires_id'])

                proj_objs[seq['module']].getInterface(seq['requires_id']).deleteModule()

            # Find a module in a stack. This is a helper function for get_dependency when you don't want to list out a long stack.
            # - op: find_module
            #   module: Existing module to start search from
            #   requires_id: The ID of the requires that the module you wish to find satisfies. This will be from the parent XML of the module you want to find.
            #   name: Name to register this property to
            if seq['op'] == 'find_module':
                logger.debug("Finding module: Start of search:" + seq['module'] + ". Requires ID: " + seq[
                    'requires_id'] + '. Name to register: ' + seq['name'])

                # Try to find module in stack
                found_module = find_module(proj_objs[seq['module']], seq['requires_id'])

                if found_module:
                    proj_objs[seq['name']] = found_module
                else:
                    log_error("Module was not found.")

            # Search for content in a file. Regex is used for comparison.
            # - op: search_file
            #   src: Path to file inside of project that should be searched.
            #   regex_matches: List of regexes to check for in file
            if seq['op'] == 'search_file':
                logger.debug("Checking for regex matches in file: " + seq['src'])

                # Read file to search
                with open(os.path.join(project_path, seq['src']), 'r') as f:
                    fdata = f.read()

                logger.debug("File read")

                regexes_found = True

                # Look for matches
                for rx in seq['regex_matches']:
                    logger.debug("  Current regex: " + rx)

                    match = re.match(rx, fdata, flags=re.DOTALL)

                    if match == None:
                        # No match found
                        log_error("Regex not matched. File: " + seq['src'] + ". Regex: " + rx)
                        regexes_found = False
                    else:
                        # Match found
                        logger.debug("  Regex matched")

            # Find top of stack and register to name
            # - op: get_stack
            #   thread: Which thread to search in. This name must have been registered earlier.
            #   id: Which ID to look for. This is the "id" attribute <module id="">.
            #   name: Name to register found top of stack to
            #   [Optional] If there are 2 instances of the same module then you will need some way of distinguishing which one you want
            #   property_id: Which <property> to check the value of
            #   property_value: The value of property_id must match this value to be confirmed as a match
            if seq['op'] == 'get_stack':
                logger.debug("Finding top of stack: Thread " + seq['thread'] + ". ID " + seq['id'])

                # Get all tops of stacks for this thread
                tops_of_stack = proj_objs[seq['thread']].getStacks()

                found_module = None

                # Look for matching id attribute
                for stack in tops_of_stack:
                    # id will be the <module_id>.<random_number>. eg: module.driver.timer_on_gpt.1618864991
                    logger.debug(" Checking stack: " + stack.getId())
                    if seq['id'] in stack.getId():
                        logger.debug(" Module found with right id")
                        # See if we need to check a property as well
                        if 'property_id' not in seq:
                            logger.debug(" Module found and registered")
                            found_module = stack
                            break
                        else:
                            # Get property and then check its value
                            property = stack.getProperty(seq['property_id'])
                            if property.getValue() == seq['property_value']:
                                logger.debug(" Matching property found. Module registered")
                                found_module = stack
                                break
                            else:
                                logger.debug(" Property did not match, continue searching")

                if not found_module:
                    log_error("Top of stack module was not found.")
                else:
                    proj_objs[seq['name']] = found_module

            # Change a clock <node>
            # - op: change_clock
            #   id: <node id=""> to use
            #   value: New setting for this node
            if seq['op'] == 'change_clock':
                logger.debug("Changing clock. ID: " + seq['id'] + ". Value: " + str(seq['value']))

                # Get clock node
                c0 = cfg.getClockNode(seq['id'])
                c0.setValue(str(seq['value']))

            # Check if module can be added to the specified thread.
            # - op: check_if_module_can_be_added
            #   thread: Which thread to check. Some modules can only be added to Thread or HAL/Common.
            #   id: Which <module id=""> to check for
            #   success: Whether "id" will be found in the list of module IDs that can be added
            if seq['op'] == 'check_if_module_can_be_added':
                logger.debug("Checking if module " + seq['id'] + " can be added to thread " + seq['thread'])
                current_thread = proj_objs[seq['thread']]

                # Get list of modules available to create for this thread
                valid_ids = current_thread.getCreatableModuleIds()

                # See if this module is in list
                id_in_list = seq['id'] in valid_ids

                if (((id_in_list == True) and (seq['success'] == 'no')) or
                        ((id_in_list == False) and (seq['success'] == 'yes'))):
                    log_error(
                        seq['id'] + " availability of " + str(id_in_list) + " did not match expectation of " + seq[
                            'success'])
                else:
                    logger.debug(seq['id'] + " availability matched expectation of " + str(id_in_list))

            # Delete a top of stack
            # - op: delete_stack
            #   name: Which stack to delete. Must have been registered in previous operation (eg: add or get_stack).
            #   thread: Which thread the stack is in.
            if seq['op'] == 'delete_stack':
                logger.debug("Deleting stack " + seq['name'] + " from thread " + seq['thread'])
                current_thread = proj_objs[seq['thread']]
                current_thread.deleteStack(proj_objs[seq['name']])
                # Delete object from dict
                proj_objs.pop(seq['name'])

            # Change a property of a module instance or thread
            # - op: deselect_option
            #   type: "instance" or "common"
            #   module_or_thread: Previously named module or thread. Use 'bsp' for setting BSP property.
            #   id: Which <property id=""> to alter.
            #   value: Which <option id=""> to deselect.
            #   name: [Optional] Name to register this property to (currently only used for debugging)
            if seq['op'] == 'deselect_option':

                logger.debug(
                    "Deselcting option. " + seq['module_or_thread'] + "-" + seq['id'] + "-" + str(seq['value']))
                # Get module to set property for
                if seq['module_or_thread'] == "bsp":
                    s0 = cfg.getBSP()
                    seq['type'] = 'instance'
                else:
                    s0 = proj_objs[seq['module_or_thread']]

                # Get property
                if seq['type'] == 'common':
                    # Common
                    p0 = s0.getCommonProperty(seq['id'])
                else:
                    # Instance specific
                    p0 = s0.getProperty(seq['id'])

                try:
                    # Set new value
                    p0.excludeOption(str(seq['value']))
                    if 'name' in seq:
                        proj_objs[seq['name']] = p0
                # If <constraint> catches an invalid option then an exeception will be triggered. Catch and continue.
                except py4j.protocol.Py4JError as e:
                    command_descriptor = seq['name'] if 'name' in seq.keys() else seq['id']
                    log_error(project[
                                  'name'] + " - Result of deselecting option for " + command_descriptor + " did not match expectation. Expected - " +
                              seq['success'] + ". Actual - " + str(set_value_succeeded))

            # Print a message to the output log and optionally fail the project.
            # - op: log
            #   message: Message that will be printed.
            #  [Optional] Default for fail is false. Set to true to fail the project.
            #   fail: false
            if seq['op'] == 'log':

                if 'fail' not in seq or seq['fail'] == False:
                    logger.info(seq['message'])
                else:
                    log_error(seq['message'])

        # This is a catch-all for py4j exceptions. If this is encountered then it means you've passed a bad module ID, interface, etc as part of your sequence.
        # Check the logs for more information.
        except py4j.protocol.Py4JError as e:
            log_error("Invalid options provided.\n" + traceback.format_exc() + '\r\n' + str(seq))

    # Save project.
    cfg.save()

if not global_error:
    logger.info("All projects processed with no errors.")
else:
    logger.info("All projects processed. Errors were found.")
