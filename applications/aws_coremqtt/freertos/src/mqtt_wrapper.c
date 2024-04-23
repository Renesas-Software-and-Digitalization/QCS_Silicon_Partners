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
#include <mqtt_wrapper.h>
#include "common_data.h"

/**
 * @brief Global entry time into the application to use as a reference timestamp
 * in the #prvGetTimeMs function. #prvGetTimeMs will always return the difference
 * between the current time and the global entry time. This will reduce the chances
 * of overflow for the 32 bit unsigned integer used for holding the timestamp.
 */
static uint32_t ulGlobalEntryTimeMs;

/**
 * @brief Packet Identifier generated when Publish request was sent to the broker;
 * it is used to match received Publish ACK to the transmitted Publish packet.
 */
static uint16_t usPublishPacketIdentifier;

/**
 * @brief MQTT packet type received from the MQTT broker.
 *
 * @note Only on receiving incoming PUBLISH, SUBACK, and UNSUBACK, this
 * variable is updated. For MQTT packets PUBACK and PINGRESP, the variable is
 * not updated since there is no need to specifically wait for it in this demo.
 * A single variable suffices as this demo uses single task and requests one operation
 * (of PUBLISH, SUBSCRIBE, UNSUBSCRIBE) at a time before expecting response from
 * the broker. Hence it is not possible to receive multiple packets of type PUBLISH,
 * SUBACK, and UNSUBACK in a single call of #prvWaitForPacket.
 * For a multi task application, consider a different method to wait for the packet, if needed.
 */
static uint16_t usPacketTypeReceived = 0U;

/**
 * @brief Packet Identifier generated when Subscribe request was sent to the broker;
 * it is used to match received Subscribe ACK to the transmitted Subscribe packet.
 */
static uint16_t usSubscribePacketIdentifier;

/**
 * @brief Packet Identifier generated when Unsubscribe request was sent to the broker;
 * it is used to match received Unsubscribe response to the transmitted Unsubscribe
 * request.
 */
static uint16_t usUnsubscribePacketIdentifier;

/**
 * @brief Static buffer used to hold MQTT messages being sent and received.
 */
static uint8_t ucSharedBuffer[2048];

/**
 * @brief A pair containing a topic filter and its SUBACK status.
 */
typedef struct topicFilterContext
{
    const char *pcTopicFilter;
    MQTTSubAckStatus_t xSubAckStatus;
} topicFilterContext_t;

/**
 * @brief An array containing the context of a SUBACK; the SUBACK status
 * of a filter is updated when the event callback processes a SUBACK.
 */
static topicFilterContext_t xTopicFilterContext;

/** @brief Static buffer used to hold MQTT messages being sent and received. */
static MQTTFixedBuffer_t xBuffer =
    {
        ucSharedBuffer,
        2048};

static TlsTransportStatus_t TLSTransportStatus;
static ProvisioningParams_t params;
static NetworkCredentials_t credentials;
static TlsTransportParams_t transport_params;
struct NetworkContext
{
    TlsTransportParams_t *pParams;
};

static const char SERVER_CERTIFICATE_PEM[] = ROOT_CA;

static uint32_t prvGetTimeMs(void)
{
    TickType_t xTickCount = 0;
    uint32_t ulTimeMs = 0UL;

    /* Get the current tick count. */
    xTickCount = xTaskGetTickCount();

    /* Convert the ticks to milliseconds. */
    ulTimeMs = (uint32_t)xTickCount * MILLISECONDS_PER_TICK;

    /* Reduce ulGlobalEntryTimeMs from obtained time so as to always return the
     * elapsed time in the application. */
    ulTimeMs = (uint32_t)(ulTimeMs - ulGlobalEntryTimeMs);

    return ulTimeMs;
}

static void prvUpdateSubAckStatus(MQTTPacketInfo_t *pxPacketInfo)
{
    MQTTStatus_t xResult = MQTTSuccess;
    uint8_t *pucPayload = NULL;
    size_t ulSize = 0;

    xResult = MQTT_GetSubAckStatusCodes(pxPacketInfo, &pucPayload, &ulSize);

    /* MQTT_GetSubAckStatusCodes always returns success if called with packet info
     * from the event callback and non-NULL parameters. */
    configASSERT(xResult == MQTTSuccess);

    xTopicFilterContext.xSubAckStatus = (MQTTSubAckStatus_t)pucPayload;
}

