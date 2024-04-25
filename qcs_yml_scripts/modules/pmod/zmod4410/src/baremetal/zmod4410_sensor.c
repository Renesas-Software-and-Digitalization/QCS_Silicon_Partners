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
#include "zmod4410_sensor.h"
#include "common_utils.h"

#define DEMO_ULP_DELAY_MS  (1010) // longer than 1010ms

/* Set period for delay */
#define ZMOD4410_DELAY_PERIOD (100)

/* TODO: Enable if you want to open ZMOD4XXX */
#define G_ZMOD4XXX_SENSOR0_IRQ_ENABLE   (0)

typedef enum e_demo_sequence
{
	DEMO_SEQUENCE_1 = (1),
	DEMO_SEQUENCE_2,
	DEMO_SEQUENCE_3,
	DEMO_SEQUENCE_4,
	DEMO_SEQUENCE_5,
	DEMO_SEQUENCE_6,
	DEMO_SEQUENCE_7,
	DEMO_SEQUENCE_8,
	DEMO_SEQUENCE_9,
	DEMO_SEQUENCE_10,
	DEMO_SEQUENCE_11,
	DEMO_SEQUENCE_12,
	DEMO_SEQUENCE_13,
	DEMO_SEQUENCE_14,
	DEMO_SEQUENCE_15
} demo_sequence_t;

typedef enum e_demo_callback_status
{
	DEMO_CALLBACK_STATUS_WAIT = (0),
	DEMO_CALLBACK_STATUS_SUCCESS,
	DEMO_CALLBACK_STATUS_REPEAT,
	DEMO_CALLBACK_STATUS_DEVICE_ERROR
} demo_callback_status_t;

/* Available delay units. */
typedef enum e_zmod4410_delay_units
{
	ZMOD4410_DELAY_MICROSECS = (0),     /* Requested delay amount is in microseconds */
	ZMOD4410_DELAY_MILLISECS,           /* Requested delay amount is in milliseconds */
	ZMOD4410_DELAY_SECS                 /* Requested delay amount is in seconds */
} zmod4410_delay_units_t;

static void zmod4410_delay_quick_setup(void);
static void zmod4410_delay_start(uint32_t delay, zmod4410_delay_units_t units);
static bool zmod4410_delay_wait(void);

static volatile demo_callback_status_t          gs_i2c_callback_status = DEMO_CALLBACK_STATUS_WAIT;
#if G_ZMOD4XXX_SENSOR0_IRQ_ENABLE
static volatile demo_callback_status_t          gs_irq_callback_status = DEMO_CALLBACK_STATUS_WAIT;
#endif

static volatile rm_zmod4xxx_iaq_1st_data_t      gs_iaq_1st_gen_data;
static volatile rm_zmod4xxx_iaq_2nd_data_t      gs_iaq_2nd_gen_data;
static volatile rm_zmod4xxx_odor_data_t         gs_odor_data;
static volatile rm_zmod4xxx_sulfur_odor_data_t  gs_sulfur_odor_data;
static volatile rm_zmod4xxx_rel_iaq_data_t      gs_rel_iaq_data;
static volatile rm_zmod4xxx_pbaq_data_t         gs_pbaq_data;
static          rm_zmod4xxx_raw_data_t          gs_zmod4410_raw_data;
static          demo_sequence_t                 gs_zmod4410_sequence = DEMO_SEQUENCE_1;
static          uint32_t                        gs_zmod4410_delay_count = 0;

/* Delay time definition table */
static const uint32_t gs_zmod4410_delay_time[] = {
    1,
    1000,
    1000000
};

void zmod4xxx_comms_i2c_callback(rm_zmod4xxx_callback_args_t * p_args)
{
    if ((RM_ZMOD4XXX_EVENT_DEV_ERR_POWER_ON_RESET == p_args->event)
     || (RM_ZMOD4XXX_EVENT_DEV_ERR_ACCESS_CONFLICT == p_args->event))
    {
        gs_i2c_callback_status = DEMO_CALLBACK_STATUS_DEVICE_ERROR;
    }
    else if (RM_ZMOD4XXX_EVENT_ERROR == p_args->event)
    {
        gs_i2c_callback_status = DEMO_CALLBACK_STATUS_REPEAT;
    }
    else
    {
        gs_i2c_callback_status = DEMO_CALLBACK_STATUS_SUCCESS;
    }
}

