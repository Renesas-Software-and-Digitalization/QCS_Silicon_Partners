#include "hal_data.h"
#include "common_data.h"
#include "zmod4510_sensor.h"
/*
 * Program flow for OAQ 2nd gen is changed. Please refer to the programming manual [R36US0004EU] and the application note[R01AN5899].\n
 * R01AN5899 : https://www.renesas.com/document/apn/zmod4xxx-sample-application
 * R36US0004EU : https://www.renesas.com/document/mat/zmod4510-programming-manual-read-me
 */

/* Set period for delay */
#define ZMOD4510_DELAY_PERIOD (100)

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
typedef enum e_zmod4510_delay_units
{
    ZMOD4510_DELAY_MICROSECS = (0),     /* Requested delay amount is in microseconds */
    ZMOD4510_DELAY_MILLISECS,           /* Requested delay amount is in milliseconds */
    ZMOD4510_DELAY_SECS                 /* Requested delay amount is in seconds */
} zmod4510_delay_units_t;


static void zmod4510_delay_quick_setup(void);
static void zmod4510_delay_start(uint32_t delay, zmod4510_delay_units_t units);
static bool zmod4510_delay_wait(void);

static volatile demo_callback_status_t     gs_i2c_callback_status = DEMO_CALLBACK_STATUS_WAIT;
#if G_ZMOD4XXX_SENSOR0_IRQ_ENABLE
static volatile demo_callback_status_t     gs_irq_callback_status = DEMO_CALLBACK_STATUS_WAIT;
#endif

static volatile rm_zmod4xxx_oaq_1st_data_t gs_oaq_1st_gen_data;
static volatile rm_zmod4xxx_oaq_2nd_data_t gs_oaq_2nd_gen_data;
static rm_zmod4xxx_raw_data_t              gs_zmod4510_raw_data;
static demo_sequence_t                     gs_zmod4510_sequence = DEMO_SEQUENCE_1;
static uint32_t                            gs_zmod4510_delay_count = 0;