static void prvMQTTProcessResponse(MQTTPacketInfo_t *pxIncomingPacket,
                                   uint16_t usPacketId)
{
    switch (pxIncomingPacket->type)
    {
    case MQTT_PACKET_TYPE_PUBACK:
    {
        printf("PUBACK received for packet Id %u.\r\n", usPacketId);
        /* Make sure ACK packet identifier matches with Request packet identifier. */
        configASSERT(usPublishPacketIdentifier == usPacketId);
    }

    break;

    case MQTT_PACKET_TYPE_SUBACK:
    {
        /* Update the packet type received to SUBACK. */
        usPacketTypeReceived = MQTT_PACKET_TYPE_SUBACK;

        /* A SUBACK from the broker, containing the server response to our subscription request, has been received.
         * It contains the status code indicating server approval/rejection for the subscription to the single topic
         * requested. The SUBACK will be parsed to obtain the status code, and this status code will be stored in global
         * variable #xTopicFilterContext. */
        prvUpdateSubAckStatus(pxIncomingPacket);

        if (xTopicFilterContext.xSubAckStatus != MQTTSubAckFailure)
        {
            printf("SUBACK received for packet Id %u.\r\n", usPacketId);
        }

        /* Make sure ACK packet identifier matches with Request packet identifier. */
        //            configASSERT( usSubscribePacketIdentifier == usPacketId );
    }

    break;

    case MQTT_PACKET_TYPE_UNSUBACK:
        printf("Unsubscribed received for packet ID %u. \r\n", usPacketId);

        /* Update the packet type received to UNSUBACK. */
        usPacketTypeReceived = MQTT_PACKET_TYPE_UNSUBACK;

        /* Make sure ACK packet identifier matches with Request packet identifier. */
        configASSERT(usUnsubscribePacketIdentifier == usPacketId);

        break;

    case MQTT_PACKET_TYPE_PINGRESP:
        //            printf( "Ping Response successfully received." );

        break;

        /* Any other packet type is invalid. */
    default:
        printf("prvMQTTProcessResponse() called with unknown packet type:(%02X).",
               pxIncomingPacket->type);
    }
}

static void prvMQTTProcessIncomingPublish(MQTTPublishInfo_t *pxPublishInfo)
{
    configASSERT(pxPublishInfo != NULL);
    int led_state = 0;

    char *ptr = strstr(pxPublishInfo->pTopicName, USER_LED_TOPIC);
    if (ptr != NULL)
    {
        ptr = ptr + strlen(USER_LED_TOPIC);

        led_state = atoi((char *)ptr);

        if (led_state)
        {
            // Turn on Green LED
            LED_Toggle(GREEN_LED, ON);
        }
        else
        {
            // Turn off Green LED
            LED_Toggle(GREEN_LED, OFF);
        }
    }
}

static void prvEventCallback(MQTTContext_t *pxMQTTContext,
                             MQTTPacketInfo_t *pxPacketInfo,
                             MQTTDeserializedInfo_t *pxDeserializedInfo)
{
    /* The MQTT context is not used for this demo. */
    (void)pxMQTTContext;

    if ((pxPacketInfo->type & 0xF0U) == MQTT_PACKET_TYPE_PUBLISH)
    {
        prvMQTTProcessIncomingPublish(pxDeserializedInfo->pPublishInfo);
    }
    else
    {
        prvMQTTProcessResponse(pxPacketInfo, pxDeserializedInfo->packetIdentifier);
    }
}

static MQTTStatus_t prvWaitForPacket(MQTTContext_t *pxMQTTContext,
                                     uint16_t usPacketType)
{
    uint8_t ucCount = 0U;
    MQTTStatus_t xMQTTStatus = MQTTSuccess;
    uint32_t ulMqttProcessLoopTimeoutTime;
    uint32_t ulCurrentTime;

    /* Reset the packet type received. */
    usPacketTypeReceived = 0U;

    ulCurrentTime = pxMQTTContext->getTime();
    ulMqttProcessLoopTimeoutTime = ulCurrentTime + mqttexamplePROCESS_LOOP_TIMEOUT_MS;

    while ((ulCurrentTime < ulMqttProcessLoopTimeoutTime) && (usPacketTypeReceived != usPacketType) &&
           (ucCount++ < MQTT_PROCESS_LOOP_PACKET_WAIT_COUNT_MAX) &&
           (xMQTTStatus == MQTTSuccess || xMQTTStatus == MQTTNeedMoreBytes))
    {
        /* Event callback will set #usPacketTypeReceived when receiving appropriate packet. This
         * will wait for at most mqttexamplePROCESS_LOOP_TIMEOUT_MS. */
        xMQTTStatus = MQTT_ProcessLoop(pxMQTTContext);
        ulCurrentTime = pxMQTTContext->getTime();
    }

    if ((xMQTTStatus != MQTTSuccess && xMQTTStatus != MQTTNeedMoreBytes) || (usPacketTypeReceived != usPacketType))
    {
        printf("MQTT_ProcessLoop failed to receive packet: Packet type=%02X, LoopDuration=%u, Status=%s",
               usPacketType,
               (mqttexamplePROCESS_LOOP_TIMEOUT_MS * ucCount),
               MQTT_Status_strerror(xMQTTStatus));
    }
    else if (xMQTTStatus == MQTTNeedMoreBytes)
    {
        /* Only a partial MQTT packet is received. Call MQTT_ProcessLoop again; ideally after a small delay. */
        vTaskDelay(10);
    }

    return xMQTTStatus;
}

