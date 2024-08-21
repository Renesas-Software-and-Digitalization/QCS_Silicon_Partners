#include <stdio.h>
#include <stdint.h>
#include "common_utils.h"
#include "sm_handle.h"
#include "i2c.h"
#include "hs3001_sensor.h"
#if BSP_CFG_RTOS
#include <sensor_thread.h>
#endif
// Uncomment the desired debug level
#include "log_disabled.h"
//#include "log_error.h"
//#include "log_warning.h"
//#include "log_info.h"
//#include "log_debug.h"

volatile bool hs300x_completed = false;
#define G_SENSOR_TIMEOUT 10uL
// Interval between each sample
#define WAITING_INTERVAL_MS  1000
// Measurement time set to 35ms as mentioned in the HS3001 datasheet
#define MEASUREMENT_TIME_MS 35
// Number of sensor channels
#define NUM_CHANNELS 2

typedef enum {
    SENSOR_NEXT_SAMPLE,
    SENSOR_WAIT_SAMPLE,
    SENSOR_MEASUREMENT_START,
    SENSOR_MEASUREMENT_WAIT,
    SENSOR_READ,
    SENSOR_CALCULATE
} sstate;

static uint8_t data_ready[2] = { 0 };
static int32_t temperature = 0;
static int32_t humidity = 0;
static uint8_t channels_open = 0;
static sm_sensor_status sensor_status[2] = {SM_SENSOR_ERROR};
static uint32_t acq_interval = WAITING_INTERVAL_MS;

static fsp_err_t i2c_waiting(void) {
    fsp_err_t err = FSP_SUCCESS;
    static uint32_t sample_time;

    sample_time = utils_systime_get();
    while (!hs300x_completed && (utils_systime_get() - sample_time < G_SENSOR_TIMEOUT)) {}
    if (utils_systime_get() - sample_time>= G_SENSOR_TIMEOUT) {
        err = FSP_ERR_TIMEOUT;
    }
    hs300x_completed = false;
    return err;
}

void hs300x_callback(rm_hs300x_callback_args_t * p_args) {
    if (RM_HS300X_EVENT_SUCCESS == p_args->event) {
        hs300x_completed = true;
    }
}

void hs3001_sensor_open(sm_handle * handle, uint8_t address, uint8_t channel) {
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
        // Open HS300X sensor instance, this must be done before calling any HS300X API
        status = g_hs300x_sensor0.p_api->open(g_hs300x_sensor0.p_ctrl, g_hs300x_sensor0.p_cfg);
        if(FSP_SUCCESS != status && FSP_ERR_ALREADY_OPEN != status) {
            // Do something in case of error
            log_error("Sensor open err %d", status);
            APP_TRAP();
        }
    }
    channels_open++;
    log_info("Sensor channel %d open success",channel);
    handle->address = address;
    handle->channel = channel;
}

void hs3001_sensor_close(sm_handle handle) {
    // Handle not used here
    FSP_PARAMETER_NOT_USED(handle);
    fsp_err_t status = FSP_SUCCESS;
    if (0 == channels_open) {
        log_error("Sensor not open");
    } else {
        channels_open--;
        if (0 == channels_open) {
            status = g_hs300x_sensor0.p_api->close(g_hs300x_sensor0.p_ctrl);
            if(FSP_SUCCESS != status) {
                // Do something in case of error
                log_error("Sensor close err %d", status);
                APP_TRAP();
            }
        }
    }
}

sm_sensor_status hs3001_sensor_read(sm_handle handle, int32_t * data) {
    log_debug("Sensor read channel %d", handle.channel);
    sm_sensor_status status = SM_SENSOR_ERROR;
    if (handle.channel == 0) {
        status = sensor_status[0];
        *data =  (int32_t)temperature;
        sensor_status[0] = SM_SENSOR_STALE_DATA;
    } else if (handle.channel == 1) {
        status = sensor_status[1];
        *data =  (int32_t)humidity;
        sensor_status[1] = SM_SENSOR_STALE_DATA;
    }
    return status;
}

uint8_t * hs3001_sensor_get_flag(sm_handle handle) {
    if (handle.channel < NUM_CHANNELS) return &data_ready[handle.channel]; else return NULL;
}

