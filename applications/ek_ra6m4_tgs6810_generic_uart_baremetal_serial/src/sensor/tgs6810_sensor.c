/*
* Copyright (c) 2020 - 2024 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier:  BSD-3-Clause
*/
#include <stdio.h>
#include <stdint.h>
#include "common_utils.h"
#include "sm_handle.h"
#include "i2c.h"
#include "common_utils.h"
#include "tgs6810_sensor.h"

#if BSP_CFG_RTOS
#include <sensor_thread.h>
#endif
// Uncomment the desired debug level
#include "log_disabled.h"
//#include "log_error.h"
//#include "log_warning.h"
//#include "log_info.h"
//#include "log_debug.h"


// Number of sensor channels
#define NUM_CHANNELS 3
// Interval between each sample
#define WAITING_INTERVAL_MS  1000

rm_tgs6810_instance_ctrl_t g_tgs6810_sensor0_ctrl;
const rm_tgs6810_cfg_t g_tgs6810_sensor0_cfg =
{
 .p_instance = &g_comms_i2c_tgs6810,
 .p_callback = NULL,
 .p_context  = NULL,
};
const rm_tgs6810_instance_t g_tgs6810_sensor0 =
{ .p_ctrl = &g_tgs6810_sensor0_ctrl, .p_cfg = &g_tgs6810_sensor0_cfg, .p_api = &g_tgs6810_on_tgs6810, };

volatile i2c_master_event_t g_master_event = (i2c_master_event_t)0x00;
static uint8_t data_ready[3] = { 0 };
static uint8_t channels_open = 0;
static sm_sensor_status sensor_status[3] = {SM_SENSOR_ERROR};
static rm_tgs6810_data_t p_data;
static uint32_t acq_interval = WAITING_INTERVAL_MS;


void tgs6810_sensor_open(sm_handle * handle, uint8_t address, uint8_t channel)
{
    fsp_err_t status;

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
        if (FSP_SUCCESS != status && FSP_ERR_ALREADY_OPEN != status)
        {
            log_error("I2C init failed");
            APP_TRAP();
        }
        status = g_tgs6810_sensor0.p_api->open(g_tgs6810_sensor0.p_ctrl, g_tgs6810_sensor0.p_cfg);
        if(FSP_SUCCESS != status )
        {
            // Do something in case of error
            log_error("Sensor open err %d", status);
            APP_TRAP();
        }
    }
    log_info("Sensor channel %d open success",channel);
    channels_open++;
    handle->address = address;
    handle->channel = channel;
}

void tgs6810_sensor_close(sm_handle handle) {
    // Handle not used here
    FSP_PARAMETER_NOT_USED(handle);
    fsp_err_t status = FSP_SUCCESS;
    if (0 == channels_open)
    {
        log_error("Sensor not open");
    }
    else
    {
        channels_open--;
        if (0 == channels_open)
        {
            status = g_tgs6810_sensor0.p_api->close(g_tgs6810_sensor0.p_ctrl);
            if(FSP_SUCCESS != status)
            {
                // Do something in case of error
                log_error("Sensor close err %d", status);
                APP_TRAP();
            }
        }
    }
}

uint8_t * tgs6810_sensor_get_flag(sm_handle handle) {
    if (handle.channel < NUM_CHANNELS) return &data_ready[handle.channel]; else return NULL;
}

sm_result tgs6810_sensor_set_attr(sm_handle handle, sm_sensor_attributes attr, uint32_t value) {
    FSP_PARAMETER_NOT_USED(handle);	// tgs6810 can't set attributes per channel
    sm_result result = SM_ERROR;
    if (handle.channel < NUM_CHANNELS) {
        switch (attr) {
            case SM_ACQUISITION_INTERVAL:
                acq_interval = value;
                result = SM_OK;
                break;
            default:
                result = SM_NOT_SUPPORTED;
        }
    }
    return result;
}

sm_result tgs6810_sensor_get_attr(sm_handle handle, sm_sensor_attributes attr, uint32_t * value) {
    FSP_PARAMETER_NOT_USED(handle);	// tgs6810 can't get attributes per channel
    sm_result result = SM_ERROR;
    if (handle.channel < NUM_CHANNELS) {
        switch (attr) {
            case SM_ACQUISITION_INTERVAL:
                *value = acq_interval;
                result = SM_OK;
                break;
            default:
                result = SM_NOT_SUPPORTED;
        }
    }
    return result;
}

/* Quick getting tgs6810 data*/
sm_sensor_status tgs6810_sensor_read(sm_handle handle, int32_t * data)
{
    static sm_sensor_status status;
    fsp_err_t err = FSP_SUCCESS;

    switch(handle.channel){
        case SM_CH0:
            err = g_tgs6810_sensor0.p_api->read(g_tgs6810_sensor0.p_ctrl, &p_data);
            if(err != FSP_SUCCESS)
            {
            	status = SM_SENSOR_ERROR;
                sensor_status[0] = SM_SENSOR_ERROR;
                sensor_status[1] = SM_SENSOR_ERROR;
                sensor_status[2] = SM_SENSOR_ERROR;
                // Do something in case of error
                log_error("tgs6810 read err %d", err);
                APP_TRAP();
            }
            else
            {
                status = SM_SENSOR_DATA_VALID;
                *data = (int32_t)(p_data.temperature*100);
                sensor_status[0] = SM_SENSOR_DATA_VALID;
                sensor_status[1] = SM_SENSOR_DATA_VALID;
                sensor_status[2] = SM_SENSOR_DATA_VALID;
            }
            break;
        case SM_CH1:
            status = sensor_status[1];
            *data = (int32_t)(p_data.humidity*100);
            sensor_status[1] = SM_SENSOR_STALE_DATA;
            break;
        case SM_CH2:
            status = sensor_status[2];
            *data = (int32_t)(p_data.gas*100);
            sensor_status[2] = SM_SENSOR_STALE_DATA;
            break;
    }
    return status;

}

