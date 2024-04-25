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
#include "zmod4510_sensor.h"

/*
 * When combining with other ZMOD4XXX sensors,
 * set the constant G_ZMOD4XXX_SENSOR_RESET_ENABLE to 1 in the sensor thread called first.
 * Basically, the first created thread is called first.
 */
#define G_ZMOD4XXX_SENSOR_RESET_ENABLE  (0)

/* TODO: Enable if you want to open ZMOD4XXX */

/*
 * Program flow for OAQ 2nd gen is changed. Please refer to the programming manual [R36US0004EU] and the application note[R01AN5899].\n
 * R01AN5899 : https://www.renesas.com/document/apn/zmod4xxx-sample-application
 * R36US0004EU : https://www.renesas.com/document/mat/zmod4510-programming-manual-read-me
 */

#define G_ZMOD4XXX_SENSOR1_IRQ_ENABLE   (0)

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

volatile demo_callback_status_t            gs_i2c_callback_status = DEMO_CALLBACK_STATUS_WAIT;
#if G_ZMOD4XXX_SENSOR1_IRQ_ENABLE
volatile demo_callback_status_t            gs_irq_callback_status = DEMO_CALLBACK_STATUS_WAIT;
#endif

static volatile rm_zmod4xxx_oaq_1st_data_t gs_oaq_1st_gen_data;
static volatile rm_zmod4xxx_oaq_2nd_data_t gs_oaq_2nd_gen_data;
extern          TaskHandle_t               zmod4510_sensor_thread;

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
#if G_ZMOD4XXX_SENSOR1_IRQ_ENABLE
	FSP_PARAMETER_NOT_USED(p_args);

	gs_irq_callback_status = DEMO_CALLBACK_STATUS_SUCCESS;
#else
	FSP_PARAMETER_NOT_USED(p_args);
#endif
}

/* Quick setup for g_zmod4510_sensor0. */
void g_zmod4510_sensor0_quick_setup(void)
{
	fsp_err_t err;

	/* Reset ZMOD sensor (active low). Please change to the IO port connected to the RES_N pin of the ZMOD sensor on the customer board. */ 


	/* Open ZMOD4510 sensor instance, this must be done before calling any zmod4xxx API */
	err = g_zmod4xxx_sensor0.p_api->open(g_zmod4xxx_sensor0.p_ctrl, g_zmod4xxx_sensor0.p_cfg);
	assert(FSP_SUCCESS == err);

}