sm_result hs3001_sensor_set_attr(sm_handle handle, sm_sensor_attributes attr, uint32_t value) {
	FSP_PARAMETER_NOT_USED(handle);	// HS3001 can't set attributes per channel
    sm_result result = SM_ERROR;
    if (handle.channel < NUM_CHANNELS) {
    	switch (attr) {
			case SM_ACQUISITION_INTERVAL:
				if (value >= MEASUREMENT_TIME_MS) {
					acq_interval = value;
					result = SM_OK;
				}
				break;
			default:
				result = SM_NOT_SUPPORTED;
    	}
    }
    return result;
}

sm_result hs3001_sensor_get_attr(sm_handle handle, sm_sensor_attributes attr, uint32_t * value) {
	FSP_PARAMETER_NOT_USED(handle);	// HS3001 can't get attributes per channel
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

void hs3001_sensor_fsm(void) {
    static uint32_t timer;
    static sstate sensor_state;
    static rm_hs300x_raw_data_t hs300x_raw_data;
    static rm_hs300x_data_t data;
    fsp_err_t status = FSP_SUCCESS;
    switch (sensor_state) {
        case SENSOR_NEXT_SAMPLE:
            timer = utils_systime_get();
            sensor_state = SENSOR_WAIT_SAMPLE;
            break;
        case SENSOR_WAIT_SAMPLE:
            if (utils_systime_get() >= (timer + acq_interval)) sensor_state = SENSOR_MEASUREMENT_START;
            break;
        case SENSOR_MEASUREMENT_START:
            /* Start the measurement */
            status = g_hs300x_sensor0.p_api->measurementStart(g_hs300x_sensor0.p_ctrl);
            if(FSP_SUCCESS != status) {
                log_error("Measurement start err %d", status);
            }
            status = i2c_waiting();
            if(FSP_SUCCESS != status) {
            	log_error("i2c waiting err %d", status);
            } else {
                sensor_state = SENSOR_MEASUREMENT_WAIT;
                timer = utils_systime_get();
            }
            break;
        case SENSOR_MEASUREMENT_WAIT:
            /* Wait for the sensor to complete the measurement */
            if (utils_systime_get() >= (timer + MEASUREMENT_TIME_MS)) sensor_state = SENSOR_READ;
            break;
        case SENSOR_READ:
            /* Read ADC data from HS300X sensor */
            status = g_hs300x_sensor0.p_api->read(g_hs300x_sensor0.p_ctrl, &hs300x_raw_data);
            if(FSP_SUCCESS != status) {
                log_error("Read err %d", status);
            }
            status = i2c_waiting();
            if(FSP_SUCCESS != status) {
            	log_error("i2c waiting err %d", status);
                sensor_state = SENSOR_MEASUREMENT_START;
            } else {
                sensor_state = SENSOR_CALCULATE;
            }
            break;
        case SENSOR_CALCULATE:
            /* Calculate humidity and temperature values from ADC data */
            status = g_hs300x_sensor0.p_api->dataCalculate(g_hs300x_sensor0.p_ctrl, &hs300x_raw_data, &data);
            if (FSP_SUCCESS == status) {
                data_ready[0] = 1;
                data_ready[1] = 1;
                temperature = data.temperature.integer_part*100 + data.temperature.decimal_part;
                humidity = data.humidity.integer_part*100 + data.humidity.decimal_part;
                sensor_status[0] = SM_SENSOR_DATA_VALID;
                sensor_status[1] = SM_SENSOR_DATA_VALID;
                sensor_state = SENSOR_NEXT_SAMPLE;
            }
            else if (FSP_ERR_SENSOR_INVALID_DATA == status) {
                log_error("Invalid data");
                sensor_status[0] = SM_SENSOR_INVALID_DATA;
                sensor_status[1] = SM_SENSOR_INVALID_DATA;
                sensor_state = SENSOR_READ;
            } else {
                log_error("DataCalculate err %d", status);
                sensor_status[0] = SM_SENSOR_ERROR;
                sensor_status[1] = SM_SENSOR_ERROR;
                sensor_state = SENSOR_NEXT_SAMPLE;
            }
            break;
        default:
            sensor_state = SENSOR_NEXT_SAMPLE;
            break;
    }
}

