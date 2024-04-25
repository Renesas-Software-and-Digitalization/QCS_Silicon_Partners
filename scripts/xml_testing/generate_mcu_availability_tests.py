import yaml
import os
import xml.etree.ElementTree as ET
from jinja2 import Template

# This is a list of peripherals that are not in the generated data YML but every MCU has it.
# We will use this data to init the peripheral list for each MCU.
CONST_GEN_DATA_MISSING_IP = \
  [
      'module.driver.lpm',
      'module.driver.lvd'
  ]

# This is a list of strings to remove from MDF names. We just need the IP name, not the functionality.
# For example, we need to differentiate "sci" from "sci_b". If we just search for "sci" in "sci_b_spi"
# then we would get a false positive.
CONST_STRINGS_TO_REMOVE = \
  [
      # eg: r_sci_uart, r_sci_b_uart
      '_spi',
      '_uart',
      '_i2c',
      # eg: r_iic_master, r_iic_b_master
      '_master',
      '_slave',
      # For SCE
      '_plaintext',
      '_protected',
  ]

# When looking through generated data, sometimes we need a translation.
# eg: we need 'ETHER' instead of 'ETHERC'
# Each entry is what to search for in generated YML data, and what to replace it with if found
CONST_GEN_DATA_REPLACEMENTS = \
  {
      'ETHERC' : 'ETHER',
      'ETHERC_EDMAC' : 'EDMAC',
      'ETHERC_EPTPC': 'PTP',
      'ETHERC_MII': 'ETHER_PHY',
      'ETHERC_RMII': 'ETHER_PHY',
      'SCE5': 'SCE_RA4',
      'SCE7': 'SCE_RA6',
      'SCE9': 'SCE9_RA6',
      'GPT_POEG': 'POEG',
      'AES': 'SCE_RA2',
      'SSIE': 'SSI',
      'PORT': 'IOPORT',
      'ELC_B': 'ELC',
  }

# Module id's to ignore because this script cannot determine whether it should be allowed or not.
# eg: in r_adc XML the module.driver.adc_on_adc_with_dmac ID is only available on a subset of MCUs that have the ADC IP
CONST_IGNORE_IDS = \
  [
      'module.driver.adc_on_adc_with_dmac',
      'module.driver.sce_protected', # This cannot be used because the RA4E1 and RA6E1 both have SCE9 but is not guaranteed so we do not want to expose it in e2studio
  ]

# Usually we can pick any part number from the part_numbers list in each MCU YML file. For the RA6M5 we cannot do this because the part number determines
# whether CANFD is available or not. The r_canfd XML does check if the part number is one with CANFD support or not so choosing a part number at random
# causes issues with the availability test. Here we will define a static part number for certain MCUs when needed.
CONST_STATIC_PART_NUMBER = \
  {
      'RA6M5.yml' : 'R7FA6M5BH2CBG', # The 'B' (R7FA6M5 --> B <-- H2CBG) determines that this part does have CANFD
  }

# Returns 'True' if the MCU uses the CANFD (Lite) variant of the CANFD MDF.
# CANFD (Lite) MCUs will provide interface.mcu.canfdlite.driver in the Renesas##BSP##{mcu}##fsp####x.xx.xx.xml MDF
def is_canfdlite_mcu(mdf_dir, mcu_name):
    mdf_file = os.path.join(mdf_dir, 'Renesas##BSP##{mcu}##fsp####x.xx.xx.xml'.format(mcu=mcu_name))
    with open(mdf_file, 'r') as f:
        data = f.read()
        if 'interface.mcu.canfdlite.driver' in data:
            return True
    return False

