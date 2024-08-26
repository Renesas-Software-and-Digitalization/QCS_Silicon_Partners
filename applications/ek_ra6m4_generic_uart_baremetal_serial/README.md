## Overview
This application project publishes data read from one or more sensors to the serial port.


#### Instructions For Running the Application
1. Build your QCStudio project. Click on spanner icon (3rd icon from top left).
2. After successful build, download the .srec file from the project debug folder to your local PC.
3. Open J-Flash Lite application in your local PC, choose the MCU that matches in your QCStudio project.
4. Select the .srec file and program the kit.
5. Connect a serial to USB adapter to PMOD2 (RA pins TXD0 and RXD0 plus ground)
6. The user can configure Segger RTT Viewer to view the project log.
7. Use a serial terminal program such as Teraterm configured for 115200bps.
8. Run the application and watch the sensor data get published to the terminal


## References
1. J-Link / J-Trace Downloads: https://www.segger.com/downloads/jlink/