/* Delay time definition table */
static const uint32_t gs_zmod4510_delay_time[] = {
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

/* Quick setup for g_zmod4xxx_sensor0. */
void g_zmod4510_sensor0_quick_setup(void)
{
    fsp_err_t err;

    /* Clear status */
    gs_i2c_callback_status = DEMO_CALLBACK_STATUS_WAIT;
#if G_ZMOD4XXX_SENSOR0_IRQ_ENABLE
    gs_irq_callback_status = DEMO_CALLBACK_STATUS_WAIT;
#endif

    /* Reset ZMOD sensor (active low). Please change to the IO port connected to the RES_N pin of the ZMOD sensor on the customer board. */

	/* Open ZMOD4510 sensor instance, this must be done before calling any zmod4xxx API */
	err = g_zmod4xxx_sensor0.p_api->open(g_zmod4xxx_sensor0.p_ctrl, g_zmod4xxx_sensor0.p_cfg);
	assert(FSP_SUCCESS == err);

    /* Open timer */
    zmod4510_delay_quick_setup();
}

/* Quick setup for zmod4510_delay */
static void zmod4510_delay_quick_setup(void)
{
    fsp_err_t err;

    /* Open timer instance, this must be done before calling any timer API */
    err = zmod4510_delay.p_api->open(zmod4510_delay.p_ctrl, zmod4510_delay.p_cfg);
    if (FSP_SUCCESS != err)
    {
        halt_func();
    }
}

static void zmod4510_delay_start(uint32_t delay, zmod4510_delay_units_t units)
{
    fsp_err_t err;

    /* Convert to units of ZMOD4510_DELAY_PERIOD */
    gs_zmod4510_delay_count = (delay * gs_zmod4510_delay_time[units]) / ZMOD4510_DELAY_PERIOD;

    /* Stop timer */
    err = zmod4510_delay.p_api->stop(zmod4510_delay.p_ctrl);
    if (FSP_SUCCESS != err)
    {
        halt_func();
    }

    /* Reset counter */
    err = zmod4510_delay.p_api->reset(zmod4510_delay.p_ctrl);
    if (FSP_SUCCESS != err)
    {
        halt_func();
    }

    /* Start timer */
    err = zmod4510_delay.p_api->start(zmod4510_delay.p_ctrl);
    if (FSP_SUCCESS != err)
    {
        halt_func();
    }
}

static bool zmod4510_delay_wait(void)
{
    fsp_err_t err;
    bool      wait;

    if (gs_zmod4510_delay_count > 0)
    {
        wait = true;
    }
    else
    {
        /* Stop timer */
        err = zmod4510_delay.p_api->stop(zmod4510_delay.p_ctrl);
        if (FSP_SUCCESS != err)
        {
            halt_func();
        }

        wait = false;
    }

    return wait;
}

/* Timer count down */
void zmod4510_delay_callback(timer_callback_args_t *p_args)
{
    FSP_PARAMETER_NOT_USED(p_args);

    if (gs_zmod4510_delay_count > 0)
    {
        gs_zmod4510_delay_count--;
    }
}

void g_zmod4510_sensor0_quick_getting_air_quality_data(rm_zmod4xxx_oaq_2nd_data_t * p_data)
{
    fsp_err_t               err;
    rm_zmod4xxx_lib_type_t  lib_type      = g_zmod4xxx_sensor0_extended_cfg.lib_type;
    float                   temperature   = 20.0F;
    float                   humidity      = 50.0F;
    bool                    sensor_change = false;

    do
    {
        switch(gs_zmod4510_sequence)
        {
            case DEMO_SEQUENCE_1 :
            {
                /* Clear status */
                gs_i2c_callback_status = DEMO_CALLBACK_STATUS_WAIT;

                /* Start measurement */
                err = RM_ZMOD4XXX_MeasurementStart(g_zmod4xxx_sensor0.p_ctrl);
                if (FSP_SUCCESS == err)
                {
                    gs_zmod4510_sequence = DEMO_SEQUENCE_2;
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
                        sensor_change = true;
                        if (RM_ZMOD4510_LIB_TYPE_OAQ_2ND_GEN == lib_type)
                        {
                            /* Set delay period to 2000ms */
                            /* See Table 4 in the ZMOD4510 Programming Manual. */
                            zmod4510_delay_start(2000, ZMOD4510_DELAY_MILLISECS);
                            gs_zmod4510_sequence = DEMO_SEQUENCE_3;
                        }
                        else
                        {
                            gs_zmod4510_sequence = DEMO_SEQUENCE_4;
                        }
                        break;
                    case DEMO_CALLBACK_STATUS_REPEAT :
                        gs_zmod4510_sequence = DEMO_SEQUENCE_1;
                        break;
                    default :
                        halt_func();
                        break;
                }
            }
            break;

            case DEMO_SEQUENCE_3 :
            {
                /* Wait delay period */
                if (zmod4510_delay_wait() == true)
                {
                    sensor_change = true;
                }
                else
                {
                    gs_zmod4510_sequence = DEMO_SEQUENCE_4;
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
                        if (RM_ZMOD4510_LIB_TYPE_OAQ_2ND_GEN == lib_type)
                        {
                            gs_zmod4510_sequence = DEMO_SEQUENCE_6;
                        }
                        else
                        {
                            gs_zmod4510_sequence = DEMO_SEQUENCE_8;
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
                    gs_zmod4510_sequence = DEMO_SEQUENCE_5;
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
                        if (RM_ZMOD4510_LIB_TYPE_OAQ_2ND_GEN == lib_type)
                        {
                            gs_zmod4510_sequence = DEMO_SEQUENCE_6;
                        }
                        else
                        {
                            gs_zmod4510_sequence = DEMO_SEQUENCE_8;
                        }
                        break;
                    case DEMO_CALLBACK_STATUS_REPEAT :
                        gs_zmod4510_sequence = DEMO_SEQUENCE_4;
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
                    gs_zmod4510_sequence = DEMO_SEQUENCE_7;
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
                        gs_zmod4510_sequence = DEMO_SEQUENCE_8;
                        break;
                    case DEMO_CALLBACK_STATUS_REPEAT :
                        gs_zmod4510_sequence = DEMO_SEQUENCE_6;
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
                err = RM_ZMOD4XXX_Read(g_zmod4xxx_sensor0.p_ctrl, &gs_zmod4510_raw_data);
                if (FSP_SUCCESS == err)
                {
                    gs_zmod4510_sequence = DEMO_SEQUENCE_10;
                }
                else if (FSP_ERR_SENSOR_MEASUREMENT_NOT_FINISHED == err)
                {
                    /* Set delay period to 50ms */
                    zmod4510_delay_start(50, ZMOD4510_DELAY_MILLISECS);
                    gs_zmod4510_sequence = DEMO_SEQUENCE_9;
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
                if (zmod4510_delay_wait() == true)
                {
                    sensor_change = true;
                }
                else
                {
                    gs_zmod4510_sequence = DEMO_SEQUENCE_4;
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
                        if (RM_ZMOD4510_LIB_TYPE_OAQ_2ND_GEN == lib_type)
                        {
                            gs_zmod4510_sequence = DEMO_SEQUENCE_11;
                        }
                        else
                        {
                            gs_zmod4510_sequence = DEMO_SEQUENCE_13;
                        }
                        break;
                    case DEMO_CALLBACK_STATUS_REPEAT :
                        gs_zmod4510_sequence = DEMO_SEQUENCE_8;
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
                    gs_zmod4510_sequence = DEMO_SEQUENCE_12;
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
                        gs_zmod4510_sequence = DEMO_SEQUENCE_13;
                        break;
                    case DEMO_CALLBACK_STATUS_REPEAT :
                        gs_zmod4510_sequence = DEMO_SEQUENCE_11;
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
                    case RM_ZMOD4510_LIB_TYPE_OAQ_1ST_GEN :
                        err = RM_ZMOD4XXX_Oaq1stGenDataCalculate(g_zmod4xxx_sensor0.p_ctrl,
                                                                 &gs_zmod4510_raw_data,
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
                                                                 &gs_zmod4510_raw_data,
                                                                 (rm_zmod4xxx_oaq_2nd_data_t*)&gs_oaq_2nd_gen_data);
                        memcpy(p_data, (const void *)&gs_oaq_2nd_gen_data, sizeof (rm_zmod4xxx_oaq_2nd_data_t));
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
                else
                {
                    halt_func();
                }

                gs_zmod4510_sequence = DEMO_SEQUENCE_1;
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
