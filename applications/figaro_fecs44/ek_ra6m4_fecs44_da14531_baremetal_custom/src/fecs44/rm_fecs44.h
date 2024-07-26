/*
* Copyright (c) 2020 - 2024 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
*/

/*******************************************************************************************************************//**
 * @addtogroup RM_FECS44
 * @{
 **********************************************************************************************************************/

#ifndef RM_FECS44_H
#define RM_FECS44_H

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "rm_fecs44_api.h"

#if defined(__CCRX__) || defined(__ICCRX__) || defined(__RX__)
 #include "r_fecs44_rx_config.h"
#elif defined(__CCRL__) || defined(__ICCRL__) || defined(__RL78__)
 #include "r_fecs44_rl_config.h"
#else
 #include "rm_fecs44_cfg.h"

/* Common macro for FSP header files. There is also a corresponding FSP_FOOTER macro at the end of this file. */
FSP_HEADER
#endif

/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/** FECS44 programming mode process block */
typedef struct st_rm_fecs44_programmnig_mode_params
{
    volatile bool               enter;                  ///< Enter flag.
    volatile bool               blocking;               ///< Blocking flag.
    volatile bool               communication_finished; ///< Communication flag for blocking.
    volatile rm_fecs44_event_t event;                  ///< Callback event
} rm_fecs44_programmnig_mode_params_t;

/** FECS44 Control Block */
typedef struct rm_fecs44_instance_ctrl
{
    uint32_t                             open;                 ///< Open flag
    rm_fecs44_cfg_t const             * p_cfg;                ///< Pointer to FECS44 Configuration
    rm_comms_instance_t const          * p_comms_i2c_instance; ///< Pointer of I2C Communications Middleware instance structure
    void const                         * p_context;            ///< Pointer to the user-provided context
    rm_fecs44_programmnig_mode_params_t programming_mode;     ///< Programming mode flag
    uint8_t buf[3];                                            ///< Buffer for I2C communications

    /* Pointer to callback and optional working memory */
    void (* p_callback)(rm_fecs44_callback_args_t * p_args);
} rm_fecs44_instance_ctrl_t;

/**********************************************************************************************************************
 * Exported global variables
 **********************************************************************************************************************/

/** @cond INC_HEADER_DEFS_SEC */
/** Filled in Interface API structure for this Instance. */
extern rm_fecs44_api_t const g_fecs44_on_fecs44;

/** @endcond */

/**********************************************************************************************************************
 * Public Function Prototypes
 **********************************************************************************************************************/
fsp_err_t RM_FECS44_Open(rm_fecs44_ctrl_t * const p_api_ctrl, rm_fecs44_cfg_t const * const p_cfg);
fsp_err_t RM_FECS44_Close(rm_fecs44_ctrl_t * const p_api_ctrl);
fsp_err_t RM_FECS44_Read(rm_fecs44_ctrl_t * const p_api_ctrl, rm_fecs44_data_t * const p_raw_data);

#if defined(__CCRX__) || defined(__ICCRX__) || defined(__RX__)
#elif defined(__CCRL__) || defined(__ICCRL__) || defined(__RL78__)
#else

/* Common macro for FSP header files. There is also a corresponding FSP_FOOTER macro at the end of this file. */
FSP_FOOTER
#endif

#endif                                 /* RM_FECS44_H_*/

/*******************************************************************************************************************//**
 * @} (end addtogroup RM_FECS44)
 **********************************************************************************************************************/
