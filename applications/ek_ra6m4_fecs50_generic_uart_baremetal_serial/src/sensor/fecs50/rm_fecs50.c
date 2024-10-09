/*
* Copyright (c) 2020 - 2024 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
*/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "rm_fecs50.h"

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

#define RM_FECS50_OPEN                               (0x433E4432UL) // Open state
#define RM_FECS50_WAIT_TIME                          (200)
#define RM_FECS50_REQCOMMAND                         (0x80)
#define RM_FECS50_DATASIZE         (12)
#define RM_FECS50_PACKSIZE         (4)


/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Global function prototypes
 **********************************************************************************************************************/
void fecs50_callback(rm_comms_callback_args_t * p_args);

/***********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/
fsp_err_t rm_fecs50_delay_us(rm_fecs50_instance_ctrl_t * const p_ctrl, uint32_t const delay_us);

/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Global variables
 **********************************************************************************************************************/
rm_fecs50_api_t const g_fecs50_on_fecs50 =
{
    .open                 = RM_FECS50_Open,
    .close                = RM_FECS50_Close,
    .read                 = RM_FECS50_Read,
};

volatile bool g_i2c_completed = false;
volatile bool g_i2c_nack      = false;

/*******************************************************************************************************************//**
 * @addtogroup RM_FECS50
 * @{
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Functions
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @brief Opens and configures the FECS50 Middle module. Implements @ref rm_fecs50_api_t::open.
 *
 * Example:
 * @snippet rm_fecs50_example.c RM_FECS50_Open
 *
 * @retval FSP_SUCCESS              FECS50 successfully configured.
 * @retval FSP_ERR_ASSERTION        Null pointer, or one or more configuration options is invalid.
 * @retval FSP_ERR_ALREADY_OPEN     Module is already open.  This module can only be opened once.
 **********************************************************************************************************************/