void aws_mqtt_init(void)
{
    /* Open little FS flash and format in order to store keys */
    assert(FSP_SUCCESS == RM_LITTLEFS_FLASH_Open(g_rm_littlefs0.p_ctrl, g_rm_littlefs0.p_cfg));
    assert(0 == lfs_format(&g_rm_littlefs0_lfs, &g_rm_littlefs0_lfs_cfg));
    assert(0 == lfs_mount(&g_rm_littlefs0_lfs, &g_rm_littlefs0_lfs_cfg));

    /* Initialize the crypto hardware acceleration. */
    mbedtls_platform_setup(NULL);

    /* Write the keys into a secure region in data flash. */
    params.pucClientCertificate = (uint8_t *)CLIENT_CERT;
    params.ulClientCertificateLength = sizeof(CLIENT_CERT);
    params.pucClientPrivateKey = (uint8_t *)PRIVATE_KEY;
    params.ulClientPrivateKeyLength = sizeof(PRIVATE_KEY);
    params.pucJITPCertificate = NULL;
    params.ulJITPCertificateLength = 0;
    vAlternateKeyProvisioning(&params);
}

BaseType_t aws_mqtt_connect(MQTTContext_t *pxMQTTContext, NetworkContext_t *pxNetworkContext)
{
    MQTTStatus_t xResult;
    TransportInterface_t transport;
    MQTTConnectInfo_t xConnectInfo;
    BaseType_t xStatus = pdFAIL;
    bool xSessionPresent;
    char pClientIdentifierBuffer[24] = {0};
    int ClientIdentifierLen = 0;

    FSP_ASSERT(NULL != pxNetworkContext);

    /* Initialize network context */
    pxNetworkContext->pParams = &transport_params;

    /* Setup network credentials */
    credentials.pAlpnProtos = NULL;
    credentials.disableSni = DISABLE_SNI;
    credentials.pRootCa = (const unsigned char *)SERVER_CERTIFICATE_PEM;
    credentials.rootCaSize = sizeof(SERVER_CERTIFICATE_PEM);
    credentials.pUserName = NULL;
    credentials.userNameSize = 0;
    credentials.pPassword = NULL;
    credentials.passwordSize = 0;
    credentials.pClientCertLabel = pkcs11configLABEL_DEVICE_CERTIFICATE_FOR_TLS;
    credentials.pPrivateKeyLabel = pkcs11configLABEL_DEVICE_PRIVATE_KEY_FOR_TLS;

    printf("Connecting to the broker: %s port: %d\r\n", EXAMPLE_MQTT_HOST, EXAMPLE_MQTT_HOST_PORT);

    /* Connect to a MQTT host */
    TLSTransportStatus = TLS_FreeRTOS_Connect(pxNetworkContext, EXAMPLE_MQTT_HOST, EXAMPLE_MQTT_HOST_PORT, &credentials, MQTT_RX_TIMEOUT_MS, MQTT_TX_TIMEOUT_MS);
    FSP_ERROR_RETURN((TLSTransportStatus == TLS_TRANSPORT_SUCCESS), pdFAIL);

    /* Fill in TransportInterface send and receive function pointers.
     * For this demo, TCP sockets are used to send and receive data
     * from network. Network context is SSL context for OpenSSL.*/
    transport.pNetworkContext = pxNetworkContext;
    transport.send = TLS_FreeRTOS_send;
    transport.recv = TLS_FreeRTOS_recv;
    transport.writev = NULL;

    /* Initialize MQTT library. */
    xResult = MQTT_Init(pxMQTTContext,
                        &transport,
                        prvGetTimeMs,
                        prvEventCallback,
                        &xBuffer);

    if (MQTTSuccess == xResult)
    {
        /* Some fields are not used in this demo so start with everything at 0. */
        (void)memset((void *)&xConnectInfo, 0x00, sizeof(xConnectInfo));

        /* Start with a clean session i.e. direct the MQTT broker to discard any
         * previous session data. Also, establishing a connection with clean session
         * will ensure that the broker does not store any data when this client
         * gets disconnected. */
        xConnectInfo.cleanSession = true;

        /* The client identifier is used to uniquely identify this MQTT client to
         * the MQTT broker. In a production device the identifier can be something
         * unique, such as a device serial number. */
        ClientIdentifierLen = snprintf(pClientIdentifierBuffer, 24,
                                       CLIENT_IDENTIFIER_PREFIX "%lu", (long unsigned int)R_BSP_UniqueIdGet());

        xConnectInfo.pClientIdentifier = pClientIdentifierBuffer;
        xConnectInfo.clientIdentifierLength = (uint16_t)ClientIdentifierLen;

        /* Set MQTT keep-alive period. If the application does not send packets at an interval less than
         * the keep-alive period, the MQTT library will send PINGREQ packets. */
        xConnectInfo.keepAliveSeconds = KEEP_ALIVE_SECONDS;

        xConnectInfo.pUserName = (const char *)IO_USERNAME;
        xConnectInfo.userNameLength = strlen(IO_USERNAME);
        xConnectInfo.pPassword = NULL;
        xConnectInfo.passwordLength = 0;

        /* Send MQTT CONNECT packet to broker. LWT is not used in this demo, so it
         * is passed as NULL. */
        xResult = MQTT_Connect(pxMQTTContext,
                               &xConnectInfo,
                               NULL,
                               MQTT_TIMEOUT_MS,
                               &xSessionPresent);
        if (xResult != MQTTSuccess)
        {
            printf("Failed to establish MQTT connection: Server=%s, MQTTStatus=%s",
                   EXAMPLE_MQTT_HOST, MQTT_Status_strerror(xResult));
        }
        else
        {
            /* Successfully established and MQTT connection with the broker. */
            printf("Successfully Connected with %s.\r\n", EXAMPLE_MQTT_HOST);
            xStatus = pdPASS;
        }
    }

    return xStatus;
}

