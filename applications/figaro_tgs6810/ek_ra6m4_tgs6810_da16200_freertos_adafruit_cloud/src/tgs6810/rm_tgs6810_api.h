/*
* Copyright (c) 2020 - 2024 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
*/

/*******************************************************************************************************************//**
 * @ingroup RENESAS_SENSOR_INTERFACES
 * @defgroup RM_TGS6810_API TGS6810 Middleware Interface
 * @brief Interface for TGS6810 Middleware functions.
 *
 * @section RM_TGS6810_API_Summary Summary
 * The TGS6810 interface provides TGS6810 functionality.
 *
 *
 * @{
 **********************************************************************************************************************/

#ifndef RM_TGS6810_API_H_
#define RM_TGS6810_API_H_

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#if defined(__CCRX__) || defined(__ICCRX__) || defined(__RX__)
 #include <string.h>
 #include "platform.h"
#elif defined(__CCRL__) || defined(__ICCRL__) || defined(__RL78__)
 #include <string.h>
 #include "r_cg_macrodriver.h"
 #include "r_fsp_error.h"
#else
 #include "bsp_api.h"
#endif

#include "rm_comms_api.h"

#if defined(__CCRX__) || defined(__ICCRX__) || defined(__RX__)
#elif defined(__CCRL__) || defined(__ICCRL__) || defined(__RL78__)
#else

/* Common macro for FSP header files. There is also a corresponding FSP_FOOTER macro at the end of this file. */
FSP_HEADER
#endif

/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

/**********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/** Event in the callback function */
typedef enum e_rm_tgs6810_event
{
    RM_TGS6810_EVENT_SUCCESS = 0,
    RM_TGS6810_EVENT_ERROR,
} rm_tgs6810_event_t;

/** Data type of TGS6810 */
typedef enum e_rm_tgs6810_data_type
{
    RM_TGS6810_HUMIDITY_DATA = 0,
    RM_TGS6810_TEMPERATURE_DATA,
} rm_tgs6810_data_type_t;

/** TGS6810 callback parameter definition */
typedef struct st_rm_tgs6810_callback_args
{
    void const      * p_context;
    rm_tgs6810_event_t event;
} rm_tgs6810_callback_args_t;

/** TGS6810 data */
typedef struct st_rm_tgs6810_data
{
    float temperature;
    float humidity;
    float gas;
} rm_tgs6810_data_t;

/** TGS6810 Configuration */
typedef struct st_rm_tgs6810_cfg
{
    rm_comms_instance_t const * p_instance;                  ///< Pointer to Communications Middleware instance.
    void const                * p_context;                   ///< Pointer to the user-provided context.
    void const                * p_extend;                    ///< Pointer to extended configuration by instance of interface.
    void (* p_callback)(rm_tgs6810_callback_args_t * p_args); ///< Pointer to callback function.
} rm_tgs6810_cfg_t;

/** TGS6810 control block.  Allocate an instance specific control block to pass into the TGS6810 API calls.
 */
typedef void rm_tgs6810_ctrl_t;

/** TGS6810 APIs */
typedef struct st_rm_tgs6810_api
{
    /** Open sensor.
     *
     * @param[in]  p_ctrl       Pointer to control structure.
     * @param[in]  p_cfg        Pointer to configuration structure.
     */
    fsp_err_t (* open)(rm_tgs6810_ctrl_t * const p_ctrl, rm_tgs6810_cfg_t const * const p_cfg);

    /** Request data from TGS6810.
     *
     * @param[in]  p_ctrl       Pointer to control structure.
     */
    fsp_err_t (* requestData)(rm_tgs6810_ctrl_t * const p_ctrl);

    /** Read data from TGS6810.
     *
     * @param[in]  p_ctrl       Pointer to control structure.
     * @param[in]  p_data       Pointer to data structure.
     */
    fsp_err_t (* read)(rm_tgs6810_ctrl_t * const p_ctrl, rm_tgs6810_data_t * const p_data);

    /** Close TGS6810.
     *
     * @param[in]  p_ctrl       Pointer to control structure.
     */
    fsp_err_t (* close)(rm_tgs6810_ctrl_t * const p_ctrl);
} rm_tgs6810_api_t;

/** TGS6810 instance */
typedef struct st_rm_tgs6810_instance
{
    rm_tgs6810_ctrl_t      * p_ctrl;    /**< Pointer to the control structure for this instance */
    rm_tgs6810_cfg_t const * p_cfg;     /**< Pointer to the configuration structure for this instance */
    rm_tgs6810_api_t const * p_api;     /**< Pointer to the API structure for this instance */
} rm_tgs6810_instance_t;

/**********************************************************************************************************************
 * Exported global variables
 **********************************************************************************************************************/

/**********************************************************************************************************************
 * Public Function Prototypes
 **********************************************************************************************************************/

#if defined(__CCRX__) || defined(__ICCRX__) || defined(__RX__)
#elif defined(__CCRL__) || defined(__ICCRL__) || defined(__RL78__)
#else

/* Common macro for FSP header files. There is also a corresponding FSP_FOOTER macro at the end of this file. */
FSP_FOOTER
#endif

#endif                                 /* RM_TGS6810_API_H_*/

/*******************************************************************************************************************//**
 * @} (end defgroup RM_TGS6810_API)
 **********************************************************************************************************************/
