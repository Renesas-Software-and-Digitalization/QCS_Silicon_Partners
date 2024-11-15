## Overview
The objective of this repo is to provide some reference applications where partner can add its sensor modules to the current QC Studio enviroment.

## Tools
To use this repo users must have:

Hardware
* EK-RA6M4
* US082-INTERPEVZ - required for I2C PMODs
* USB-to-UART PMOD or equivalent adapter (only GND, TX and RX connections are required)

Software
* FSP 5.3.0 platform installer bundle (e2_studio + FSP)

### Instructions for contribuiting
1. Fork the Project
2. Create your Feature Branch (`git checkout -b feature/new_module`)
3. Import the reference projects (sensordummy) on your e2studio using the `Rename and import Existing C/C++ Project into Workspace`.
[Changing an e² studio project name](https://en-support.renesas.com/knowledgeBase/21225277#:~:text=On%20the%20project%20import%20wizard,name%20in%20the%20current%20workspace.)
    * module name: sensor1
        * ek_ra6m4_generic_uart_baremetal_serial
4. Add your sensor driver to a folder under src/sensor. For instance, if your sensor is xyz, its driver files (.c and .h) shall be
in the folder src/sensor/xyz. Make sure you add the path to the C compiler include path within e2_studio. Note that the application comes
with two sensors already implemented as examples: HS3001 is Renesas temperature and humidity sensor and dummy is an example of how to interface
to our I2C FSP driver. Please remove these sensors and drivers for testing your own drivers! A quick and easy way to remove the drivers is by
removing the respective instances in the sm_define_sensors.inc file.
5. Add the thin-layer source file that implements Renesas' Sensor Manager API. These files shall be named after the new sensor so, for instance,
if the new sensor is xyz, the thin-layer files shall be named **xyz_sensor.c** and **xyz_sensor.h**. These files must be located preferrably
under src/sensor.
6. Update the sm_define_sensors.inc to include the type of sensor and unit (DEFINE_SENSOR_TYPE), sensor driver name (this is the thin-layer name
used for the new sensor, for instance, xyz_sensor) and create new sensor instances, one for each channel on the new sensor (so if for instance
the new sensor measures voltage and current, two new instances must be added). Each instance shall map to a unique channel and that channel
shall be properly configured in the thin-layer driver. The instance definition also includes fields for sensor scaling, so if the new sensor or
driver applies any scaling, use the multiplier, divider and offset factors to match the scaling properly. The last field is the reading interval.
Set it to zero if the sensor uses its own internal state machine for sequencing, otherwise set the interval to the standard measurement rate for
that sensor (usually 1000 for once a second).
7. Make sure you include the logging headers as part of all your C source code:
![Logging](/images/readme_data/logging.PNG)
For local-only logging, comment out the **include "log_disabled.h"** and uncomment the desired logging level. For global logging control please
update the GLOBAL_LOGGING_LEVEL defined in the **global_logging_definitions.h** file under src/qc-middleware/common_utils. When defined, that
symbol overrides any local logging setting!
Please refer to [this file](https://github.com/fabiorenesas/QCS_Silicon_Partners/blob/feature/sm/applications/ek_ra6m4_generic_uart_baremetal_serial/src/qc-middleware/common_utils/logger.h#L16) for the usage of logging functions.
8. **DO NOT modify** any files inside qc-middleware or any of the main application files (hal_entry.c, main_application.*, uart.*) if for some reason
your code requires changing any of those files, contact the Renesas team and make sure your PR clarifies why that is needed.
9. Create a new folder on images with your company and sensor name, then add the 3D rendered module image.
10. Create a new folder on docs with your company and sensor name, then add the module documentation.
11. Commit your Changes (`git commit -m 'Add a new_module'`).
12. Push to the Branch (`git push origin feature/new_module`).
13. Create a pull request to the QCS_Silicon Partner repository.

## References
1. EK-RA6M4: https://www.renesas.com/us/en/products/microcontrollers-microprocessors/ra-cortex-m-mcus/ek-ra6m4-evaluation-kit-ra6m4-mcu-group
2. FSP 5.3.0 platform installer (setup_fsp_v5_3_0_e2s_v2024-04): https://github.com/renesas/fsp/releases/tag/v5.3.0
3. Interposer Board for Pmod™ Type 2A/3A to 6A (US082-INTERPEVZ): https://www.renesas.com/us/en/products/microcontrollers-microprocessors/us082-interpevz-interposer-board-pmod-type-2a3a-6a-renesas-quick-connect-iot
