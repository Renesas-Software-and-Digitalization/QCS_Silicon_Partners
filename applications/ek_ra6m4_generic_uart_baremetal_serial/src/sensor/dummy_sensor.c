/*
* Copyright (c) 2020 - 2024 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier:  BSD-3-Clause
*
* This file demonstrates a sensor implementation over I2C, it performs a direct read and the sensor
* is expected to reply with data without the need of additional transactions (standard I2C transaction).
* There are two channels: temperature (channel 0) and humidity (channel 1).
* If a sensor needs multiple transactions to perform a read (ie.: one transaction to start the read and another
* for retrieving data) then we recommend using a state machine to handle it. For details please refer t
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
#include "dummy_sensor.h"
#if BSP_CFG_RTOS
#include <sensor_thread.h>
#endif
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
// Sensor communication timeout
#define SENSOR_DUMMY_TIMEOUT_MS (100)
// These are some pretend registers...
#define DUMMY_PWR_CTRL          (0x01)
#define DUMMY_TEMP_LO           (0x02)
#define DUMMY_TEMP_HI   	    (0x03)
#define DUMMY_HUMI_LO           (0x04)
#define DUMMY_HUMI_HI           (0x05)
#define DUMMY_POWER_ON          (0x02)

/***********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/
static fsp_err_t dummy_readReg(uint8_t reg, uint8_t *p_data);
static fsp_err_t dummy_writeReg(uint8_t reg, uint8_t data);
static fsp_err_t dummy_read(rm_comms_write_read_params_t write_read_params);
static fsp_err_t dummy_write(uint8_t * const p_src, uint8_t const bytes);

/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/
static uint8_t channels_open = 0;
#if (BSP_CFG_RTOS == 0)
static bool g_comm_i2c_flag = false;
static rm_comms_event_t g_comm_i2c_event;
#endif

/***********************************************************************************************************************
 * Private Functions
 * These functions can be implemented in a separate file if desired
 **********************************************************************************************************************/

static fsp_err_t dummy_readReg(uint8_t reg, uint8_t *p_data) {
	/******************** Example on how to read data from i2c *******************************************************************************/
	rm_comms_write_read_params_t write_read_params;
	write_read_params.p_src      = &reg;
	write_read_params.src_bytes  = 1;
	write_read_params.p_dest     = p_data;
	write_read_params.dest_bytes = 1;
	//return dummy_read(write_read_params);  // Since there is no sensor, let's not try to read anything!
    // The following block is just for faking some registers readings, remove it for a real sensor!
    {
        switch (reg) {
            case DUMMY_TEMP_LO: *p_data = 0xC4; return FSP_SUCCESS;  // 0x09C4 -> 2500 -> 25.00 degrees Celsius
            case DUMMY_TEMP_HI: *p_data = 0x09; return FSP_SUCCESS;
            case DUMMY_HUMI_LO: *p_data = 0x70; return FSP_SUCCESS;  // 0x1770 -> 6000 -> 60.00% relative humidity
            case DUMMY_HUMI_HI: *p_data = 0x17; return FSP_SUCCESS; 
        }
    }
}

static fsp_err_t dummy_writeReg(uint8_t reg, uint8_t data) {
	/******************** Example on how to write data to a specific register *******************************************************************************/
	uint8_t write_data[2];
	write_data[0] = reg;
	write_data[1] =  data;
	//return dummy_write(write_data, sizeof(write_data));   // Since there is no sensor, let's not try to write anything!
    return FSP_SUCCESS;
}

/*******************************************************************************************************************//**
* @brief Read data from Sensor Dummy device.
*
* @retval FSP_SUCCESS              Successfully started.
* @retval FSP_ERR_TIMEOUT          communication is timeout.
* @retval FSP_ERR_ABORTED          communication is aborted.
**********************************************************************************************************************/
static fsp_err_t dummy_read(rm_comms_write_read_params_t write_read_params) {
   fsp_err_t err = FSP_SUCCESS;
    g_comm_i2c_flag = false;
   /* WriteRead data */
   err = g_comms_i2c_dummy_sensor.p_api->writeRead(g_comms_i2c_dummy_sensor.p_ctrl, write_read_params);
   FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

#if (BSP_CFG_RTOS == 0)
   uint32_t counter = 0;
   /* Wait callback */
   while (false == g_comm_i2c_flag) {
       utils_delay_us(100);
       counter++;
       FSP_ERROR_RETURN((SENSOR_DUMMY_TIMEOUT_MS*10) < counter, FSP_ERR_TIMEOUT);
   }
   /* Check callback event */
   FSP_ERROR_RETURN(RM_COMMS_EVENT_OPERATION_COMPLETE == g_comm_i2c_event, FSP_ERR_ABORTED);
#endif

   return FSP_SUCCESS;
}

/*******************************************************************************************************************//**
* @brief Write data to Sensor Dummy device.
*
* @retval FSP_SUCCESS              Successfully started.
* @retval FSP_ERR_TIMEOUT          communication is timeout.
* @retval FSP_ERR_ABORTED          communication is aborted.
**********************************************************************************************************************/
static fsp_err_t dummy_write(uint8_t * const p_src, uint8_t const bytes) {
   fsp_err_t err = FSP_SUCCESS;
    g_comm_i2c_flag = false;
   /* Write data */
   err = g_comms_i2c_dummy_sensor.p_api->write(g_comms_i2c_dummy_sensor.p_ctrl, p_src, (uint32_t) bytes);
   FSP_ERROR_RETURN(FSP_SUCCESS == err, err);
#if (BSP_CFG_RTOS == 0)
   uint16_t counter = 0;
   /* Wait callback */
   while (false == g_comm_i2c_flag) {
       utils_delay_us(100);
       counter++;
       FSP_ERROR_RETURN((SENSOR_DUMMY_TIMEOUT_MS*10) < counter, FSP_ERR_TIMEOUT);
   }
   /* Check callback event */
   FSP_ERROR_RETURN(RM_COMMS_EVENT_OPERATION_COMPLETE == g_comm_i2c_event, FSP_ERR_ABORTED);
#endif

   return FSP_SUCCESS;
}

/***********************************************************************************************************************
 * Public Functions - these are called by Sensor Manager
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @brief callback function called in the I2C Communications Middleware callback function.
 **********************************************************************************************************************/
void dummy_sensor_callback(rm_comms_callback_args_t *p_args) {
#if (BSP_CFG_RTOS > 0)
    FSP_PARAMETER_NOT_USED(p_args);
#else
	/* Set event */
	switch (p_args->event) {
        case RM_COMMS_EVENT_OPERATION_COMPLETE:
            g_comm_i2c_event = RM_COMMS_EVENT_OPERATION_COMPLETE;
            g_comm_i2c_flag = true;
            break;
        case RM_COMMS_EVENT_ERROR:
            g_comm_i2c_event = RM_COMMS_EVENT_ERROR;
            break;
        default: 
            break;
	}
#endif
}

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
        FSP_ERROR_RETURN(FSP_SUCCESS == status, status);
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
            FSP_ERROR_RETURN(FSP_SUCCESS == status, status);
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