BaseType_t aws_mqtt_subscribe(MQTTContext_t *pxMQTTContext, const char *pSubTopics)
{
    MQTTStatus_t xResult = MQTTSuccess;
    MQTTSubscribeInfo_t xMQTTSubscription = {0};
    BaseType_t xStatus = pdFAIL;

    /* Get a unique packet id. */
    usSubscribePacketIdentifier = MQTT_GetPacketId(pxMQTTContext);

    /* Subscribe to the pSubTopics topic filter. This example subscribes to
     * only one topic and uses QoS1. */
    xMQTTSubscription.qos = MQTTQoS0;
    xMQTTSubscription.pTopicFilter = pSubTopics;
    xMQTTSubscription.topicFilterLength = (uint16_t)strlen(pSubTopics);

    /* The client is now connected to the broker. Subscribe to the topic
     * as specified in mqttexampleTOPIC at the top of this file by sending a
     * subscribe packet then waiting for a subscribe acknowledgment (SUBACK).
     * This demo uses QOS0 in Subscribe, therefore, the Publish
     * messages received from the broker will have QOS0.
     */
    printf("\r\n Subscribing to topic:  %s.\r\n", pSubTopics);
    xResult = MQTT_Subscribe(pxMQTTContext,
                             &xMQTTSubscription,
                             sizeof(xMQTTSubscription) / sizeof(MQTTSubscribeInfo_t),
                             usSubscribePacketIdentifier);

    if (xResult != MQTTSuccess)
    {
        printf("Failed to SUBSCRIBE to MQTT topic %s. Error=%s\r\n",
               pSubTopics, MQTT_Status_strerror(xResult));
    }
    else
    {
        xStatus = pdPASS;
        printf("SUBSCRIBE request sent for topic %s.\r\n", pSubTopics);
    }

    return xStatus;
}

