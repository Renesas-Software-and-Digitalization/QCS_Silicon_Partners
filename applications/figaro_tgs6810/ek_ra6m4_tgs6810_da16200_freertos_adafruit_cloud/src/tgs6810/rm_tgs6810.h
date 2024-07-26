/*
* Copyright (c) 2020 - 2024 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
*/

/*******************************************************************************************************************//**
 * @addtogroup RM_TGS6810
 * @{
 **********************************************************************************************************************/

#ifndef RM_TGS6810_H
#define RM_TGS6810_H

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "rm_tgs6810_api.h"

#if defined(__CCRX__) || defined(__ICCRX__) || defined(__RX__)
 #include "r_tgs6810_rx_config.h"
#elif defined(__CCRL__) || defined(__ICCRL__) || defined(__RL78__)
 #include "r_tgs6810_rl_config.h"
#else
 #include "rm_tgs6810_cfg.h"

/* Common macro for FSP header files. There is also a corresponding FSP_FOOTER macro at the end of this file. */
FSP_HEADER
#endif

/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/** TGS6810 programming mode process block */
typedef struct st_rm_tgs6810_programmnig_mode_params
{
    volatile bool               enter;                  ///< Enter flag.
    volatile bool               blocking;               ///< Blocking flag.
    volatile bool               communication_finished; ///< Communication flag for blocking.
    volatile rm_tgs6810_event_t event;                  ///< Callback event
} rm_tgs6810_programmnig_mode_params_t;

/** TGS6810 Control Block */
typedef struct rm_tgs6810_instance_ctrl
{
    uint32_t                             open;                 ///< Open flag
    rm_tgs6810_cfg_t const             * p_cfg;                ///< Pointer to TGS6810 Configuration
    rm_comms_instance_t const          * p_comms_i2c_instance; ///< Pointer of I2C Communications Middleware instance structure
    void const                         * p_context;            ///< Pointer to the user-provided context
    rm_tgs6810_programmnig_mode_params_t programming_mode;     ///< Programming mode flag
    uint8_t buf[3];                                            ///< Buffer for I2C communications

    /* Pointer to callback and optional working memory */
    void (* p_callback)(rm_tgs6810_callback_args_t * p_args);
} rm_tgs6810_instance_ctrl_t;

/**********************************************************************************************************************
 * Exported global variables
 **********************************************************************************************************************/

/** @cond INC_HEADER_DEFS_SEC */
/** Filled in Interface API structure for this Instance. */
extern rm_tgs6810_api_t const g_tgs6810_on_tgs6810;

/** @endcond */

/**********************************************************************************************************************
 * Public Function Prototypes
 **********************************************************************************************************************/
fsp_err_t RM_TGS6810_Open(rm_tgs6810_ctrl_t * const p_api_ctrl, rm_tgs6810_cfg_t const * const p_cfg);
fsp_err_t RM_TGS6810_Close(rm_tgs6810_ctrl_t * const p_api_ctrl);
fsp_err_t RM_TGS6810_Read(rm_tgs6810_ctrl_t * const p_api_ctrl, rm_tgs6810_data_t * const p_raw_data);

#if defined(__CCRX__) || defined(__ICCRX__) || defined(__RX__)
#elif defined(__CCRL__) || defined(__ICCRL__) || defined(__RL78__)
#else

/* Common macro for FSP header files. There is also a corresponding FSP_FOOTER macro at the end of this file. */
FSP_FOOTER
#endif

#endif                                 /* RM_TGS6810_H_*/

/*******************************************************************************************************************//**
 * @} (end addtogroup RM_TGS6810)
 **********************************************************************************************************************/
