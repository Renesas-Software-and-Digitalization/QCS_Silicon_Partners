/*
* Copyright (c) 2020 - 2024 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
*/

/*******************************************************************************************************************//**
 * @ingroup RENESAS_SENSOR_INTERFACES
 * @defgroup RM_TGS5141_API TGS5141 Middleware Interface
 * @brief Interface for TGS5141 Middleware functions.
 *
 * @section RM_TGS5141_API_Summary Summary
 * The TGS5141 interface provides TGS5141 functionality.
 *
 *
 * @{
 **********************************************************************************************************************/

#ifndef RM_TGS5141_API_H_
#define RM_TGS5141_API_H_

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
typedef enum e_rm_tgs5141_event
{
    RM_TGS5141_EVENT_SUCCESS = 0,
    RM_TGS5141_EVENT_ERROR,
} rm_tgs5141_event_t;

/** Data type of TGS5141 */
typedef enum e_rm_tgs5141_data_type
{
    RM_TGS5141_HUMIDITY_DATA = 0,
    RM_TGS5141_TEMPERATURE_DATA,
} rm_tgs5141_data_type_t;

/** TGS5141 callback parameter definition */
typedef struct st_rm_tgs5141_callback_args
{
    void const      * p_context;
    rm_tgs5141_event_t event;
} rm_tgs5141_callback_args_t;

/** TGS5141 data */
typedef struct st_rm_tgs5141_data
{
    float temperature;
    float humidity;
    float gas;
} rm_tgs5141_data_t;

/** TGS5141 Configuration */
typedef struct st_rm_tgs5141_cfg
{
    rm_comms_instance_t const * p_instance;                  ///< Pointer to Communications Middleware instance.
    void const                * p_context;                   ///< Pointer to the user-provided context.
    void const                * p_extend;                    ///< Pointer to extended configuration by instance of interface.
    void (* p_callback)(rm_tgs5141_callback_args_t * p_args); ///< Pointer to callback function.
} rm_tgs5141_cfg_t;

/** TGS5141 control block.  Allocate an instance specific control block to pass into the TGS5141 API calls.
 */
typedef void rm_tgs5141_ctrl_t;

/** TGS5141 APIs */
typedef struct st_rm_tgs5141_api
{
    /** Open sensor.
     *
     * @param[in]  p_ctrl       Pointer to control structure.
     * @param[in]  p_cfg        Pointer to configuration structure.
     */
    fsp_err_t (* open)(rm_tgs5141_ctrl_t * const p_ctrl, rm_tgs5141_cfg_t const * const p_cfg);

    /** Request data from TGS5141.
     *
     * @param[in]  p_ctrl       Pointer to control structure.
     */
    fsp_err_t (* requestData)(rm_tgs5141_ctrl_t * const p_ctrl);

    /** Read data from TGS5141.
     *
     * @param[in]  p_ctrl       Pointer to control structure.
     * @param[in]  p_data       Pointer to data structure.
     */
    fsp_err_t (* read)(rm_tgs5141_ctrl_t * const p_ctrl, rm_tgs5141_data_t * const p_data);

    /** Close TGS5141.
     *
     * @param[in]  p_ctrl       Pointer to control structure.
     */
    fsp_err_t (* close)(rm_tgs5141_ctrl_t * const p_ctrl);
} rm_tgs5141_api_t;

/** TGS5141 instance */
typedef struct st_rm_tgs5141_instance
{
    rm_tgs5141_ctrl_t      * p_ctrl;    /**< Pointer to the control structure for this instance */
    rm_tgs5141_cfg_t const * p_cfg;     /**< Pointer to the configuration structure for this instance */
    rm_tgs5141_api_t const * p_api;     /**< Pointer to the API structure for this instance */
} rm_tgs5141_instance_t;

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

#endif                                 /* RM_TGS5141_API_H_*/

/*******************************************************************************************************************//**
 * @} (end defgroup RM_TGS5141_API)
 **********************************************************************************************************************/
