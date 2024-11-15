/*
* Copyright (c) 2020 - 2024 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
*/

/*******************************************************************************************************************//**
 * @ingroup RENESAS_SENSOR_INTERFACES
 * @defgroup RM_FECS43_API FECS43 Middleware Interface
 * @brief Interface for FECS43 Middleware functions.
 *
 * @section RM_FECS43_API_Summary Summary
 * The FECS43 interface provides FECS43 functionality.
 *
 *
 * @{
 **********************************************************************************************************************/

#ifndef RM_FECS43_API_H_
#define RM_FECS43_API_H_

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
typedef enum e_rm_fecs43_event
{
    RM_FECS43_EVENT_SUCCESS = 0,
    RM_FECS43_EVENT_ERROR,
} rm_fecs43_event_t;

/** Data type of FECS43 */
typedef enum e_rm_fecs43_data_type
{
    RM_FECS43_HUMIDITY_DATA = 0,
    RM_FECS43_TEMPERATURE_DATA,
} rm_fecs43_data_type_t;

/** FECS43 callback parameter definition */
typedef struct st_rm_fecs43_callback_args
{
    void const      * p_context;
    rm_fecs43_event_t event;
} rm_fecs43_callback_args_t;

/** FECS43 data */
typedef struct st_rm_fecs43_data
{
    float temperature;
    float humidity;
    float gas;
} rm_fecs43_data_t;

/** FECS43 Configuration */
typedef struct st_rm_fecs43_cfg
{
    rm_comms_instance_t const * p_instance;                  ///< Pointer to Communications Middleware instance.
    void const                * p_context;                   ///< Pointer to the user-provided context.
    void const                * p_extend;                    ///< Pointer to extended configuration by instance of interface.
    void (* p_callback)(rm_fecs43_callback_args_t * p_args); ///< Pointer to callback function.
} rm_fecs43_cfg_t;

/** FECS43 control block.  Allocate an instance specific control block to pass into the FECS43 API calls.
 */
typedef void rm_fecs43_ctrl_t;

/** FECS43 APIs */
typedef struct st_rm_fecs43_api
{
    /** Open sensor.
     *
     * @param[in]  p_ctrl       Pointer to control structure.
     * @param[in]  p_cfg        Pointer to configuration structure.
     */
    fsp_err_t (* open)(rm_fecs43_ctrl_t * const p_ctrl, rm_fecs43_cfg_t const * const p_cfg);

    /** Request data from FECS43.
     *
     * @param[in]  p_ctrl       Pointer to control structure.
     */
    fsp_err_t (* requestData)(rm_fecs43_ctrl_t * const p_ctrl);

    /** Read data from FECS43.
     *
     * @param[in]  p_ctrl       Pointer to control structure.
     * @param[in]  p_data       Pointer to data structure.
     */
    fsp_err_t (* read)(rm_fecs43_ctrl_t * const p_ctrl, rm_fecs43_data_t * const p_data);

    /** Close FECS43.
     *
     * @param[in]  p_ctrl       Pointer to control structure.
     */
    fsp_err_t (* close)(rm_fecs43_ctrl_t * const p_ctrl);
} rm_fecs43_api_t;

/** FECS43 instance */
typedef struct st_rm_fecs43_instance
{
    rm_fecs43_ctrl_t      * p_ctrl;    /**< Pointer to the control structure for this instance */
    rm_fecs43_cfg_t const * p_cfg;     /**< Pointer to the configuration structure for this instance */
    rm_fecs43_api_t const * p_api;     /**< Pointer to the API structure for this instance */
} rm_fecs43_instance_t;

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

#endif                                 /* RM_FECS43_API_H_*/

/*******************************************************************************************************************//**
 * @} (end defgroup RM_FECS43_API)
 **********************************************************************************************************************/
