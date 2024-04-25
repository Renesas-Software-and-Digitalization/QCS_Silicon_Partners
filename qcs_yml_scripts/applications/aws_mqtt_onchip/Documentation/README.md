## Overview
This application project showcases the MQTT Cloud connectivity use case using the AWS IoT.
In this application project, the RA MCU device will read sensor data through the I2C interface and publish
the sensor data through the DA16200 Wi-Fi module using its onchip MQTT stack. It will first publish three times each sensor data and then will wait for user to publish 4 times on the LED topic before closing the MQTT connection.


#### Instructions For Running the Application
1. Follow the AWS IoT instructions to create a Thing.
2. At this stage, it is assumed, that the user generate/build the QCStudio project.
3. Open the user.h and add user Wifi SSID and Password, Username and AWS endpoint by modifying the following MACROS:
```c
#define WIFI_SSID               "USER-WIFI-SSID"
#define WIFI_PWD                "USER-WIFI-PASSWORD"

#define IO_USERNAME             "USERNAME"

#define EXAMPLE_MQTT_HOST       ("AWS-ENDPOINT")
```
4. Open the certificate.h and provide the server certificate, user's client certificate, user's client private key by modifying the following MACROS:

```c
/* server certificate */
#define ROOT_CA                                          \
"-----BEGIN CERTIFICATE-----\n" \
"-----END CERTIFICATE-----\n"


/* client certificate */
#define CLIENT_CERT                                   \
"-----BEGIN CERTIFICATE-----\n" \
"-----END CERTIFICATE-----\n"


/* client private key */
#define PRIVATE_KEY                                   \
"-----BEGIN RSA PRIVATE KEY-----\n" \
"-----END RSA PRIVATE KEY-----\n"
```
5. Rebuild your QCStudio project. Click on the spanner icon (4th icon from top left) drop-down menu,
select Build QCStudio project option.
6. After a successful build, download the .srec file from the project debug folder to your local PC.
7. Open the J-Flash Lite application on your local PC, and choose the MCU that matches your QCStudio project.
8. Select the .srec file and program the kit
9. At this stage, the MCU kit will start publishing sensor data to your AWS endpoint.
10. Open your AWS IoT and click on MQTT Test client. 
11. Check the user.h and subscribe for the topics avaliable for your sensor.\
e.g: Sensor HS3001:\
Topics:\
*USERNAME*/feeds/temperature\
*USERNAME*/feeds/humidity
12. Publish on the LED topic to control the LED state.\
Topics:\
*USERNAME*/feeds/led -> 0 (Turn LED off)\
*USERNAME*/feeds/led -> 1 (Turn LED on)
13. User can configure Segger RTT Viewer to view the project log.

#### Troubleshooting
- Wifi not connecting:
    - Check your wifi SSID and Password.   
    - Ensure proper connection of the Sensor interposer. The sticker should be positioned on the bottom side.
- AWS not connecting:
    - Check your credentials and AWS IoT endpoint.
    - Verify if your US159-DA16200MEVZ nave the latest SDK firmware. (Refer to "SDK Update Guide DA16200/DA16600" for instructions). 

## References
1. AWS IoT: https://aws.amazon.com/iot-core/
2. J-Link / J-Trace Downloads: https://www.segger.com/downloads/jlink/
3. SDK Update Guide DA16200/DA16600: https://www.renesas.com/us/en/document/apn/da16200da16600-sdk-update-guide
4. Interposer Board for Pmod™ Type 2A/3A to 6A: https://www.renesas.com/us/en/products/microcontrollers-microprocessors/us082-interpevz-interposer-board-pmod-type-2a3a-6a-renesas-quick-connect-iot