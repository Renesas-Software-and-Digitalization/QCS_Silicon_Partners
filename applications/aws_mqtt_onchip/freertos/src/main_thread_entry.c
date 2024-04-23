#include <main_thread.h>
#include "user.h"
#include "common_utils.h"
#include "sensor_thread.h"
@INCLUDES

/******************************************************************************
 User function prototype declarations
*******************************************************************************/
static void ftoa( float n, char *res, int afterpoint);
static void reverse(char* str, int len);
static int intToStr(int x, char str[], int d);
@FN_DECLARATION

/******************************************************************************
 User Macros
*******************************************************************************/
#define WIDTH_64                                          (64)
#define CONNECT_TIMEOUT                                   (5000)
#define SUB_TOPIC_FILTER_COUNT                            ( 1 )
#define PUBLISH_MAX_NUMBER                                ( 3 )
#define RECV_MAX_NUMBER                                   ( 4 )

/******************************************************************************
 User global variables
*******************************************************************************/
extern QueueHandle_t g_sensor_queue;
extern TaskHandle_t sensor_thread;
static char g_led_topic[WIDTH_64]   = IO_USERNAME;
static char pub_topic[WIDTH_64]     = {0};
const char *pSubTopics[SUB_TOPIC_FILTER_COUNT] = {
                                                  USER_LED_TOPIC
};
mqtt_onchip_da16xxx_pub_info_t pubTopics[SENSOR_DATA_COUNT];
mqtt_onchip_da16xxx_sub_info_t subTopics[SUB_TOPIC_FILTER_COUNT];
mqtt_onchip_da16xxx_instance_ctrl_t g_rm_mqtt_onchip_da16xxx_instance;
uint8_t receive_cnt;
/* Setup Access Point connection parameters */
WIFINetworkParams_t net_params =
{
 .ucChannel                  = 0,
 .xPassword.xWPA.cPassphrase = WIFI_PWD,
 .ucSSID                     = WIFI_SSID,
 .xPassword.xWPA.ucLength = sizeof(WIFI_PWD),
 .ucSSIDLength            = sizeof(WIFI_SSID),
 .xSecurity               = eWiFiSecurityWPA2,
};
@GLOBAL_VARIABLES


void mqtt0_callback (mqtt_onchip_da16xxx_callback_args_t * p_args)
{
    int led_state = 0;

    char * ptr = strstr(p_args->p_topic, USER_LED_TOPIC);
    if (ptr != NULL)
    {
        led_state = strtol((char *) p_args->p_data, NULL, 10);
        printf("recv count %d out of %d\r\n",(receive_cnt + 1), RECV_MAX_NUMBER);
        if (led_state)
        {
            //Turn on Green LED
            LED_Toggle(GREEN_LED, ON);
            receive_cnt++;

        }
        else
        {
            //Turn off Green LED
            LED_Toggle(GREEN_LED, OFF);
            receive_cnt++;
        }
    }
}