/* TODO: Enable if you want to use a IRQ callback */
void zmod4xxx_irq_callback(rm_zmod4xxx_callback_args_t * p_args)
{
#if G_ZMOD4XXX_SENSOR0_IRQ_ENABLE
    FSP_PARAMETER_NOT_USED(p_args);

    gs_irq_callback_status = DEMO_CALLBACK_STATUS_SUCCESS;
#else
    FSP_PARAMETER_NOT_USED(p_args);
#endif
}

/* Quick setup for g_hs300x_sensor0. */
void g_zmod4410_sensor0_quick_setup(void)
{
	fsp_err_t err;

    /* Clear status */
    gs_i2c_callback_status = DEMO_CALLBACK_STATUS_WAIT;
#if G_ZMOD4XXX_SENSOR0_IRQ_ENABLE
    gs_irq_callback_status = DEMO_CALLBACK_STATUS_WAIT;
#endif

    /* Reset ZMOD sensor (active low). Please change to the IO port connected to the RES_N pin of the ZMOD sensor on the customer board. */


	/* Open ZMOD4410 sensor instance, this must be done before calling any zmod4xxx API */
	err = g_zmod4xxx_sensor0.p_api->open(g_zmod4xxx_sensor0.p_ctrl, g_zmod4xxx_sensor0.p_cfg);
	assert(FSP_SUCCESS == err);

    /* Open timer */
    zmod4410_delay_quick_setup();

}

/* Quick setup for zmod4410_delay */
static void zmod4410_delay_quick_setup(void)
{
    fsp_err_t err;

    /* Open timer instance, this must be done before calling any timer API */
    err = zmod4410_delay.p_api->open(zmod4410_delay.p_ctrl, zmod4410_delay.p_cfg);
    if (FSP_SUCCESS != err)
    {
        halt_func();
    }
}

static void zmod4410_delay_start(uint32_t delay, zmod4410_delay_units_t units)
{
    fsp_err_t err;

    /* Convert to units of ZMOD4410_DELAY_PERIOD */
    gs_zmod4410_delay_count = (delay * gs_zmod4410_delay_time[units]) / ZMOD4410_DELAY_PERIOD;

    /* Stop timer */
    err = zmod4410_delay.p_api->stop(zmod4410_delay.p_ctrl);
    if (FSP_SUCCESS != err)
    {
        halt_func();
    }

    /* Reset counter */
    err = zmod4410_delay.p_api->reset(zmod4410_delay.p_ctrl);
    if (FSP_SUCCESS != err)
    {
        halt_func();
    }

    /* Start timer */
    err = zmod4410_delay.p_api->start(zmod4410_delay.p_ctrl);
    if (FSP_SUCCESS != err)
    {
        halt_func();
    }
}

static bool zmod4410_delay_wait(void)
{
    fsp_err_t err;
    bool      wait;

    if (gs_zmod4410_delay_count > 0)
    {
        wait = true;
    }
    else
    {
        /* Stop timer */
        err = zmod4410_delay.p_api->stop(zmod4410_delay.p_ctrl);
        if (FSP_SUCCESS != err)
        {
            halt_func();
        }

        wait = false;
    }

    return wait;
}

/* Timer count down */
void zmod4410_delay_callback(timer_callback_args_t *p_args)
{
    FSP_PARAMETER_NOT_USED(p_args);

    if (gs_zmod4410_delay_count > 0)
    {
        gs_zmod4410_delay_count--;
    }
}

