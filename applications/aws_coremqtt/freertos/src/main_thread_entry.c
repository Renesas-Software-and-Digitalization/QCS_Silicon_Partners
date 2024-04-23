/***********************************************************************************************************************
 * DISCLAIMER
 * This software is supplied by Renesas Electronics Corporation and is only intended for use with Renesas products. No
 * other uses are authorized. This software is owned by Renesas Electronics Corporation and is protected under all
 * applicable laws, including copyright laws.
 * THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
 * THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED. TO THE MAXIMUM
 * EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES
 * SHALL BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO THIS
 * SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 * Renesas reserves the right, without notice, to make changes to this software and to discontinue the availability of
 * this software. By using this software, you agree to the additional terms and conditions found by accessing the
 * following link:
 * http://www.renesas.com/disclaimer
 *
 * Copyright (C) 2020 Renesas Electronics Corporation. All rights reserved.
 ***********************************************************************************************************************/
#include "hal_data.h"
#include "main_thread.h"
#include "user.h"
#include "transport_interface.h"
#include <mqtt_wrapper.h>
#include "core_mqtt.h"
#include "sensor_thread.h"
@INCLUDES

/******************************************************************************
 User function prototype declarations
*******************************************************************************/
static int platform_init(void);
static void init_network_interface(void);
void halt_func(void);
static void reverse(char *str, int len);
static int intToStr(int x, char str[], int d);
static void ftoa(float n, char *res, int afterpoint);
@FN_DECLARATION

/******************************************************************************
 User Macros
*******************************************************************************/
#define WIDTH_64 (64)

/******************************************************************************
 User global variables
*******************************************************************************/
extern TaskHandle_t sensor_thread;
extern TaskHandle_t sub_thread;
extern QueueHandle_t g_sensor_queue;
NetworkContext_t xNetworkContext;
MQTTContext_t mqttContext;
TlsTransportParams_t transport_params;

struct NetworkContext
{
    TlsTransportParams_t *pParams;
};

static char g_led_topic[WIDTH_64] = IO_USERNAME;
static char pub_topic[WIDTH_64] = {0};
static char pub_message[WIDTH_64] = {0};

const char *pSubTopics[SUB_TOPIC_FILTER_COUNT] = {
    USER_LED_TOPIC};

@GLOBAL_VARIABLES