/* MQTT Thread entry function */
/* pvParameters contains TaskHandle_t */
void main_thread_entry (void * pvParameters)
{
    FSP_PARAMETER_NOT_USED (pvParameters);
    vTaskDelay(1000);

    printf("Starting Wi-Fi connection...\r\n");

    /* Open connection to the Wifi Module */
    if(eWiFiSuccess != WIFI_On())
    {
        printf("Wi-Fi Open failed\r\n");
        halt_func();
    }

    /* Connect to the Access Point */
    if(eWiFiSuccess != WIFI_ConnectAP(&net_params))
    {
        printf("Wi-Fi Connect failed\r\n");
        halt_func();
    }

    printf("Wi-Fi connection successful!\r\n");

    mqtt_onchip_da16xxx_cfg_t g_mqtt_onchip_da16xxx_usrcfg = g_mqtt_onchip_da16xxx_cfg;
    g_mqtt_onchip_da16xxx_usrcfg.p_host_name = (char*) &EXAMPLE_MQTT_HOST;
    /* Initialize the MQTT Client connection */
    if(FSP_SUCCESS != RM_MQTT_DA16XXX_Open(&g_rm_mqtt_onchip_da16xxx_instance, &g_mqtt_onchip_da16xxx_usrcfg))
    {
        printf("MQTT Open failed\r\n");
        halt_func();
    }

    printf("MQTT setup successful!\r\n");

    vTaskDelay(1000);

    /* Subscribe to topics */
    strcat(g_led_topic, *pSubTopics);
    subTopics[0].qos =MQTT_ONCHIP_DA16XXX_QOS_0;
    subTopics[0].p_topic_filter = g_led_topic;
    subTopics[0].topic_filter_length= (uint16_t)strlen(g_led_topic);
    /* Subscribe to MQTT topics to be received */
    if(FSP_SUCCESS != RM_MQTT_DA16XXX_Subscribe(&g_rm_mqtt_onchip_da16xxx_instance, subTopics, 1))
    {
        printf("MQTT Subscribe failed\r\n");
        halt_func();
    }
    printf("MQTT Subscribe to %s\r\n", g_led_topic);

    vTaskDelay(1000);
    /* Connect to the MQTT Broker */
    if(FSP_SUCCESS != RM_MQTT_DA16XXX_Connect(&g_rm_mqtt_onchip_da16xxx_instance, CONNECT_TIMEOUT))
    {
        vTaskDelay(1000);
        if(FSP_SUCCESS != RM_MQTT_DA16XXX_Connect(&g_rm_mqtt_onchip_da16xxx_instance, CONNECT_TIMEOUT))
        {
            printf("MQTT Connect failed\r\n");
            halt_func();
        }
    }

    @MAIN_INITIALIZATION

    vTaskResume(sensor_thread);

    for (int i = 0; i < PUBLISH_MAX_NUMBER; i++)
    {
        float sens_data[SENSOR_DATA_COUNT] = {0};
        char sensdata[SENSOR_DATA_COUNT][10] ={0};

        xQueueReceive(g_sensor_queue, &sens_data, portMAX_DELAY);
        printf("\r\nPublishing %d out of %d\r\n",(i+1),PUBLISH_MAX_NUMBER);
        for (uint8_t pub = 0; pub < SENSOR_DATA_COUNT; pub++)
        {
        	memset(pub_topic, 0x00, WIDTH_64);
        	snprintf(pub_topic, WIDTH_64, IO_USERNAME"%s", pPubTopics[pub]);
            ftoa(sens_data[pub], sensdata[pub],2);

        	pubTopics[pub].qos =MQTT_ONCHIP_DA16XXX_QOS_0,
            pubTopics[pub].p_topic_name = pub_topic;
            pubTopics[pub].topic_name_Length = (uint16_t)strlen(pub_topic);
            pubTopics[pub].p_payload = sensdata[pub];
            pubTopics[pub].payload_length = strlen(pubTopics[pub].p_payload);
            printf("Topic:%s\r\ndata: %s\r\n",pubTopics[pub].p_topic_name, pubTopics[pub].p_payload);
            /* Publish data to the MQTT Broker */
            if(FSP_SUCCESS != RM_MQTT_DA16XXX_Publish(&g_rm_mqtt_onchip_da16xxx_instance, &pubTopics[pub]))
            {
                printf("MQTT Publish 1 failed\r\n");
                halt_func();
            }
            vTaskDelay(1000);
        }
    }

    printf("\r\nReceiving...\r\n");
    do
    {
        /* Loop to receive data from the MQTT Broker */
        RM_MQTT_DA16XXX_Receive(&g_rm_mqtt_onchip_da16xxx_instance, &g_mqtt_onchip_da16xxx_cfg);
    } while (receive_cnt < RECV_MAX_NUMBER);


    /* Disconnect from the MQTT Broker */
    if(FSP_SUCCESS != RM_MQTT_DA16XXX_Disconnect(&g_rm_mqtt_onchip_da16xxx_instance))
    {
        printf("MQTT Disconnect failed\r\n");
        halt_func();
    }


    /* Close the MQTT Client module */
    if(FSP_SUCCESS != RM_MQTT_DA16XXX_Close(&g_rm_mqtt_onchip_da16xxx_instance))
    {
        printf("MQTT Close failed\r\n");
        halt_func();
    }
    printf("\r\nMQTT Disconnected\r\n");

    while (1)
    {
        /* Go idle */
        vTaskDelay(3000);
        @MAIN_LOOP
    }
}

// Reverses a string 'str' of length 'len'
static void reverse(char* str, int len)
{
    int i = 0, j = len - 1, temp;
    while (i < j) {
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
    while (x) {
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

static void ftoa( float n, char *res, int afterpoint)
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
        if (0==i)
        {
            res[i]='0';
            res[i+1]='.';
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