fsp_err_t RM_FECS50_Open (rm_fecs50_ctrl_t * const p_api_ctrl, rm_fecs50_cfg_t const * const p_cfg)
{
    fsp_err_t err = FSP_SUCCESS;
    rm_fecs50_instance_ctrl_t * p_ctrl = (rm_fecs50_instance_ctrl_t *) p_api_ctrl;

    p_ctrl->p_cfg                  = p_cfg;
    p_ctrl->p_comms_i2c_instance   = p_cfg->p_instance;
    p_ctrl->p_context              = p_cfg->p_context;
    p_ctrl->p_callback             = p_cfg->p_callback;
    p_ctrl->programming_mode.enter = false;

    /* Open Communications middleware */
    err = p_ctrl->p_comms_i2c_instance->p_api->open(p_ctrl->p_comms_i2c_instance->p_ctrl,
                                                    p_ctrl->p_comms_i2c_instance->p_cfg);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    /* Set open flag */
    p_ctrl->open = RM_FECS50_OPEN;

    return FSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief Disables specified FECS50 control block. Implements @ref rm_fecs50_api_t::close.
 *
 * @retval FSP_SUCCESS              Successfully closed.
 * @retval FSP_ERR_ASSERTION        Null pointer passed as a parameter.
 * @retval FSP_ERR_NOT_OPEN         Module is not open.
 **********************************************************************************************************************/
fsp_err_t RM_FECS50_Close (rm_fecs50_ctrl_t * const p_api_ctrl)
{
    rm_fecs50_instance_ctrl_t * p_ctrl = (rm_fecs50_instance_ctrl_t *) p_api_ctrl;

#if RM_FECS50_CFG_PARAM_CHECKING_ENABLE
    FSP_ASSERT(NULL != p_ctrl);
    FSP_ERROR_RETURN(RM_FECS50_OPEN == p_ctrl->open, FSP_ERR_NOT_OPEN);
#endif

    /* Close Communications Middleware */
    p_ctrl->p_comms_i2c_instance->p_api->close(p_ctrl->p_comms_i2c_instance->p_ctrl);

    /* Clear Open flag */
    p_ctrl->open = 0;

    return FSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief Reads data from FECS50.
 * Implements @ref rm_fecs50_api_t::read.
 *
 * @retval FSP_SUCCESS              Successfully data decoded.
 * @retval FSP_ERR_ASSERTION        Null pointer, or one or more configuration options is invalid.
 * @retval FSP_ERR_NOT_OPEN         Module is not open.
 **********************************************************************************************************************/
fsp_err_t RM_FECS50_Read (rm_fecs50_ctrl_t * const p_api_ctrl, rm_fecs50_data_t * const p_data)
{
    fsp_err_t err = FSP_SUCCESS;
    rm_fecs50_instance_ctrl_t * p_ctrl = (rm_fecs50_instance_ctrl_t *) p_api_ctrl;
    uint8_t write_data = RM_FECS50_REQCOMMAND;
    uint8_t read_data[RM_FECS50_DATASIZE];

    /* Request data command to FECS50 */
    g_i2c_completed = false;
    g_i2c_nack      = false;

    err = p_ctrl->p_comms_i2c_instance->p_api->write(p_ctrl->p_comms_i2c_instance->p_ctrl, &write_data, 1);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    while (!g_i2c_completed && !g_i2c_nack)
    {
        /* Wait callback */
    }

    if(g_i2c_nack)
    {
        return FSP_ERR_INVALID_HW_CONDITION;
    }

    rm_fecs50_delay_us(p_api_ctrl, RM_FECS50_WAIT_TIME);

    /* Read FECS50 data */
    g_i2c_completed = false;
    g_i2c_nack      = false;

    err = p_ctrl->p_comms_i2c_instance->p_api->read(p_ctrl->p_comms_i2c_instance->p_ctrl, read_data, RM_FECS50_DATASIZE);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    while (!g_i2c_completed && !g_i2c_nack)
    {
        /* Wait callback */
    }

    if(g_i2c_nack)
    {
        return FSP_ERR_INVALID_HW_CONDITION;
    }
    else
    {
        for(int i = 0; i < RM_FECS50_PACKSIZE; i++)
        {
            *((unsigned char*)&p_data->temperature +i) = read_data[i];
            *((unsigned char*)&p_data->humidity    +i) = read_data[i+RM_FECS50_PACKSIZE];
            *((unsigned char*)&p_data->gas         +i) = read_data[i+RM_FECS50_PACKSIZE*2];
        }
    }

    return FSP_SUCCESS;
}


/*******************************************************************************************************************//**
 * @} (end addtogroup FECS50)
 **********************************************************************************************************************/
/*******************************************************************************************************************//**
 * @brief FECS50 callback function called in the I2C Communications Middleware callback function.
 **********************************************************************************************************************/
void fecs50_callback(rm_comms_callback_args_t * p_args)
{
    if (RM_COMMS_EVENT_OPERATION_COMPLETE == p_args->event)
    {
        g_i2c_completed = true;
    }
    if (RM_COMMS_EVENT_ERROR == p_args->event)
    {
    	g_i2c_nack = true;
    }
}

/***********************************************************************************************************************
 * Private Functions
 **********************************************************************************************************************/
/*******************************************************************************************************************//**
 * @brief Delay some microseconds.
 *
 * @retval FSP_SUCCESS              successfully configured.
 **********************************************************************************************************************/
fsp_err_t rm_fecs50_delay_us (rm_fecs50_instance_ctrl_t * const p_ctrl, uint32_t const delay_us)
{
    FSP_PARAMETER_NOT_USED(p_ctrl);

    /* Software delay */
    R_BSP_SoftwareDelay(delay_us, BSP_DELAY_UNITS_MICROSECONDS);

    return FSP_SUCCESS;
}


/*******************************************************************************************************************//**
 * Internal fecs50 private function.
 **********************************************************************************************************************/