/* Quick getting humidity and temperature for g_zmod4xxx_sensor0. */
void g_zmod4510_sensor0_quick_getting_air_quality_data(rm_zmod4xxx_oaq_2nd_data_t * p_data)
{
	/* TODO: add your own code here */
	fsp_err_t               err;
	rm_zmod4xxx_raw_data_t  raw_data;
	demo_sequence_t         sequence    = DEMO_SEQUENCE_1;
	rm_zmod4xxx_lib_type_t  lib_type    = g_zmod4xxx_sensor0_extended_cfg.lib_type;
	float					temperature = 20.0F;
	float					humidity	= 50.0F;

	/* Clear status */
	gs_i2c_callback_status = DEMO_CALLBACK_STATUS_WAIT;
#if G_ZMOD4XXX_SENSOR1_IRQ_ENABLE
	gs_irq_callback_status = DEMO_CALLBACK_STATUS_WAIT;
#endif


	do
	{
		switch(sequence)
		{
		case DEMO_SEQUENCE_1 :
		{
			/* Clear status */
			gs_i2c_callback_status = DEMO_CALLBACK_STATUS_WAIT;

			/* Start measurement */
			err = RM_ZMOD4XXX_MeasurementStart(g_zmod4xxx_sensor0.p_ctrl);
			if (FSP_SUCCESS == err)
			{
				sequence = DEMO_SEQUENCE_2;
			}
			else
			{
				halt_func();
			}
		}
		break;

		case DEMO_SEQUENCE_2 :
		{
			/* Check I2C callback status */
			switch (gs_i2c_callback_status)
			{
			case DEMO_CALLBACK_STATUS_WAIT :
				break;
			case DEMO_CALLBACK_STATUS_SUCCESS :
				if (RM_ZMOD4510_LIB_TYPE_OAQ_2ND_GEN == lib_type)
				{
					/* See Table 4 in the ZMOD4510 Programming Manual. */
					vTaskDelay(2000);
				}
				sequence = DEMO_SEQUENCE_3;
				break;
			case DEMO_CALLBACK_STATUS_REPEAT :
				sequence = DEMO_SEQUENCE_1;
				break;
			default :
				halt_func();
				break;
			}
		}
		break;

#if G_ZMOD4XXX_SENSOR1_IRQ_ENABLE
		case DEMO_SEQUENCE_3 :
		{
			/* Check IRQ callback status */
			switch (gs_irq_callback_status)
			{
			case DEMO_CALLBACK_STATUS_WAIT :
				break;
			case DEMO_CALLBACK_STATUS_SUCCESS :
				gs_irq_callback_status = DEMO_CALLBACK_STATUS_WAIT;
				if (RM_ZMOD4510_LIB_TYPE_OAQ_2ND_GEN == lib_type)
				{
					sequence = DEMO_SEQUENCE_5;
				}
				else
				{
					sequence = DEMO_SEQUENCE_7;
				}
				break;
			default :
				halt_func();
				break;
			}
		}
		break;
#else
		case DEMO_SEQUENCE_3 :
		{
			/* Clear status */
			gs_i2c_callback_status = DEMO_CALLBACK_STATUS_WAIT;

			/* Get status */
			err = RM_ZMOD4XXX_StatusCheck(g_zmod4xxx_sensor0.p_ctrl);
			if (FSP_SUCCESS == err)
			{
				sequence = DEMO_SEQUENCE_4;
			}
			else
			{
				halt_func();
			}
		}
		break;

		case DEMO_SEQUENCE_4 :
		{
			/* Check I2C callback status */
			switch (gs_i2c_callback_status)
			{
			case DEMO_CALLBACK_STATUS_WAIT :
				break;
			case DEMO_CALLBACK_STATUS_SUCCESS :
				if (RM_ZMOD4510_LIB_TYPE_OAQ_2ND_GEN == lib_type)
				{
					sequence = DEMO_SEQUENCE_5;
				}
				else
				{
					sequence = DEMO_SEQUENCE_7;
				}
				break;
			case DEMO_CALLBACK_STATUS_REPEAT :
				sequence = DEMO_SEQUENCE_3;
				break;
			default :
				halt_func();
				break;
			}
		}
		break;
#endif
		case DEMO_SEQUENCE_5 :
		{
			/* Clear status */
			gs_i2c_callback_status = DEMO_CALLBACK_STATUS_WAIT;

			/* Check device error */
			err = RM_ZMOD4XXX_DeviceErrorCheck(g_zmod4xxx_sensor0.p_ctrl);
			if (FSP_SUCCESS == err)
			{
				sequence = DEMO_SEQUENCE_6;
			}
			else
			{
				halt_func();
			}
		}
		break;

		case DEMO_SEQUENCE_6 :
		{
			/* Check I2C callback status */
			switch (gs_i2c_callback_status)
			{
			case DEMO_CALLBACK_STATUS_WAIT :
				break;
			case DEMO_CALLBACK_STATUS_SUCCESS :
				sequence = DEMO_SEQUENCE_7;
				break;
			case DEMO_CALLBACK_STATUS_REPEAT :
				sequence = DEMO_SEQUENCE_5;
				break;
			case DEMO_CALLBACK_STATUS_DEVICE_ERROR :
			default :
				halt_func();
				break;
			}
		}
		break;


		case DEMO_SEQUENCE_7 :
		{
			/* Clear status */
			gs_i2c_callback_status = DEMO_CALLBACK_STATUS_WAIT;

			/* Read data */
			err = RM_ZMOD4XXX_Read(g_zmod4xxx_sensor0.p_ctrl, &raw_data);
			if (FSP_SUCCESS == err)
			{
				sequence = DEMO_SEQUENCE_8;
			}
			else if (FSP_ERR_SENSOR_MEASUREMENT_NOT_FINISHED == err)
			{
				sequence = DEMO_SEQUENCE_3;

				/* Delay 50ms */
				vTaskDelay(50);
			}
			else
			{
				halt_func();
			}
		}
		break;

		case DEMO_SEQUENCE_8 :
		{
			/* Check I2C callback status */
			switch (gs_i2c_callback_status)
			{
			case DEMO_CALLBACK_STATUS_WAIT :
				break;
			case DEMO_CALLBACK_STATUS_SUCCESS :
				if (RM_ZMOD4510_LIB_TYPE_OAQ_2ND_GEN == lib_type)
				{
					sequence = DEMO_SEQUENCE_9;
				}
				else
				{
					sequence = DEMO_SEQUENCE_11;
				}
				break;
			case DEMO_CALLBACK_STATUS_REPEAT :
				sequence = DEMO_SEQUENCE_7;
				break;
			default :
				halt_func();
				break;
			}
		}
		break;

		case DEMO_SEQUENCE_9 :
		{
			/* Clear status */
			gs_i2c_callback_status = DEMO_CALLBACK_STATUS_WAIT;

			/* Check device error */
			err = RM_ZMOD4XXX_DeviceErrorCheck(g_zmod4xxx_sensor0.p_ctrl);
			if (FSP_SUCCESS == err)
			{
				sequence = DEMO_SEQUENCE_10;
			}
			else
			{
				halt_func();
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
				sequence = DEMO_SEQUENCE_11;
				break;
			case DEMO_CALLBACK_STATUS_REPEAT :
				sequence = DEMO_SEQUENCE_9;
				break;
			case DEMO_CALLBACK_STATUS_DEVICE_ERROR :
			default :
				halt_func();
				break;
			}
		}
		break;

		case DEMO_SEQUENCE_11 :
		{
			/* Calculate data */
			switch (lib_type)
			{
			case RM_ZMOD4510_LIB_TYPE_OAQ_1ST_GEN :
				err = RM_ZMOD4XXX_Oaq1stGenDataCalculate(g_zmod4xxx_sensor0.p_ctrl,
						&raw_data,
						(rm_zmod4xxx_oaq_1st_data_t*)&gs_oaq_1st_gen_data);
				break;
			case RM_ZMOD4510_LIB_TYPE_OAQ_2ND_GEN :
				err = RM_ZMOD4XXX_TemperatureAndHumiditySet(g_zmod4xxx_sensor0.p_ctrl,
						temperature,
						humidity);
				if (err != FSP_SUCCESS)
				{
					halt_func();
				}
				err = RM_ZMOD4XXX_Oaq2ndGenDataCalculate(g_zmod4xxx_sensor0.p_ctrl,
						&raw_data,
						(rm_zmod4xxx_oaq_2nd_data_t*)&gs_oaq_2nd_gen_data);
				memcpy(p_data, (const void *)&gs_oaq_2nd_gen_data, sizeof (rm_zmod4xxx_oaq_2nd_data_t));
				break;
			default :
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
			else
			{
				halt_func();
			}

			sequence = DEMO_SEQUENCE_12;
		}
		break;

		default :
		{

		}
		break;
		}
	}while(sequence < DEMO_SEQUENCE_12);
}

