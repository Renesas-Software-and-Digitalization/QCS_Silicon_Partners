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
#include "fs3000_sensor.h"

/* TODO: Enable if you want to open FS3000 */
#define G_FS3000_SENSOR0_NON_BLOCKING (1)

#if G_FS3000_SENSOR0_NON_BLOCKING
volatile bool g_fs3000_completed = false;
#endif

/* TODO: Enable if you want to use a callback */
#define G_FS3000_SENSOR0_CALLBACK_ENABLE (1)
#if G_FS3000_SENSOR0_CALLBACK_ENABLE
void fs3000_callback(rm_fsxxxx_callback_args_t * p_args)
{
#if G_FS3000_SENSOR0_NON_BLOCKING
    if (RM_FSXXXX_EVENT_SUCCESS == p_args->event)
    {
        g_fs3000_completed = true;
    }
#else
    FSP_PARAMETER_NOT_USED(p_args);
#endif
}
#endif

/* Quick setup for g_fs3000_sensor0.
 * - g_comms_i2c_bus0 must be setup before calling this function
 *     (See Developer Assistance -> g_fs3000_sensor0 -> g_comms_i2c_device0 -> g_comms_i2c_bus0 -> Quick Setup).
 */
void g_fs3000_sensor0_quick_setup(void);

/* Quick setup for g_fs3000_sensor0. */
void g_fs3000_sensor0_quick_setup(void)
{
    fsp_err_t err;

    /* Open FS3000 sensor instance, this must be done before calling any FSXXXX API */
    err = g_fs3000_sensor0.p_api->open(g_fs3000_sensor0.p_ctrl, g_fs3000_sensor0.p_cfg);
    assert(FSP_SUCCESS == err);
}


/* Quick getting flow values for g_fs3000_sensor0.
 * - g_fs3000_sensor0 must be setup before calling this function.
 */
void g_fs3000_sensor0_quick_getting_flow(rm_fsxxxx_data_t * p_flow);

/* Quick getting flow for g_fs3000_sensor0. */
void g_fs3000_sensor0_quick_getting_flow(rm_fsxxxx_data_t * p_flow)
{
    fsp_err_t            err;
    rm_fsxxxx_raw_data_t fs3000_raw_data;
    bool                 is_valid_data = false;

    do
    {
        /* Read ADC data from FS3000 sensor */
        err = g_fs3000_sensor0.p_api->read(g_fs3000_sensor0.p_ctrl, &fs3000_raw_data);
        assert(FSP_SUCCESS == err);

#if G_FS3000_SENSOR0_NON_BLOCKING
        while (!g_fs3000_completed)
        {
            ;
        }
        g_fs3000_completed = false;
#endif

        /* Calculate flow values from FS3000 ADC data */
        err = g_fs3000_sensor0.p_api->dataCalculate(g_fs3000_sensor0.p_ctrl, &fs3000_raw_data, p_flow);
        if (FSP_SUCCESS == err)
        {
            is_valid_data = true;
        }
        else if (FSP_ERR_SENSOR_INVALID_DATA == err)    /* Checksum error */
        {
            is_valid_data = false;
        }
        else
        {
            assert(false);
        }
    } while (false == is_valid_data);

    /* Wait 125 milliseconds. See table 4 on the page 7 of the datasheet. */
    R_BSP_SoftwareDelay(125, BSP_DELAY_UNITS_MILLISECONDS);
}

