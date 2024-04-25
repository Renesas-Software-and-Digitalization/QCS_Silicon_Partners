## Overview
This application project showcases BLE connectivity use case. In this application, the RA MCU kit
will act as BLE peripheral and user's cellphone/tablet will act as BLE Central.
In this application project, RA MCU device will read sensor data through I2C interface and publish
the sensor data through DA14531 BLE module.


#### Instructions For Running the Application
1. Build your QCStudio project. Click on spanner icon (3rd icon from top left).
2. After successful build, download the .srec file from the project debug folder to your local PC.
3. Open J-Flash Lite application in your local PC, choose the MCU that matches in your QCStudio project.
4. Select the .srec file and program the kit.
5. At this stage, the MCU kit will start BLE Advertising packet and the green LED will blink.
6. User can configure Segger RTT Viewer to view the project log.
7. Open the Quick-Connect Mobile Sandbox application on cellphone/tablet, scan for BLE device and connect to RA MCU Kit.
8. After connection to the BLE peripheral, user can check the sensor data published on the mobile application.

#### Troubleshooting
- Green LED not blinking:
    - Ensure proper connection of the Sensor interposer. The sticker should be positioned on the bottom side.
- Nothing is displaying on the Quick-Connect Mobile Sandbox:
    - Verify if your US159-DA14531EVZ is equipped with the latest DA14531-GTL-FSP firmware. (Refer to "US159-DA14531EVZ Firmware Upgrade" for instructions).

## References
1. Apple Store Quick-Connect Mobile Sandbox: https://apps.apple.com/us/app/quick-connect-mobile-sandbox/id6448645028
2. Google Play Quick-Connect Mobile Sandbox: https://play.google.com/store/apps/details?id=com.renesas.sst.qcsandbox&pli=1
3. J-Link / J-Trace Downloads: https://www.segger.com/downloads/jlink/
4. US159-DA14531EVZ Firmware Upgrade: https://lpccs-docs.renesas.com/US159-DA14531EVZ_Firmware_Upgrade/index.html
5. Interposer Board for Pmod™ Type 2A/3A to 6A: https://www.renesas.com/us/en/products/microcontrollers-microprocessors/us082-interpevz-interposer-board-pmod-type-2a3a-6a-renesas-quick-connect-iot
