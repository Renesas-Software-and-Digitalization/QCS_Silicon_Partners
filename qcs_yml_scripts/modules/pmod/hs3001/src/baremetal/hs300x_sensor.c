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
#include "common_utils.h"
#include "hs300x_sensor.h"

void comms_i2c_callback(rm_comms_callback_args_t * p_args);

/* TODO: Enable if you want to open HS300X */
#define G_HS300X_SENSOR0_NON_BLOCKING (1)
#define G_SENSOR_TIMEOUT (100)

#if G_HS300X_SENSOR0_NON_BLOCKING
volatile bool g_hs300x_completed = false;
#endif

#if RM_HS300X_CFG_PROGRAMMING_MODE
uint32_t g_hs300x_sensor_id;
#endif

/* TODO: Enable if you want to use a callback */
#define G_HS300X_SENSOR0_CALLBACK_ENABLE (1)
#if G_HS300X_SENSOR0_CALLBACK_ENABLE
void hs300x_callback(rm_hs300x_callback_args_t * p_args)
{
#if G_HS300X_SENSOR0_NON_BLOCKING
    if (RM_HS300X_EVENT_SUCCESS == p_args->event)
    {
        g_hs300x_completed = true;
    }
#else
    FSP_PARAMETER_NOT_USED(p_args);
#endif
}
#endif


/* Quick setup for g_hs300x_sensor0. */
void g_hs300x_sensor0_quick_setup(void)
{
    fsp_err_t err;

    /* Open HS300X sensor instance, this must be done before calling any HS300X API */
    err = g_hs300x_sensor0.p_api->open(g_hs300x_sensor0.p_ctrl, g_hs300x_sensor0.p_cfg);
    assert(FSP_SUCCESS == err);

#if RM_HS300X_CFG_PROGRAMMING_MODE
    /* Enter the programming mode. This must be called within 10ms after applying power. */
    err = g_hs300x_sensor0.p_api->programmingModeEnter(g_hs300x_sensor0.p_ctrl);
    assert(FSP_SUCCESS == err);

#if G_HS300X_SENSOR0_NON_BLOCKING
    while (!g_hs300x_completed)
    {
        ;
    }
    g_hs300x_completed = false;
#endif

    /* Delay 120us. Entering the programming mode takes 120us. */
    R_BSP_SoftwareDelay(120, BSP_DELAY_UNITS_MICROSECONDS);

    /* Get the sensor ID */
    err = g_hs300x_sensor0.p_api->sensorIdGet(g_hs300x_sensor0.p_ctrl, (uint32_t *)&g_hs300x_sensor_id);
    assert(FSP_SUCCESS == err);

    /* Change the humidity resolution to 8 bit */
    err = g_hs300x_sensor0.p_api->resolutionChange(g_hs300x_sensor0.p_ctrl, RM_HS300X_HUMIDITY_DATA, RM_HS300X_RESOLUTION_8BIT);
    assert(FSP_SUCCESS == err);

#if G_HS300X_SENSOR0_NON_BLOCKING
    while (!g_hs300x_completed)
    {
        ;
    }
    g_hs300x_completed = false;
#endif

    /* Delay 14ms. Failure to comply with these times may result in data corruption and introduce errors in sensor measurements. */
    R_BSP_SoftwareDelay(14, BSP_DELAY_UNITS_MILLISECONDS);

    /* Change the temperature resolution to 8 bit */
    err = g_hs300x_sensor0.p_api->resolutionChange(g_hs300x_sensor0.p_ctrl, RM_HS300X_TEMPERATURE_DATA, RM_HS300X_RESOLUTION_8BIT);
    assert(FSP_SUCCESS == err);

#if G_HS300X_SENSOR0_NON_BLOCKING
    while (!g_hs300x_completed)
    {
        ;
    }
    g_hs300x_completed = false;
#endif

    /* Delay 14ms. Failure to comply with these times may result in data corruption and introduce errors in sensor measurements. */
    R_BSP_SoftwareDelay(14, BSP_DELAY_UNITS_MILLISECONDS);

    /* Exit the programming mode */
    err = g_hs300x_sensor0.p_api->programmingModeExit(g_hs300x_sensor0.p_ctrl);
    assert(FSP_SUCCESS == err);

#if G_HS300X_SENSOR0_NON_BLOCKING
    while (!g_hs300x_completed)
    {
        ;
    }
    g_hs300x_completed = false;
#endif
#endif
}


/* TODO: Enable if you want to open I2C Communications Device */
#define G_COMMS_I2C_DEVICE0_NON_BLOCKING (0)

#if G_COMMS_I2C_DEVICE0_NON_BLOCKING
volatile bool g_i2c_completed = false;
#endif

/* TODO: Enable if you want to use a callback */
#define G_COMMS_I2C_DEVICE0_CALLBACK_ENABLE (0)
#if G_COMMS_I2C_DEVICE0_CALLBACK_ENABLE
void comms_i2c_callback(rm_comms_callback_args_t * p_args)
{
#if G_COMMS_I2C_DEVICE0_NON_BLOCKING
    if (RM_COMMS_EVENT_OPERATION_COMPLETE == p_args->event)
    {
        g_i2c_completed = true;
    }
#else
    FSP_PARAMETER_NOT_USED(p_args);
#endif
}
#endif


/* Quick getting humidity and temperature for g_hs300x_sensor0. */
void g_hs300x_sensor0_quick_getting_humidity_and_temperature(rm_hs300x_data_t * p_data)
{
    fsp_err_t            err;
    uint32_t startTime = 0;
    uint32_t currentTime = 0;
    rm_hs300x_raw_data_t hs300x_raw_data;
    bool is_valid_data = false;

    /* Start the measurement */
    err = g_hs300x_sensor0.p_api->measurementStart(g_hs300x_sensor0.p_ctrl);
    assert(FSP_SUCCESS == err);
    currentTime = startTime =  SysTime_get();
#if G_HS300X_SENSOR0_NON_BLOCKING
    while (!g_hs300x_completed && (currentTime - startTime < G_SENSOR_TIMEOUT))
    {
        currentTime = SysTime_get();
    }
    if (currentTime - startTime >= G_SENSOR_TIMEOUT)
    {
    	halt_func();
    }
    g_hs300x_completed = false;
#endif

    do
    {
        /* Read ADC data from HS300X sensor */
        err = g_hs300x_sensor0.p_api->read(g_hs300x_sensor0.p_ctrl, &hs300x_raw_data);
        assert(FSP_SUCCESS == err);
#if G_HS300X_SENSOR0_NON_BLOCKING
        while (!g_hs300x_completed && (currentTime - startTime < G_SENSOR_TIMEOUT))
        {
            currentTime = SysTime_get();
        }
        if (currentTime - startTime >= G_SENSOR_TIMEOUT)
        {
        	halt_func();
        }
        g_hs300x_completed = false;
#endif

        /* Calculate humidity and temperature values from ADC data */
        err = g_hs300x_sensor0.p_api->dataCalculate(g_hs300x_sensor0.p_ctrl, &hs300x_raw_data, p_data);
        if (FSP_SUCCESS == err)
        {
            is_valid_data = true;
        }
        else if (FSP_ERR_SENSOR_INVALID_DATA == err)
        {
            is_valid_data = false;
        }
        else
        {
            assert(false);
        }
    }
    while (false == is_valid_data);
}