BaseType_t aws_mqtt_Unsubscribe(MQTTContext_t *pxMQTTContext, const char *pSubTopics)
{
    BaseType_t xStatus = pdPASS;
    MQTTStatus_t xResult;
    MQTTSubscribeInfo_t xMQTTSubscription[SUB_TOPIC_FILTER_COUNT] = {0};

    /* Some fields not used by this demo so start with everything at 0. */
    (void)memset((void *)&xMQTTSubscription, 0x00, sizeof(xMQTTSubscription));

    /* Get a unique packet id. */
    usSubscribePacketIdentifier = MQTT_GetPacketId(pxMQTTContext);

    /* Subscribe to the pSubTopics topic filter. */
    xMQTTSubscription[0].qos = MQTTQoS1;
    xMQTTSubscription[0].pTopicFilter = pSubTopics;
    xMQTTSubscription[0].topicFilterLength = (uint16_t)strlen(pSubTopics);

    /* Send UNSUBSCRIBE packet. */
    xResult = MQTT_Unsubscribe(pxMQTTContext,
                               xMQTTSubscription,
                               sizeof(xMQTTSubscription) / sizeof(MQTTSubscribeInfo_t),
                               usUnsubscribePacketIdentifier);

    if (xResult != MQTTSuccess)
    {
        xStatus = pdFAIL;
        printf("Failed to send UNSUBSCRIBE request to broker: TopicFilter=%s, Error=%s",
               pSubTopics,
               MQTT_Status_strerror(xResult));
    }

    /* Process incoming UNSUBACK packet from the broker. */
    xResult = prvWaitForPacket(pxMQTTContext, MQTT_PACKET_TYPE_UNSUBACK);

    if (xResult != MQTTSuccess)
    {
        xStatus = pdFAIL;
    }

    return xStatus;
}

BaseType_t aws_mqtt_publish(MQTTContext_t *pxMQTTContext, const char *pPublishTopic, const char *msg)
{
    MQTTStatus_t xResult;
    MQTTPublishInfo_t xMQTTPublishInfo = {0};
    BaseType_t xStatus = pdPASS;

    /* This application uses QoS0. */
    xMQTTPublishInfo.qos = MQTTQoS0;
    xMQTTPublishInfo.retain = false;
    xMQTTPublishInfo.pTopicName = pPublishTopic;
    xMQTTPublishInfo.topicNameLength = (uint16_t)strlen(pPublishTopic);
    xMQTTPublishInfo.pPayload = msg;
    xMQTTPublishInfo.payloadLength = strlen(msg);

    /* Get a unique packet id. */
    usPublishPacketIdentifier = MQTT_GetPacketId(pxMQTTContext);

    /* Send PUBLISH packet. Packet ID is not used for a QoS1 publish. */
    xResult = MQTT_Publish(pxMQTTContext, &xMQTTPublishInfo, usPublishPacketIdentifier);

    if (xResult != MQTTSuccess)
    {
        xStatus = pdFAIL;
        printf("Failed to send PUBLISH message to broker: Topic=%s, Error=%s",
               pPublishTopic,
               MQTT_Status_strerror(xResult));
    }

    //    /* Process incoming PUBACK packet from the broker. */
    //    xResult = prvWaitForPacket( pxMQTTContext, MQTT_PACKET_TYPE_PUBLISH  );
    //
    //    if( xResult != MQTTSuccess )
    //    {
    //        xStatus = pdFAIL;
    //    }

    return xStatus;
}

BaseType_t aws_mqtt_disconnect(MQTTContext_t *pxMQTTContext)
{
    BaseType_t xStatus = pdPASS;
    MQTTStatus_t xMQTTStatus;

    /* Send an MQTT Disconnect packet over the already connected TLS over TCP connection.
     * There is no corresponding response for the disconnect packet. After sending
     * disconnect, client must close the network connection. */

    printf("\r\n Disconnecting MQTT connection with %s.\r\n", EXAMPLE_MQTT_HOST);

    xMQTTStatus = MQTT_Disconnect(pxMQTTContext);

    if (xMQTTStatus != MQTTSuccess)
    {
        xStatus = pdFAIL;
        printf("Failed to disconnect with %s.", EXAMPLE_MQTT_HOST);
    }

    return xStatus;
}

BaseType_t aws_mqtt_wait(MQTTContext_t *pxMQTTContext)
{
    BaseType_t xStatus = pdPASS;
    MQTTStatus_t xResult;

    xResult = MQTT_ReceiveLoop(pxMQTTContext);
    if (xResult != MQTTSuccess && xResult != MQTTNeedMoreBytes)
    {
        xStatus = pdFAIL;
    }
    else if (xResult == MQTTNeedMoreBytes)
    {
        /* Only a partial MQTT packet is received. Call MQTT_ReceiveLoop again; ideally after a small delay. */
        vTaskDelay(10);
    }

    return xStatus;
}

BaseType_t aws_mqtt_ping(MQTTContext_t *pxMQTTContext)
{
    BaseType_t xStatus = pdPASS;
    MQTTStatus_t xResult;

    xResult = MQTT_Ping(pxMQTTContext);
    if (xResult != MQTTSuccess)
    {
        xStatus = pdFAIL;
    }

    return xStatus;
}
