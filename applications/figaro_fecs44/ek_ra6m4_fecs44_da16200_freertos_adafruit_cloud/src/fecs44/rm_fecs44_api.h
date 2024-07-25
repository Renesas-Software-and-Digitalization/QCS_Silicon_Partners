/*
* Copyright (c) 2020 - 2024 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
*/

/*******************************************************************************************************************//**
 * @ingroup RENESAS_SENSOR_INTERFACES
 * @defgroup RM_FECS44_API FECS44 Middleware Interface
 * @brief Interface for FECS44 Middleware functions.
 *
 * @section RM_FECS44_API_Summary Summary
 * The FECS44 interface provides FECS44 functionality.
 *
 *
 * @{
 **********************************************************************************************************************/

#ifndef RM_FECS44_API_H_
#define RM_FECS44_API_H_

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
typedef enum e_rm_fecs44_event
{
    RM_FECS44_EVENT_SUCCESS = 0,
    RM_FECS44_EVENT_ERROR,
} rm_fecs44_event_t;

/** Data type of FECS44 */
typedef enum e_rm_fecs44_data_type
{
    RM_FECS44_HUMIDITY_DATA = 0,
    RM_FECS44_TEMPERATURE_DATA,
} rm_fecs44_data_type_t;

/** FECS44 callback parameter definition */
typedef struct st_rm_fecs44_callback_args
{
    void const      * p_context;
    rm_fecs44_event_t event;
} rm_fecs44_callback_args_t;

/** FECS44 data */
typedef struct st_rm_fecs44_data
{
    float temperature;
    float humidity;
    float gas;
} rm_fecs44_data_t;

/** FECS44 Configuration */
typedef struct st_rm_fecs44_cfg
{
    rm_comms_instance_t const * p_instance;                  ///< Pointer to Communications Middleware instance.
    void const                * p_context;                   ///< Pointer to the user-provided context.
    void const                * p_extend;                    ///< Pointer to extended configuration by instance of interface.
    void (* p_callback)(rm_fecs44_callback_args_t * p_args); ///< Pointer to callback function.
} rm_fecs44_cfg_t;

/** FECS44 control block.  Allocate an instance specific control block to pass into the FECS44 API calls.
 */
typedef void rm_fecs44_ctrl_t;

/** FECS44 APIs */
typedef struct st_rm_fecs44_api
{
    /** Open sensor.
     *
     * @param[in]  p_ctrl       Pointer to control structure.
     * @param[in]  p_cfg        Pointer to configuration structure.
     */
    fsp_err_t (* open)(rm_fecs44_ctrl_t * const p_ctrl, rm_fecs44_cfg_t const * const p_cfg);

    /** Request data from FECS44.
     *
     * @param[in]  p_ctrl       Pointer to control structure.
     */
    fsp_err_t (* requestData)(rm_fecs44_ctrl_t * const p_ctrl);

    /** Read data from FECS44.
     *
     * @param[in]  p_ctrl       Pointer to control structure.
     * @param[in]  p_data       Pointer to data structure.
     */
    fsp_err_t (* read)(rm_fecs44_ctrl_t * const p_ctrl, rm_fecs44_data_t * const p_data);

    /** Close FECS44.
     *
     * @param[in]  p_ctrl       Pointer to control structure.
     */
    fsp_err_t (* close)(rm_fecs44_ctrl_t * const p_ctrl);
} rm_fecs44_api_t;

/** FECS44 instance */
typedef struct st_rm_fecs44_instance
{
    rm_fecs44_ctrl_t      * p_ctrl;    /**< Pointer to the control structure for this instance */
    rm_fecs44_cfg_t const * p_cfg;     /**< Pointer to the configuration structure for this instance */
    rm_fecs44_api_t const * p_api;     /**< Pointer to the API structure for this instance */
} rm_fecs44_instance_t;

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

#endif                                 /* RM_FECS44_API_H_*/

/*******************************************************************************************************************//**
 * @} (end defgroup RM_FECS44_API)
 **********************************************************************************************************************/