/* Main Thread entry function */
/* pvParameters contains TaskHandle_t */
void main_thread_entry(void *pvParameters)
{
    int status = EXIT_SUCCESS;

    status = platform_init();
    if (EXIT_SUCCESS != status)
    {
        halt_func();
    }

    BaseType_t mqtt_status = 0;

    aws_mqtt_init();

    printf("\r\nMQTT Init Successful\r\n");

    mqtt_status = aws_mqtt_connect(&mqttContext, &xNetworkContext);
    if (pdPASS != mqtt_status)
    {
        printf("\r\n aws_mqtt_connect failed!!!\r\n");
        halt_func();
    }

    /* Subscribe to topics */
    strcat(g_led_topic, USER_LED_TOPIC);
    mqtt_status = aws_mqtt_subscribe(&mqttContext, g_led_topic);
    if (pdPASS != mqtt_status)
    {
        printf("\r\n aws_mqtt_subscribe failed!!!\r\n");
        halt_func();
    }

    @MAIN_INITIALIZATION

    vTaskResume(sub_thread);
    vTaskResume(sensor_thread);

    /* TODO: add your own code here */
    while (1)
    {
        float sens_data[SENSOR_DATA_COUNT] = {0};
        char sensdata[SENSOR_DATA_COUNT][10] = {0};

        xQueueReceive(g_sensor_queue, &sens_data, portMAX_DELAY);

        /* Place message in the sensor queue */
        for (int i = 0; i < SENSOR_DATA_COUNT; i++)
        {
            snprintf(pub_topic, WIDTH_64, IO_USERNAME "%s", pPubTopics[i]);
            ftoa(sens_data[i], sensdata[i], 2);
            snprintf((char *)pub_message, WIDTH_64, "%s", sensdata[i]);
            printf("Topic:%s\r\ndata: %s\r\n", pub_topic, pub_message);
            mqtt_status = aws_mqtt_publish(&mqttContext, pub_topic, pub_message);
            if (pdPASS != mqtt_status)
            {
                printf("\r\n aws_mqtt_subscribe failed!!!\r\n");
                halt_func();
            }
            @MAIN_LOOP
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

    FSP_PARAMETER_NOT_USED(pvParameters);
}

/*Function to initialize platform specific components */
static int platform_init(void)
{
    int status = EXIT_SUCCESS;
    fsp_err_t fsp_err = FSP_SUCCESS;

    /* Initialize platform specific crypto initialization */
    if (mbedtls_platform_setup(NULL) != 0)
    {
        printf("\r\nmbedtls_platform_setup Failed!!!\r\n");
        status = EXIT_FAILURE;
    }

    if (EXIT_SUCCESS == status)
    {
        status = psa_crypto_init();
        if (status != EXIT_SUCCESS)
        {
            printf("\r\npsa_crypto_init failed!!!, ret: %d\r\n", status);
            status = EXIT_FAILURE;
        }

        /* Set up Wi-Fi connectivity */
        init_network_interface();

        /* Initialize LittleFS */
        fsp_err = RM_LITTLEFS_FLASH_Open(&g_rm_littlefs0_ctrl, &g_rm_littlefs0_cfg);
        if (FSP_SUCCESS != fsp_err)
            status = EXIT_FAILURE;

        fsp_err = lfs_mount(&g_rm_littlefs0_lfs, &g_rm_littlefs0_lfs_cfg);
        if (FSP_SUCCESS != fsp_err)
        {
            fsp_err = lfs_format(&g_rm_littlefs0_lfs, &g_rm_littlefs0_lfs_cfg);
            if (FSP_SUCCESS != fsp_err)
                status = EXIT_FAILURE;
            fsp_err = lfs_mount(&g_rm_littlefs0_lfs, &g_rm_littlefs0_lfs_cfg);
            if (FSP_SUCCESS != fsp_err)
                status = EXIT_FAILURE;
        }
    }

    return status;
}
WIFIIPConfiguration_t ipconfg;
/* Function to initialize network interface */
static void init_network_interface(void)
{
    WIFINetworkParams_t net_params;

    /* Turn ON Wi-Fi */
    if (WIFI_On() != eWiFiSuccess)
    {
        printf("DA16200 Init Failed!!!\r\n");
        halt_func();
    }

    /* Configure network parameters */
    strncpy((char *)net_params.ucSSID, WIFI_SSID, (uint8_t)(strlen(WIFI_SSID) + 1));
    net_params.ucSSIDLength = (uint8_t)(strlen((char *)net_params.ucSSID) + 1);
    strncpy((char *)net_params.xPassword.xWPA.cPassphrase, WIFI_PWD, (uint8_t)(strlen(WIFI_PWD) + 1));
    net_params.xPassword.xWPA.ucLength = (uint8_t)(strlen(net_params.xPassword.xWPA.cPassphrase) + 1);
    net_params.ucChannel = 0;
    net_params.xSecurity = 3;

    printf("\r\nConnecting to Wi-Fi...\r\n");

    /* Connect to WiFi Access Point */
    if (WIFI_ConnectAP(&net_params) != eWiFiSuccess)
    {
        printf("Connection failed!!!\r\n");
        halt_func();
    }

    printf("Connection successful!!!\r\n");

    if (WIFI_GetIPInfo(&ipconfg) != eWiFiSuccess)
    {
        printf("Failed to get IP info!!!\r\n");
        halt_func();
    }

    printf("\r\n IP Addr: %d.%d.%d.%d\r\n", ((uint8_t *)ipconfg.xIPAddress.ulAddress)[0], ((uint8_t *)ipconfg.xIPAddress.ulAddress)[1], ((uint8_t *)ipconfg.xIPAddress.ulAddress)[2], ((uint8_t *)ipconfg.xIPAddress.ulAddress)[3]);
}

// Reverses a string 'str' of length 'len'
static void reverse(char *str, int len)
{
    int i = 0, j = len - 1, temp;
    while (i < j)
    {
        temp = str[i];
        str[i] = str[j];
        str[j] = (char)temp;
        i++;
        j--;
    }
}

// Converts a given integer x to string str[].
// d is the number of digits required in the output.
// If d is more than the number of digits in x,
// then 0s are added at the beginning.
static int intToStr(int x, char str[], int d)
{
    int i = 0;
    while (x)
    {
        str[i++] = (char)(x % 10) + '0';
        x = x / 10;
    }

    // If number of digits required is more, then
    // add 0s at the beginning
    while (i < d)
        str[i++] = '0';

    reverse(str, i);
    str[i] = '\0';
    return i;
}

static void ftoa(float n, char *res, int afterpoint)
{
    // Check if the number is negative
    if (n < 0)
    {
        res[0] = '-';
        res++;
        n = -n;
    }

    // Extract integer part
    int ipart = (int)n;

    // Extract floating part
    float fpart = n - (float)ipart;

    // convert integer part to string
    int i = intToStr(ipart, res, 0);

    // check for display option after point
    if (afterpoint != 0)
    {
        if (0 == i)
        {
            res[i] = '0';
            res[i + 1] = '.';
            // Get the value of fraction part upto given no.
            // of points after dot. The third parameter
            // is needed to handle cases like 233.007
            fpart = fpart * (float)pow(10, afterpoint);
            intToStr((int)fpart, res + i + 2, afterpoint);
        }
        else
        {
            res[i] = '.'; // add dot
            // Get the value of fraction part upto given no.
            // of points after dot. The third parameter
            // is needed to handle cases like 233.007
            fpart = fpart * (float)pow(10, afterpoint);
            intToStr((int)fpart, res + i + 1, afterpoint);
        }
    }
}

@FN_IMPLEMENTATION