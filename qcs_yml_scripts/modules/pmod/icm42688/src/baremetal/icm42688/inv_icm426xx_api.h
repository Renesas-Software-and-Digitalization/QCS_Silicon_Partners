/*
 * ________________________________________________________________________________________________________
 * Copyright (c) 2017 InvenSense Inc. All rights reserved.
 *
 * This software, related documentation and any modifications thereto (collectively "Software") is subject
 * to InvenSense and its licensors' intellectual property rights under U.S. and international copyright
 * and other intellectual property rights laws.
 *
 * InvenSense and its licensors retain all intellectual property and proprietary rights in and to the Software
 * and any use, reproduction, disclosure or distribution of the Software without an express license agreement
 * from InvenSense is strictly prohibited.
 *
 * EXCEPT AS OTHERWISE PROVIDED IN A LICENSE AGREEMENT BETWEEN THE PARTIES, THE SOFTWARE IS
 * PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
 * TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * EXCEPT AS OTHERWISE PROVIDED IN A LICENSE AGREEMENT BETWEEN THE PARTIES, IN NO EVENT SHALL
 * INVENSENSE BE LIABLE FOR ANY DIRECT, SPECIAL, INDIRECT, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, OR ANY
 * DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THE SOFTWARE.
 * ________________________________________________________________________________________________________
 */

/*******************************************************************************************************************//**
 * @ingroup RENESAS_INTERFACES
 * @defgroup RM_ICM426XX_API ICM426XX Middleware Interface
 * @brief Interface for ICM426XX Middleware functions.
 *
 * @section RM_ICM426XX_API_Summary Summary
 * The ICM426XX interface provides ICM426XX functionality.
 *
 * The ICM426XX interface can be implemented by:
 * - @ref RM_ICM426XX
 *
 * @{
 **********************************************************************************************************************/

#ifndef RM_ICM426XX_API_H_
#define RM_ICM426XX_API_H_

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

#define I2C_USE_SCI           1

#if defined(__CCRX__) || defined(__ICCRX__) || defined(__RX__)
#elif defined(__CCRL__) || defined(__ICCRL__) || defined(__RL78__)
#else

/* Common macro for FSP header files. There is also a corresponding FSP_FOOTER macro at the end of this file. */
FSP_HEADER
#endif

/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#ifndef DATA_FORMAT_DPS_G
#define DATA_FORMAT_DPS_G                    1
#endif 

#define MAX_RECV_PACKET                      128
/*******************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/** Event in the callback function */
typedef enum e_rm_icm426xx_event
{
    RM_ICM426XX_EVENT_SUCCESS = 0,
    RM_ICM426XX_EVENT_ERROR,
} rm_icm426xx_event_t;

/** ICM426XX callback parameter definition */
typedef struct st_rm_icm426xx_callback_args
{
    void const      * p_context;
    rm_icm426xx_event_t event;
} rm_icm426xx_callback_args_t;

struct SensorData {
#if DATA_FORMAT_DPS_G
    float x, y, z;
#else
    int32_t x, y, z;
#endif
    uint64_t timeStamp;
};

#if DATA_FORMAT_DPS_G
typedef float chip_temperature;
#else
typedef int16_t chip_temperature;
#endif

typedef struct {
    uint8_t accDataSize;
    uint64_t timeStamp;
    struct SensorData databuff[MAX_RECV_PACKET];
}AccDataPacket;

typedef struct {
    uint8_t gyroDataSize;
    uint64_t timeStamp;
    struct SensorData databuff[MAX_RECV_PACKET];
}GyroDataPacket;

/** ICM426XX Configuration */
typedef struct st_rm_icm426xx_cfg
{
    rm_comms_instance_t const * p_instance;                  ///< Pointer to Communications Middleware instance.
    void const                * p_context;                   ///< Pointer to the user-provided context.
    void const                * p_extend;                    ///< Pointer to extended configuration by instance of interface.
    void (* p_callback)(rm_icm426xx_callback_args_t * p_args); ///< Pointer to callback function.
} rm_icm426xx_cfg_t;

/** ICM426XX control block.  Allocate an instance specific control block to pass into the ICM426XX API calls.
 * @par Implemented as
 * - rm_icm426xx_instance_ctrl_t
 */
typedef void rm_icm426xx_ctrl_t;

/** ICM426XX APIs */
typedef struct st_rm_icm426xx_api
{
    /** Open sensor.
     * @par Implemented as
     * - @ref RM_ICM426XX_Open()
     *
     * @param[in]  p_ctrl       Pointer to control structure.
     * @param[in]  p_cfg        Pointer to configuration structure.
     */
    fsp_err_t (* open)(rm_icm426xx_ctrl_t * const p_ctrl, rm_icm426xx_cfg_t const * const p_cfg);

    /** Get the sensor ID.
     * @par Implemented as
     * - @ref RM_ICM426XX_SensorIdGet()
     *
     * @param[in]  p_ctrl           Pointer to control structure.
     * @param[in]  p_sensor_id      Pointer to sensor ID of ICM426XX.
     */
    fsp_err_t (* sensorIdGet)(rm_icm426xx_ctrl_t * const p_ctrl, uint32_t *p_sensor_id);
    fsp_err_t (* sensorInit)(rm_icm426xx_ctrl_t * const p_ctrl, bool *init_flag);
    fsp_err_t (* accEnable)(rm_icm426xx_ctrl_t * const p_ctrl, bool enable);
    fsp_err_t (* gyroEnable)(rm_icm426xx_ctrl_t * const p_ctrl, bool enable);
    fsp_err_t (* acc_Setodr)(rm_icm426xx_ctrl_t * const p_api_ctrl, float odr_hz, uint16_t watermark);
    fsp_err_t (* gyro_Setodr)(rm_icm426xx_ctrl_t * const p_api_ctrl, float odr_hz, uint16_t watermark);
    //fsp_err_t (* register_i2c_evetnt) (rm_icm426xx_ctrl_t * const p_api_ctrl,volatile i2c_master_event_t *p);
    fsp_err_t (* register_i2c_evetnt) (rm_icm426xx_ctrl_t * const p_api_ctrl,volatile bool *p);
    fsp_err_t (* get_sensordata)(rm_icm426xx_ctrl_t * const p_api_ctrl,AccDataPacket *accDatabuff,
                                            GyroDataPacket *gyroDatabuff,chip_temperature*chip_temper);
    fsp_err_t (* close)(rm_icm426xx_ctrl_t * const p_ctrl);
} rm_icm426xx_api_t;

/** ICM426XX instance */
typedef struct st_rm_icm426xx_instance
{
    rm_icm426xx_ctrl_t      * p_ctrl;    /**< Pointer to the control structure for this instance */
    rm_icm426xx_cfg_t const * p_cfg;     /**< Pointer to the configuration structure for this instance */
    rm_icm426xx_api_t const * p_api;     /**< Pointer to the API structure for this instance */
} rm_icm426xx_instance_t;

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

#endif                                 /* RM_ICM426XX_API_H_*/

/*******************************************************************************************************************//**
 * @} (end addtogroup RM_ICM426XX_API)
 **********************************************************************************************************************/
