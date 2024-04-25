## Overview
This application project showcases the MQTT Cloud connectivity use case using the Adafruit Cloud server.
In this application project, the RA MCU device will read sensor data through the I2C interface and publish
the sensor data through the DA16200 Wi-Fi module.


### Instructions For Running the Application
1. Create a user account for adafruit.io, and configure the IO dashboard using the following steps(Illustrated to create a feed for temperature, humidity and LED feeds).

    1.1 Open a web browser and go to https://io.adafruit.com. Use the *Get Started for Free* link in the topright
        corner to register an account.
    
    <div style="text-align: center;">

    ![image description](images/adafruit_image1.png)
    
    </div>

    1.2 Enter your desired user credentials and select *Create Account*. Below is an example ***(do not copy)***.

    <div style="text-align: center;">

    ![image description](images/adafruit_image2.png)

    </div>

    1.3 On the upper left, Select the *IO* tab. From there, select the *Feeds* tab.
    
    <div style="text-align: center;">

    ![image description](images/adafruit_image3.png)

    </div>

    1.4 Select *New Feed*, enter a name and select *Create*. Do this three times and create the following
    *Feeds*:

    * **Temperature**
    * **Humidity**
    * **LED**

    <div style="text-align: center;">

    ![image description](images/adafruit_image4.png)

    ![image description](images/adafruit_image5.png)

    ![image description](images/adafruit_image6.png)

    </div>

    1.5 Select the *Humidity* feed to view it. In the upper right, select the *Feed Info* button. It will show the
    MQTT topic associated with the field. Confirm that this topic matches with the topics below
    (excluding the user). Do this for the *LED* and *Temperature* feeds as well.

    <div style="text-align: center;">

    ![image description](images/adafruit_image7.png)

    ![image description](images/adafruit_image8.png)

    </div>

    1.6 Now select the *Dashboards* tab. Select the *New Dashboard* button and provide a name to your
    Dashboard, then select *Create*. Select the dashboard you just created.

    <div style="text-align: center;">

    ![image description](images/adafruit_image9.png)

    ![image description](images/adafruit_image10.png)

    ![image description](images/adafruit_image11.png)

    ![image description](images/adafruit_image12.png)

    </div>

    1.7 In the upper right, select the gear icon and select *Create New Block*. Click on the *Gauge* button in
    the top-right corner.

    ![image description](images/adafruit_image13.png)

    1.8 Then, select the *Humidity* feed to link its value to this gauge and select *Next Step*. Modify the
    block with the following settings and select *Create block*.
    
    * **Block Title**&emsp;&emsp;&emsp;Humidity
    * **Gauge Label**&emsp;&emsp;%
    
    <div style="text-align: center;">

    ![image description](images/adafruit_image14.png)

    </div>

    1.9 Create another *Gauge* block and select *Temperature* as the feed. Modify the block with the
    following settings and select *Create block*.

    * **Block Title**&emsp;&emsp;&emsp;Temperature
    * **Gauge Label**&emsp;&emsp;Celsius

    1.10 Finally, create a new block and select the *Toggle* block (instead of Gauge) and select *LED* as the
    feed. Modify the block with the following settings and select *Create block*.

    * **Block Title**&emsp;&emsp;&emsp;&ensp;&nbsp;LED
    * **Button On Value**&emsp;1
    * **Button Off Value**&emsp;0
    
    1.11 In the upper right, select the gear icon and select *Edit Layout*. Click and drag the *Humidity*,
    *Temperature*, and *LED* blocks such that they are aligned horizontally (instead of the default
    vertical alignment) and select *Save Layout*.
    
    <div style="text-align: center;">

    ![image description](images/adafruit_image15.png)

    </div>

    1.12 In the upper right, select the yellow key icon to view your Username and Adafruit IO key. Copy
    and paste the *IO_USERNAME* and *IO_KEY* values into *user.h*.

    <div style="text-align: center;">

    ![image description](images/adafruit_image16.png)

    ![image description](images/adafruit_image17.png)
    
    </div>

2. Refer to user.h under the src directory and make sure to match the feed name and feed key in your IO dashboard.
3. At this stage, it is assumed, that the user generate/build the QCStudio project.
Open the user.h file found under the src directory and update the following macros,
which includes Wi-Fi access point credentials, Adafruit account user name, and IO key.
```c
#define WIFI_SSID               "USER-WIFI-SSID"
#define WIFI_PWD                "USER-WIFI-PASSWORD"

#define IO_USERNAME             "USERNAME"
#define IO_KEY                  "ADAFRUIT-IO-KEY"
```
4. Rebuild your QCStudio project. Click on the spanner icon (4th icon from top left) drop-down menu,
select Build QCStudio project option.
5. After a successful build, download the .srec file from the project debug folder to your local PC.
6. Open the J-Flash Lite application on your local PC, and choose the MCU that matches your QCStudio project.
7. Select the .srec file and program the kit.
8. At this stage, the MCU kit will start publishing sensor data to your IO dashboard.
9. User can configure Segger RTT Viewer to view the project log.

#### Troubleshooting
- Wifi not connecting:
    - Check your wifi SSID and Password.   
    - Ensure proper connection of the Sensor interposer. The sticker should be positioned on the bottom side.
- Adafruit not connecting:
    - Check if your Adafruit username and key match with your user.h.
    - Verify if your US159-DA16200MEVZ nave the latest SDK firmware. (Refer to "SDK Update Guide DA16200/DA16600" for instructions).   

## References
1. Adafruit Io: https://io.adafruit.com/
2. J-Link / J-Trace Downloads: https://www.segger.com/downloads/jlink/
3. SDK Update Guide DA16200/DA16600: https://www.renesas.com/us/en/document/apn/da16200da16600-sdk-update-guide
4. Interposer Board for Pmod™ Type 2A/3A to 6A: https://www.renesas.com/us/en/products/microcontrollers-microprocessors/us082-interpevz-interposer-board-pmod-type-2a3a-6a-renesas-quick-connect-iot