/* Quick getting humidity and temperature for g_zmod4xxx_sensor0. */
void g_zmod4410_sensor0_quick_getting_air_quality_data(rm_zmod4xxx_iaq_2nd_data_t * p_data)
{
    fsp_err_t               err;
    rm_zmod4xxx_lib_type_t  lib_type      = g_zmod4xxx_sensor0_extended_cfg.lib_type;
    float                   temperature   = 20.0F;
    float                   humidity      = 50.0F;
    bool                    sensor_change = false;

    do
    {
        switch(gs_zmod4410_sequence)
        {
            case DEMO_SEQUENCE_1 :
            {
                /* Clear status */
                gs_i2c_callback_status = DEMO_CALLBACK_STATUS_WAIT;

                /* Start measurement */
                err = RM_ZMOD4XXX_MeasurementStart(g_zmod4xxx_sensor0.p_ctrl);
                if (FSP_SUCCESS == err)
                {
                    if (RM_ZMOD4410_LIB_TYPE_IAQ_2ND_GEN_ULP == lib_type)
                    {
                        /* Set delay period */
                        zmod4410_delay_start(DEMO_ULP_DELAY_MS, ZMOD4410_DELAY_MILLISECS);
                        gs_zmod4410_sequence = DEMO_SEQUENCE_2;
                    }
                    else if ((RM_ZMOD4410_LIB_TYPE_IAQ_2ND_GEN == lib_type) ||
                             (RM_ZMOD4410_LIB_TYPE_REL_IAQ == lib_type))
                    {
                        /* Set delay period */
                        zmod4410_delay_start(3000, ZMOD4410_DELAY_MILLISECS);
                        gs_zmod4410_sequence = DEMO_SEQUENCE_2;
                    }
                    else if (RM_ZMOD4410_LIB_TYPE_REL_IAQ_ULP == lib_type)
                    {
                        /* Set delay period */
                        zmod4410_delay_start(1500, ZMOD4410_DELAY_MILLISECS);
                        gs_zmod4410_sequence = DEMO_SEQUENCE_2;
                    }
                    else if (RM_ZMOD4410_LIB_TYPE_PBAQ == lib_type)
                    {
                        /* Set delay period */
                        zmod4410_delay_start(5000, ZMOD4410_DELAY_MILLISECS);
                        gs_zmod4410_sequence = DEMO_SEQUENCE_2;
                    }
                    else
                    {
                        gs_zmod4410_sequence = DEMO_SEQUENCE_3;
                    }
                }
                else
                {
                    halt_func();
                }
            }
            break;

            case DEMO_SEQUENCE_2 :
            {
                /* Wait delay period */
                if (zmod4410_delay_wait() == true)
                {
                    sensor_change = true;
                }
                else
                {
                    gs_zmod4410_sequence = DEMO_SEQUENCE_3;
                }
            }
            break;

            case DEMO_SEQUENCE_3 :
            {
                /* Check I2C callback status */
                switch (gs_i2c_callback_status)
                {
                    case DEMO_CALLBACK_STATUS_WAIT :
                        break;
                    case DEMO_CALLBACK_STATUS_SUCCESS :
                        sensor_change = true;
                        gs_zmod4410_sequence = DEMO_SEQUENCE_4;
                        break;
                    case DEMO_CALLBACK_STATUS_REPEAT :
                        gs_zmod4410_sequence = DEMO_SEQUENCE_1;
                        break;
                    default :
                        halt_func();
                        break;
                }
            }
            break;

#if G_ZMOD4XXX_SENSOR0_IRQ_ENABLE
            case DEMO_SEQUENCE_4 :
            {
                /* Check IRQ callback status */
                switch (gs_irq_callback_status)
                {
                    case DEMO_CALLBACK_STATUS_WAIT :
                        sensor_change = true;
                        break;
                    case DEMO_CALLBACK_STATUS_SUCCESS :
                        gs_irq_callback_status = DEMO_CALLBACK_STATUS_WAIT;
                        if ((RM_ZMOD4410_LIB_TYPE_IAQ_2ND_GEN == lib_type) ||
                            (RM_ZMOD4410_LIB_TYPE_IAQ_2ND_GEN_ULP == lib_type) ||
                            (RM_ZMOD4410_LIB_TYPE_REL_IAQ == lib_type) ||
                            (RM_ZMOD4410_LIB_TYPE_REL_IAQ_ULP == lib_type) ||
                            (RM_ZMOD4410_LIB_TYPE_PBAQ == lib_type))
                            {
                                gs_zmod4410_sequence = DEMO_SEQUENCE_6;
                            }
                            else
                            {
                                gs_zmod4410_sequence = DEMO_SEQUENCE_8;
                            }
                        break;
                    default :
                        halt_func();
                        break;
                }
            }
            break;
#else
            case DEMO_SEQUENCE_4 :
            {
                /* Clear status */
                gs_i2c_callback_status = DEMO_CALLBACK_STATUS_WAIT;

                /* Get status */
                err = RM_ZMOD4XXX_StatusCheck(g_zmod4xxx_sensor0.p_ctrl);
                if (FSP_SUCCESS == err)
                {
                    gs_zmod4410_sequence = DEMO_SEQUENCE_5;
                }
                else
                {
                    halt_func();
                }
            }
            break;

            case DEMO_SEQUENCE_5 :
            {
                /* Check I2C callback status */
                switch (gs_i2c_callback_status)
                {
                    case DEMO_CALLBACK_STATUS_WAIT :
                        break;
                    case DEMO_CALLBACK_STATUS_SUCCESS :
                        sensor_change = true;
                        if ((RM_ZMOD4410_LIB_TYPE_IAQ_2ND_GEN == lib_type) ||
                            (RM_ZMOD4410_LIB_TYPE_IAQ_2ND_GEN_ULP == lib_type) ||
                            (RM_ZMOD4410_LIB_TYPE_REL_IAQ == lib_type) ||
                            (RM_ZMOD4410_LIB_TYPE_REL_IAQ_ULP == lib_type) ||
                            (RM_ZMOD4410_LIB_TYPE_PBAQ == lib_type))
                        {
                            gs_zmod4410_sequence = DEMO_SEQUENCE_6;
                        }
                        else
                        {
                            gs_zmod4410_sequence = DEMO_SEQUENCE_8;
                        }
                        break;
                    case DEMO_CALLBACK_STATUS_REPEAT :
                        gs_zmod4410_sequence = DEMO_SEQUENCE_4;
                        break;
                    default :
                        halt_func();
                        break;
                }
            }
            break;
#endif
            case DEMO_SEQUENCE_6 :
            {
                /* Clear status */
                gs_i2c_callback_status = DEMO_CALLBACK_STATUS_WAIT;

                /* Check device error */
                err = RM_ZMOD4XXX_DeviceErrorCheck(g_zmod4xxx_sensor0.p_ctrl);
                if (FSP_SUCCESS == err)
                {
                    gs_zmod4410_sequence = DEMO_SEQUENCE_7;
                }
                else
                {
                    halt_func();
                }
            }
            break;

            case DEMO_SEQUENCE_7 :
            {
                /* Check I2C callback status */
                switch (gs_i2c_callback_status)
                {
                    case DEMO_CALLBACK_STATUS_WAIT :
                        break;
                    case DEMO_CALLBACK_STATUS_SUCCESS :
                        sensor_change = true;
                        gs_zmod4410_sequence = DEMO_SEQUENCE_8;
                        break;
                    case DEMO_CALLBACK_STATUS_REPEAT :
                        gs_zmod4410_sequence = DEMO_SEQUENCE_6;
                        break;
                    case DEMO_CALLBACK_STATUS_DEVICE_ERROR :
                    default :
                        halt_func();
                        break;
                }
            }
            break;

            case DEMO_SEQUENCE_8 :
            {
                /* Clear status */
                gs_i2c_callback_status = DEMO_CALLBACK_STATUS_WAIT;

                /* Read data */
                err = RM_ZMOD4XXX_Read(g_zmod4xxx_sensor0.p_ctrl, &gs_zmod4410_raw_data);
                if (FSP_SUCCESS == err)
                {
                    gs_zmod4410_sequence = DEMO_SEQUENCE_10;
                }
                else if (FSP_ERR_SENSOR_MEASUREMENT_NOT_FINISHED == err)
                {
                    /* Set delay period to 50ms */
                    zmod4410_delay_start(50, ZMOD4410_DELAY_MILLISECS);
                    gs_zmod4410_sequence = DEMO_SEQUENCE_9;
                }
                else
                {
                    halt_func();
                }
            }
            break;

            case DEMO_SEQUENCE_9 :
            {
                /* Wait delay period */
                if (zmod4410_delay_wait() == true)
                {
                    sensor_change = true;
                }
                else
                {
                    gs_zmod4410_sequence = DEMO_SEQUENCE_4;
                }
            }
            break;

            case DEMO_SEQUENCE_10 :
            {
                /* Check I2C callback status */
                switch (gs_i2c_callback_status)
                {
                    case DEMO_CALLBACK_STATUS_WAIT :
                        break;
                    case DEMO_CALLBACK_STATUS_SUCCESS :
                        sensor_change = true;
                        if ((RM_ZMOD4410_LIB_TYPE_IAQ_2ND_GEN == lib_type) ||
                            (RM_ZMOD4410_LIB_TYPE_IAQ_2ND_GEN_ULP == lib_type) ||
                            (RM_ZMOD4410_LIB_TYPE_REL_IAQ == lib_type) ||
                            (RM_ZMOD4410_LIB_TYPE_REL_IAQ_ULP == lib_type) ||
                            (RM_ZMOD4410_LIB_TYPE_PBAQ == lib_type))
                        {
                            gs_zmod4410_sequence = DEMO_SEQUENCE_11;
                        }
                        else
                        {
                            gs_zmod4410_sequence = DEMO_SEQUENCE_13;
                        }
                        break;
                    case DEMO_CALLBACK_STATUS_REPEAT :
                        gs_zmod4410_sequence = DEMO_SEQUENCE_8;
                        break;
                    default :
                        halt_func();
                        break;
                }
            }
            break;

            case DEMO_SEQUENCE_11 :
            {
                /* Clear status */
                gs_i2c_callback_status = DEMO_CALLBACK_STATUS_WAIT;

                /* Check device error */
                err = RM_ZMOD4XXX_DeviceErrorCheck(g_zmod4xxx_sensor0.p_ctrl);
                if (FSP_SUCCESS == err)
                {
                    gs_zmod4410_sequence = DEMO_SEQUENCE_12;
                }
                else
                {
                    halt_func();
                }
            }
            break;

            case DEMO_SEQUENCE_12 :
            {
                /* Check I2C callback status */
                switch (gs_i2c_callback_status)
                {
                    case DEMO_CALLBACK_STATUS_WAIT :
                        break;
                    case DEMO_CALLBACK_STATUS_SUCCESS :
                        sensor_change = true;
                        gs_zmod4410_sequence = DEMO_SEQUENCE_13;
                        break;
                    case DEMO_CALLBACK_STATUS_REPEAT :
                        gs_zmod4410_sequence = DEMO_SEQUENCE_11;
                        break;
                    case DEMO_CALLBACK_STATUS_DEVICE_ERROR :
                    default :
                        halt_func();
                        break;
                }
            }
            break;

            case DEMO_SEQUENCE_13 :
            {
                /* Calculate data */
                switch (lib_type)
                {
                    case RM_ZMOD4410_LIB_TYPE_IAQ_1ST_GEN_CONTINUOUS :
                    case RM_ZMOD4410_LIB_TYPE_IAQ_1ST_GEN_LOW_POWER :
                        err = RM_ZMOD4XXX_Iaq1stGenDataCalculate(g_zmod4xxx_sensor0.p_ctrl,
                                                                 &gs_zmod4410_raw_data,
                                                                 (rm_zmod4xxx_iaq_1st_data_t*)&gs_iaq_1st_gen_data);
                        break;
                    case RM_ZMOD4410_LIB_TYPE_IAQ_2ND_GEN :
                    case RM_ZMOD4410_LIB_TYPE_IAQ_2ND_GEN_ULP :
                        err = RM_ZMOD4XXX_TemperatureAndHumiditySet(g_zmod4xxx_sensor0.p_ctrl,
                                                                    temperature,
                                                                    humidity);
                        if (FSP_SUCCESS != err)
                        {
                            halt_func();
                        }
                        err = RM_ZMOD4XXX_Iaq2ndGenDataCalculate(g_zmod4xxx_sensor0.p_ctrl,
                                                                 &gs_zmod4410_raw_data,
                                                                 (rm_zmod4xxx_iaq_2nd_data_t*)&gs_iaq_2nd_gen_data);
                        memcpy(p_data, (const void *)&gs_iaq_2nd_gen_data, sizeof (rm_zmod4xxx_iaq_2nd_data_t));
                        break;
                    case RM_ZMOD4410_LIB_TYPE_ODOR :
                        err = RM_ZMOD4XXX_OdorDataCalculate(g_zmod4xxx_sensor0.p_ctrl,
                                                            &gs_zmod4410_raw_data,
                                                            (rm_zmod4xxx_odor_data_t*)&gs_odor_data);
                        break;
                    case RM_ZMOD4410_LIB_TYPE_SULFUR_ODOR :
                        err = RM_ZMOD4XXX_SulfurOdorDataCalculate(g_zmod4xxx_sensor0.p_ctrl,
                                                                  &gs_zmod4410_raw_data,
                                                                  (rm_zmod4xxx_sulfur_odor_data_t*)&gs_sulfur_odor_data);
                        break;
                    case RM_ZMOD4410_LIB_TYPE_REL_IAQ :
                    case RM_ZMOD4410_LIB_TYPE_REL_IAQ_ULP :
                        err = RM_ZMOD4XXX_RelIaqDataCalculate(g_zmod4xxx_sensor0.p_ctrl,
                                                              &gs_zmod4410_raw_data,
                                                              (rm_zmod4xxx_rel_iaq_data_t*)&gs_rel_iaq_data);
                        break;
                    case RM_ZMOD4410_LIB_TYPE_PBAQ :
                        err = RM_ZMOD4XXX_TemperatureAndHumiditySet(g_zmod4xxx_sensor0.p_ctrl,
                                                                    temperature,
                                                                    humidity);
                        if (FSP_SUCCESS != err)
                        {
                            halt_func();
                        }
                        err = RM_ZMOD4XXX_PbaqDataCalculate(g_zmod4xxx_sensor0.p_ctrl,
                                                            &gs_zmod4410_raw_data,
                                                            (rm_zmod4xxx_pbaq_data_t*)&gs_pbaq_data);
                        break;
                    default :
                        err = FSP_ERR_SENSOR_INVALID_DATA;
                        halt_func();
                        break;
                }

                if (FSP_SUCCESS == err)
                {
                    /* Gas data is valid. Describe the process by referring to each calculated gas data. */
                }
                else if (FSP_ERR_SENSOR_IN_STABILIZATION == err)
                {
                    /* Gas data is invalid. Sensor is in stabilization. */
                }
                else if (FSP_ERR_SENSOR_INVALID_DATA == err)
                {
                    /* Gas data is invalid. */
                }
                else
                {
                    halt_func();
                }

                gs_zmod4410_sequence = DEMO_SEQUENCE_14;
            }
            break;

            case DEMO_SEQUENCE_14 :
            {
                /* Set delay period */
                switch (lib_type)
                {
                    case RM_ZMOD4410_LIB_TYPE_IAQ_1ST_GEN_CONTINUOUS :
                    case RM_ZMOD4410_LIB_TYPE_ODOR :
                        gs_zmod4410_sequence = DEMO_SEQUENCE_4;
                        break;
                    case RM_ZMOD4410_LIB_TYPE_IAQ_2ND_GEN :
                    case RM_ZMOD4410_LIB_TYPE_REL_IAQ :
                    case RM_ZMOD4410_LIB_TYPE_PBAQ :
                        gs_zmod4410_sequence = DEMO_SEQUENCE_1;
                        break;
                    case RM_ZMOD4410_LIB_TYPE_IAQ_1ST_GEN_LOW_POWER :
                        zmod4410_delay_start(5475, ZMOD4410_DELAY_MILLISECS);
                        gs_zmod4410_sequence = DEMO_SEQUENCE_15;
                        break;
                    case RM_ZMOD4410_LIB_TYPE_SULFUR_ODOR :
                        /* Sulfur Odor : See Table 8 in the ZMOD4410 Programming Manual. */
                        zmod4410_delay_start(1990, ZMOD4410_DELAY_MILLISECS);
                        gs_zmod4410_sequence = DEMO_SEQUENCE_15;
                        break;
                    case RM_ZMOD4410_LIB_TYPE_IAQ_2ND_GEN_ULP :
                    case RM_ZMOD4410_LIB_TYPE_REL_IAQ_ULP :
                        /* IAQ 2nd Gen ULP : See Table 4 in the ZMOD4410 Programming Manual. */
                        /* Rel IAQ ULP : See Table 7 in the ZMOD4410 Programming Manual. */
                        zmod4410_delay_start(90000 - DEMO_ULP_DELAY_MS, ZMOD4410_DELAY_MILLISECS);
                        gs_zmod4410_sequence = DEMO_SEQUENCE_15;
                        break;
                    default :
                        halt_func();
                        break;
                }
            }
            break;

            case DEMO_SEQUENCE_15 :
            {
                /* Wait delay period */
                if (zmod4410_delay_wait() == true)
                {
                    sensor_change = true;
                }
                else
                {
                    gs_zmod4410_sequence = DEMO_SEQUENCE_1;
                }
            }
            break;

            default :
            {
                halt_func();
            }
            break;
        }
    }while(!sensor_change);
}