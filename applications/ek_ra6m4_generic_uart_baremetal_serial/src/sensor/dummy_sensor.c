/*
* Copyright (c) 2020 - 2024 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier:  BSD-3-Clause
*
* This file demonstrates a sensor implementation over I2C, it performs a direct read and the sensor
* is expected to reply with data without the need of additional transactions (standard I2C transaction).
* There are two channels: temperature (channel 0) and humidity (channel 1).
* If a sensor needs multiple transactions to perform a read (ie.: one transaction to start the read and another
* for retrieving data) then we recommend using a state machine to handle it. For details please refer to
* hs3001_sensor_fsm() in hs3001_sensor.c.
*/
/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include "hal_data.h"
#include "common_utils.h"
#include "sm_handle.h"
#include "i2c.h"
#include "dummy_driver.h"
#include "dummy_sensor.h"
/***********************************************************************************************************************
 * Logging configuration
 * Uncomment the desired debug level
 **********************************************************************************************************************/
#include "log_disabled.h"
//#include "log_error.h"
//#include "log_warning.h"
//#include "log_info.h"
//#include "log_debug.h"

/***********************************************************************************************************************
 * Constant definitions
 **********************************************************************************************************************/
// Number of sensor channels
#define NUM_CHANNELS            (2)

/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/
static uint8_t channels_open = 0;

/***********************************************************************************************************************
 * Public Functions - these are called by Sensor Manager
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * @brief open a sensor channel
 * Note that this function is called by Sensor Manager, once for each channel
 **********************************************************************************************************************/
void dummy_sensor_open(sm_handle * handle, uint8_t address, uint8_t channel) {
    fsp_err_t status = FSP_SUCCESS;
#if BSP_CFG_RTOS
    /* Create a semaphore for blocking if a semaphore is not NULL */
    if (NULL != g_comms_i2c_bus0_extended_cfg.p_blocking_semaphore)
    {
#if BSP_CFG_RTOS == 1 // AzureOS
        tx_semaphore_create(g_comms_i2c_bus0_extended_cfg.p_blocking_semaphore->p_semaphore_handle,
                            g_comms_i2c_bus0_extended_cfg.p_blocking_semaphore->p_semaphore_name,
                            (ULONG)0);
#elif BSP_CFG_RTOS == 2 // FreeRTOS
        *(g_comms_i2c_bus0_extended_cfg.p_blocking_semaphore->p_semaphore_handle) = xSemaphoreCreateCountingStatic((UBaseType_t)1, (UBaseType_t)0, g_comms_i2c_bus0_extended_cfg.p_blocking_semaphore->p_semaphore_memory);
#endif
    }

    /* Create a recursive mutex for bus lock if a recursive mutex is not NULL */
    if (NULL != g_comms_i2c_bus0_extended_cfg.p_bus_recursive_mutex)
    {
#if BSP_CFG_RTOS == 1 // AzureOS
        tx_mutex_create(g_comms_i2c_bus0_extended_cfg.p_bus_recursive_mutex->p_mutex_handle,
                        g_comms_i2c_bus0_extended_cfg.p_bus_recursive_mutex->p_mutex_name,
                        TX_INHERIT);
#elif BSP_CFG_RTOS == 2 // FreeRTOS
        *(g_comms_i2c_bus0_extended_cfg.p_bus_recursive_mutex->p_mutex_handle) = xSemaphoreCreateRecursiveMutexStatic(g_comms_i2c_bus0_extended_cfg.p_bus_recursive_mutex->p_mutex_memory);
#endif
    }
#endif
    if (0 == channels_open) {
        status = i2c_initialize();
        if (FSP_SUCCESS != status && FSP_ERR_ALREADY_OPEN != status) {
            log_error("I2C init failed");
            APP_TRAP();
        }
        // Perform any one-time open operations required by the sensor driver here
        status = dummy_writeReg(DUMMY_PWR_CTRL, DUMMY_POWER_ON);  // Turn on sensor
        if (FSP_SUCCESS != status) {
            log_error("Sensor init failed");
            APP_TRAP();
        }
    }
    channels_open++;
    log_info("Sensor channel %d open success",channel);
    handle->address = address;
    handle->channel = channel;
}

/***********************************************************************************************************************
 * @brief close a sensor channel
 * Note that this function is called by Sensor Manager, once for each channel
 **********************************************************************************************************************/
void dummy_sensor_close(sm_handle handle) {
    // Handle not used here
    FSP_PARAMETER_NOT_USED(handle);
    fsp_err_t status = FSP_SUCCESS;
    if (0 == channels_open) {
        log_error("Sensor not open");
    } else {
        channels_open--;
        if (0 == channels_open) {
            // Perform any one-time close operations required by the sensor driver here
            status = dummy_writeReg(DUMMY_PWR_CTRL, 0);  // Turn off sensor
            if (FSP_SUCCESS != status) {
                log_error("Sensor init failed");
                APP_TRAP();
            }
        }
    }
}

/***********************************************************************************************************************
 * @brief read a sensor channel
 * Note that this function is called by Sensor Manager, once for each channel.
 * It is expected that this function does not block nor it adds long delays!
 **********************************************************************************************************************/
sm_sensor_status dummy_sensor_read(sm_handle handle, int32_t * data) {
    log_debug("Sensor read channel %d", handle.channel);
    sm_sensor_status status = SM_SENSOR_ERROR;
    fsp_err_t result = FSP_SUCCESS;
    *data = 0;
    if (handle.channel == 0) {
        uint8_t low, hi;
        result = dummy_readReg(DUMMY_TEMP_LO, &low);
        if (FSP_SUCCESS != result) return status;
        result = dummy_readReg(DUMMY_TEMP_HI, &hi);
        if (FSP_SUCCESS != result) return status;
        *data =  (int32_t)(((uint32_t)hi) << 8 | (uint32_t)(low));
        status = SM_SENSOR_DATA_VALID;
    } else if (handle.channel == 1) {
        uint8_t low, hi;
        result = dummy_readReg(DUMMY_HUMI_LO, &low);
        if (FSP_SUCCESS != result) return status;
        result = dummy_readReg(DUMMY_HUMI_HI, &hi);
        if (FSP_SUCCESS != result) return status;
        *data =  (int32_t)(((uint32_t)hi) << 8 | (uint32_t)(low));
        status = SM_SENSOR_DATA_VALID;
    }
    return status;
}