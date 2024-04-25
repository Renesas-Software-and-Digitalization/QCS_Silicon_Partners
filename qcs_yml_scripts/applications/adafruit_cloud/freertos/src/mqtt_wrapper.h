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
#ifndef MQTT_WRAPPER_H_
#define MQTT_WRAPPER_H_

#include "user.h"
#include "main_thread.h"

/* Standard includes. */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* MQTT library includes. */
#include "core_mqtt.h"
#include "common_utils.h"
#include "using_plaintext.h"

/**
 * @brief The timeout for MQTT operations in this demo.
 */
#define MQTT_RX_TIMEOUT_MS                          ( 5000 )
#define MQTT_TX_TIMEOUT_MS                          ( 5000 )
#define MQTT_TIMEOUT_MS                             ( 5000 )

/**
 * @brief The keep-alive interval used for this demo.
 *
 * An MQTT ping request will be sent periodically at this interval.
 */
#define KEEP_ALIVE_SECONDS                       ( 60 )

/**
 * @brief The maximum number of retries for network operation with server.
 */
#define RETRY_MAX_ATTEMPTS                                ( 5U )

/**
 * @brief The maximum back-off delay (in milliseconds) for retrying failed operation
 *  with server.
 */
#define RETRY_MAX_BACKOFF_DELAY_MS                        ( 5000U )

/**
 * @brief The base back-off delay (in milliseconds) to use for network operation retry
 * attempts.
 */
#define RETRY_BACKOFF_BASE_MS                             ( 500U )

/**
 * @brief Milliseconds per second.
 */
#define MILLISECONDS_PER_SECOND                           ( 1000U )

/**
 * @brief Milliseconds per FreeRTOS tick.
 */
#define MILLISECONDS_PER_TICK                             ( MILLISECONDS_PER_SECOND / configTICK_RATE_HZ )

#define CLIENT_IDENTIFIER_PREFIX                            "Renesas"

/**
 * @brief The maximum number of times to call MQTT_ProcessLoop() when polling
 * for a specific packet from the broker.
 */
#define MQTT_PROCESS_LOOP_PACKET_WAIT_COUNT_MAX           ( 1U )

/**
 * @brief Timeout for MQTT_ProcessLoop in milliseconds.
 */
#define mqttexamplePROCESS_LOOP_TIMEOUT_MS                ( 1000U )

#define mqttexampleRECEIVE_LOOP_TIMEOUT_MS                ( 50U )

#define SUB_TOPIC_FILTER_COUNT                            ( 1 )

#define EXAMPLE_MQTT_HOST                                   "io.adafruit.com"
#define EXAMPLE_MQTT_HOST_PORT                               ( 1883 )

/* Function Prototypes */
void aws_mqtt_init(void);
BaseType_t aws_mqtt_connect(MQTTContext_t *pxMQTTContext, NetworkContext_t * pxNetworkContext);
BaseType_t aws_mqtt_subscribe(MQTTContext_t * pxMQTTContext, const char *pSubTopics);
BaseType_t aws_mqtt_Unsubscribe(MQTTContext_t * pxMQTTContext, const char *pSubTopics);
BaseType_t aws_mqtt_publish(MQTTContext_t * pxMQTTContext, const char *pPublishTopic, const char *msg);
BaseType_t aws_mqtt_disconnect(MQTTContext_t * pxMQTTContext);
BaseType_t aws_mqtt_wait(MQTTContext_t * pxMQTTContext);
BaseType_t aws_mqtt_ping(MQTTContext_t *pxMQTTContext);


#endif /* AWS_MQTT_H_ */
