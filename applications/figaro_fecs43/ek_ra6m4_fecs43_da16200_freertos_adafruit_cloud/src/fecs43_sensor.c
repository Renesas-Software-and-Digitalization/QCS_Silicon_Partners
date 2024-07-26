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
#include "fecs43_sensor.h"
#include "sensor_thread.h"

rm_fecs43_instance_ctrl_t g_fecs43_sensor0_ctrl;
const rm_fecs43_cfg_t g_fecs43_sensor0_cfg =
{
 .p_instance = &g_comms_i2c_fecs43,
 .p_callback = NULL,
 .p_context  = NULL,
};

const rm_fecs43_instance_t g_fecs43_sensor0 =
{ .p_ctrl = &g_fecs43_sensor0_ctrl, .p_cfg = &g_fecs43_sensor0_cfg, .p_api = &g_fecs43_on_fecs43, };

volatile i2c_master_event_t g_master_event = (i2c_master_event_t)0x00;

void g_fecs43_quick_setup(void)
{
    fsp_err_t err;

    err = g_fecs43_sensor0.p_api->open(g_fecs43_sensor0.p_ctrl, g_fecs43_sensor0.p_cfg);
    assert(FSP_SUCCESS == err);
}

/* Quick getting fecs43 data*/
void g_fecs43_sensor0_read(float *p_temperature, float *p_humidity, float *p_gas)
{
    fsp_err_t err;
    rm_fecs43_data_t p_data;

    err = g_fecs43_sensor0.p_api->read(g_fecs43_sensor0.p_ctrl, &p_data);
    assert(FSP_SUCCESS == err);

    *p_temperature = p_data.temperature;
    *p_humidity    = p_data.humidity;
    *p_gas         = p_data.gas;
}

