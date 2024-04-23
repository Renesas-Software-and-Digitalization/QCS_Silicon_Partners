#ifndef _I2COP_H_
#define _I2COP_H_

#include "common_utils.h"
#include "hal_data.h"

#if (BSP_CFG_RTOS == 0)
#define TMF8828_TIMEOUT 	1000U
static bool g_comm_i2c_flag = false;
static rm_comms_event_t g_comm_i2c_event;
#endif

void tmf8828_callback(rm_comms_callback_args_t *p_args)
{
#if (BSP_CFG_RTOS > 0)
    FSP_PARAMETER_NOT_USED(p_args);
#else
    /* Set event */
    switch (p_args->event)
    {
        case RM_COMMS_EVENT_OPERATION_COMPLETE:
        {
            g_comm_i2c_event = RM_COMMS_EVENT_OPERATION_COMPLETE;

            g_comm_i2c_flag = true;

            break;
        }

        case RM_COMMS_EVENT_ERROR:
            g_comm_i2c_event = RM_COMMS_EVENT_ERROR;
            break;
        default:
        {
            break;
        }
    }
#endif
}

void delay(int t);

void delay(int t) {
	R_BSP_SoftwareDelay((uint32_t)t, BSP_DELAY_UNITS_MILLISECONDS);
}

static fsp_err_t validate_i2c_event(void)
{
#if (BSP_CFG_RTOS == 0)
    uint16_t counter = 0;
    g_comm_i2c_flag = false;
    /* Wait callback */
    while (false == g_comm_i2c_flag)
    {
    	delay(1);
        counter++;
        FSP_ERROR_RETURN(TMF8828_TIMEOUT >= counter, FSP_ERR_TIMEOUT);
    }
    /* Check callback event */
    FSP_ERROR_RETURN(RM_COMMS_EVENT_OPERATION_COMPLETE == g_comm_i2c_event, FSP_ERR_ABORTED);
#else
    return FSP_SUCCESS;
#endif
}

static fsp_err_t I2CStart() {
    g_comms_i2c_tmf8828.p_api->close(g_comms_i2c_tmf8828.p_ctrl);
    fsp_err_t err = g_comms_i2c_tmf8828.p_api->open(g_comms_i2c_tmf8828.p_ctrl, g_comms_i2c_tmf8828.p_cfg);
    if (FSP_SUCCESS != err) {
        APP_ERR_PRINT("** g_i2c0.p_api->open API failed ** \r\n");
    }
    return err;
}

static fsp_err_t I2CRead(uint8_t *buff, uint32_t buff_len, uint8_t restart) {
    fsp_err_t err = g_comms_i2c_tmf8828.p_api->read(g_comms_i2c_tmf8828.p_ctrl, buff, buff_len);
    FSP_PARAMETER_NOT_USED(restart);
    validate_i2c_event();
    if (FSP_SUCCESS != err) {
        APP_ERR_PRINT("** I2C Read Failed ** \r\n");
    }
    return err;
}

static fsp_err_t I2CWrite(uint8_t *buff, uint32_t buff_len, uint8_t restart) {
    fsp_err_t err = g_comms_i2c_tmf8828.p_api->write(g_comms_i2c_tmf8828.p_ctrl, buff, buff_len);
    FSP_PARAMETER_NOT_USED(restart);
    validate_i2c_event();
    if (err != FSP_SUCCESS) {
        APP_ERR_PRINT("** I2C Write Failed ** \r\n");
    }
    return err;
}

#endif //_I2COP_H_