def parse(input_dir, mdf_dir, mcu):

    mdfs = {}
    mcu_ymls = []
    if mcu is None:
        mcu_ymls = os.listdir(input_dir)
    else:
        mcu_ymls.append(mcu.upper() +'.yml')

    all_ids = []

    all_used_ids = CONST_GEN_DATA_MISSING_IP.copy()

    # Find module names in XML dir
    for filename in os.listdir(mdf_dir):
        if filename.lower().startswith('renesas'):
            module_name = filename.split('##')[3]

            if module_name.startswith('r_') or module_name.startswith('rm_'):

                # Remove r_ or rm_
                module_name = module_name.split('_', 1)[1]

                # Remove any unwanted strings
                for s in CONST_STRINGS_TO_REMOVE:
                    module_name = module_name.replace(s, '')

                # Parse XML to get module id's
                tree = ET.parse(os.path.join(mdf_dir, filename))
                root = tree.getroot()
                for module in root.findall('module'):
                    # Should we ignore this id?
                    if module.get('id') in CONST_IGNORE_IDS:
                        continue

                    # If <module> has visible="false" attribute then it cannot be added directly
                    if module.get('visible') == "false":
                        # Store this ID as one that cannot be added
                        all_used_ids.append(module.get('id'))
                        continue

                    # Store <module id=""> attribute for this MDF
                    if module_name not in mdfs:
                        mdfs[module_name] = []
                    mdfs[module_name].append(module.get('id'))
                    all_ids.append(module.get('id'))

    # Use template to create tests
    script_dir = os.path.dirname(os.path.realpath(__file__))
    with open(os.path.join(script_dir, 'ip_availability_test.j2'), 'r') as f:
        test_template = Template(f.read())

    # This loop is duplicated below. This instance of the loop is used for getting all possible IDs,
    # not just the ones for each MCU.
    for filename in mcu_ymls:
        if filename.lower().endswith(".yml"):
            with open(os.path.join(input_dir, filename), 'r') as f:
                yd = yaml.load(f, Loader=yaml.FullLoader)

                # Iterate over IP
                for ip_name, value in yd['peripheral_channel_dict'].items():

                    # Perform any replacements
                    if ip_name in CONST_GEN_DATA_REPLACEMENTS:
                        ip_name = CONST_GEN_DATA_REPLACEMENTS[ip_name]

                    # Check if a module with this IP name exists
                    if ip_name.lower() in mdfs:
                        all_used_ids += mdfs[ip_name.lower()]

    # Remove duplicates
    all_used_ids = list(set(all_used_ids))

    # This loop will generate the list of available and unavailable module id's for each MCU
    for filename in mcu_ymls:
        if filename.lower().endswith(".yml"):
            with open(os.path.join(input_dir, filename), 'r') as f:
                yd = yaml.load(f, Loader=yaml.FullLoader)

                # Init list with missing IP entries that exist on all MCUs
                good_ids = CONST_GEN_DATA_MISSING_IP.copy()

                mcu_name = filename.lower().split('.')[0]

                # Get one part number we can use for testing. If a static choice is given use it. Otherwise choose first one in list.
                if filename in CONST_STATIC_PART_NUMBER:
                    mcu_pn = CONST_STATIC_PART_NUMBER[filename]
                else:
                    mcu_pn = yd['part_numbers'][0]['name']

                # Iterate over IP
                for ip_name, value in yd['peripheral_channel_dict'].items():

                    # Perform any replacements
                    if ip_name in CONST_GEN_DATA_REPLACEMENTS:
                        ip_name = CONST_GEN_DATA_REPLACEMENTS[ip_name]

                    # Check if a module with this IP name exists
                    if ip_name.lower() in mdfs:
                        good_ids += mdfs[ip_name.lower()]

                # Remove duplicates
                good_ids = list(sorted(set(good_ids)))

                # Get MDFs that are NOT available for this MCU
                bad_ids = sorted(set(all_used_ids) - set(good_ids))

                # If the MCU uses the CANFD (Lite) MDF, then add canfd_on_canfdlite instead of canfd
                if is_canfdlite_mcu(mdf_dir, mcu_name):
                    good_ids = list(map(lambda x: x.replace('canfd_on_canfd', 'canfd_on_canfdlite'), good_ids))
                    bad_ids += ['module.driver.canfd_on_canfd']

                # Write output to specific MCU dir
                mcu_dir = os.path.join('ra', 'fsp', 'src', 'bsp', 'mcu', mcu_name, '.xml_tests')

                # Create .xml_tests dir if it does not exist already
                if not os.path.exists(mcu_dir):
                    os.makedirs(mcu_dir)

                # Create generated tests file
                gen_file_path = os.path.join(mcu_dir, 'gen_ip_availability.yml')

                output = test_template.render(generated=True, mcu=mcu_name, mcu_pn=mcu_pn, good_ids=good_ids, bad_ids=bad_ids)

                with open(gen_file_path, 'w', newline='\r\n') as f:
                    print('Writing - ' + gen_file_path)
                    f.write(output)

                # Create file for manual availability checks IF IT DOES NOT ALREADY EXIST
                manual_file_path = os.path.join(mcu_dir, 'manual_ip_availability.yml')

                if not os.path.exists(manual_file_path):
                    output = test_template.render(generated=False, mcu=mcu_name, mcu_pn=mcu_pn, good_ids=[], bad_ids=[])

                    with open(manual_file_path, 'w', newline='\r\n') as f:
                        print('Writing - ' + manual_file_path)
                        f.write(output)
    # Only create/output the not_checked_ids if building all mcus (and we're not on pipeline, so becomes noise)
    if mcu is None:
        not_checked_ids = set(all_ids) - set(all_used_ids)

        print("These IDs were not tested.")
        for id in sorted(not_checked_ids):
            print(" -" + id)

if __name__ == "__main__":
    import argparse

    # Allow command line arguments
    parser = argparse.ArgumentParser(
        description = "Map IP on MCU to modules. This is used to check module availability in e2studio",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter
    )

    parser.add_argument("-i",
        dest="input",
        help="Directory with generated MCU YML files",
        metavar="INPUT_MCU_DATA",
        default=os.path.join('scripts', 'mcu_xml_gen', 'data', 'generated', 'device'),
    )

    parser.add_argument("-d",
        dest="mdf_dir",
        help="Directory where module description files (ie: e2studio XMLs) are stored.",
        metavar="E2_MDF_DIR",
        default=os.path.join('data', '.module_descriptions'),
    )

    parser.add_argument("-m",
        dest="mcu",
        help="Specify single MCU to parse (all will be parsed if not provided).",
        metavar="MCU",
        default=None,
    )
    # Get command line arguments
    arguments = parser.parse_args()

    parse(arguments.input, arguments.mdf_dir, arguments.mcu)
