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
 * @addtogroup RM_ICM426XX
 * @{
 **********************************************************************************************************************/

#ifndef RM_ICM426XX_H
#define RM_ICM426XX_H

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "inv_icm426xx_api.h"
#include <stdint.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include "stdio.h"
#include <string.h>

#include "r_i2c_master_api.h"
#include "rm_comms_api.h"
#include "bsp_api.h"

#include "hal_data.h"

/**********************************************************************************************************************
 * Macro definitions
 * 
 * NUM_OF_SENSOR
 **********************************************************************************************************************/
#define MAXNUM_OF_SENSOR 5
/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

#define ICM42688_CFG_DEVICE_TYPE

/** ICM426XX Control Block */
typedef struct rm_icm426xx_instance_ctrl
{
    uint32_t                            open;                 ///< Open flag
    rm_icm426xx_cfg_t const             * p_cfg;                ///< Pointer to ICM426XX Configuration
    rm_comms_instance_t const         * p_comms_i2c_instance; ///< Pointer of I2C Communications Middleware instance structure
    //i2c_master_instance_t const         * p_i2c_master_instance;
    void const                          * p_context;            ///< Pointer to the user-provided context
    //volatile i2c_master_event_t         * p_g_master_event;      // point to g_master_event in i2c call back
    volatile bool                        * p_g_master_event;                 // point to g_master_event in i2c call back
    uint8_t buf[3];                                           ///< Buffer for I2C communications
    float acc_require_odr;                                           // accel odr 
    float gyro_require_odr;                                          // gyro odr
    float acc_report_odr;                                            // accel report odr 
    float gyro_report_odr;                                           // gyro report odr
    bool sensor[MAXNUM_OF_SENSOR];                                   // record sensor enable status
    /* Pointer to callback and optional working memory */
    void (* p_callback)(rm_icm426xx_callback_args_t * p_args);
} rm_icm426xx_instance_ctrl_t;

/**********************************************************************************************************************
 * Exported global variables
 **********************************************************************************************************************/

/** @cond INC_HEADER_DEFS_SEC */
/** Filled in Interface API structure for this Instance. */
extern rm_icm426xx_api_t const g_icm426xx_on_icm426xx;


/** @endcond */

/**********************************************************************************************************************
 * Public Function Prototypes
 **********************************************************************************************************************/
fsp_err_t RM_ICM426XX_Open(rm_icm426xx_ctrl_t * const p_api_ctrl, rm_icm426xx_cfg_t const * const p_cfg);
fsp_err_t RM_ICM426XX_Close(rm_icm426xx_ctrl_t * const p_api_ctrl);

fsp_err_t RM_ICM426XX_SensorIdGet(rm_icm426xx_ctrl_t * const p_api_ctrl, uint32_t * const p_sensor_id);
fsp_err_t RM_ICM426XX_Register_I2C_event (rm_icm426xx_ctrl_t * const p_api_ctrl,volatile bool *p);
fsp_err_t RM_ICM426XX_Sensor_Init (rm_icm426xx_ctrl_t * const p_api_ctrl, bool *init_flag);
fsp_err_t RM_ICM426XX_Acc_Enable(rm_icm426xx_ctrl_t * const p_api_ctrl, bool enable);
fsp_err_t RM_ICM426XX_Gyro_Enable(rm_icm426xx_ctrl_t * const p_api_ctrl, bool enable);
fsp_err_t RM_ICM426XX_Acc_SetRate(rm_icm426xx_ctrl_t * const p_api_ctrl, float odr_hz, uint16_t watermark);
fsp_err_t RM_ICM426XX_Gyro_SetRate(rm_icm426xx_ctrl_t * const p_api_ctrl, float odr_hz, uint16_t watermark);
fsp_err_t RM_ICM426XX_Sensor_GetData(rm_icm426xx_ctrl_t * const p_api_ctrl,AccDataPacket *accDatabuff,
                                            GyroDataPacket *gyroDatabuff,chip_temperature*chip_temper);

#if defined(__CCRX__) || defined(__ICCRX__) || defined(__RX__)
#elif defined(__CCRL__) || defined(__ICCRL__) || defined(__RL78__)
#else

/* Common macro for FSP header files. There is also a corresponding FSP_FOOTER macro at the end of this file. */
FSP_FOOTER
#endif

/**********************************************************************************************************************
 * Following definition and function predefinition is for hardware driver internal usage, should not be called 
 * in application layer driver.
 * 
 * ********************************************************************************************************************/

//#ifdef __cplusplus
//extern "C" {
//#endif // __cplusplus

#define FIFO_WM_MODE_EN                      1 //1:fifo mode 0:dri mode
#define IS_HIGH_RES_MODE 					 0 // 1: FIFO high resolution mode (20bits data format); 0: 16 bits data format;
#define SPI_MODE_EN                          0 //1:spi 0:i2c
#define INT_LATCH_EN                         0 //1:latch mode 0:pulse mode
#define INT_ACTIVE_HIGH_EN                   1 //1:active high 0:active low
#define SUPPORT_SELFTEST                     0 // Please define ICM_42686 for ICM-42686-P chip so that self test can pass.
#define DATA_FORMAT_DPS_G                    1 // 1: Output data in LSB format; 0: Output in dps/g/depgree.

// 1: Support data polling mode from host;
// 0: Do not support polling mode; Interrupt will be used for data reporting, either DRI or FIFO.
#define POLLING_MODE_SUPPORT  				 0
#define SENSOR_TIMESTAMP_SUPPORT      		 1  // true : support timestamp from sensor. false : disable
#define SUPPORT_TIMESTAMP_16US               1  // true: timestamp in 16us; false: timestamp in 1us.
/* APEX Features */
#define SUPPORT_PEDOMETER                    0
#define SUPPORT_WOM                          0

#define SUPPORT_DELAY_US                     1  // 1: platform supports delay in us; 0: platform does not support delay in ms.

#define SENSOR_DIRECTION                     0  // 0~7 sensorConvert map index below
/* { { 1, 1, 1},    {0, 1, 2} },
   { { -1, 1, 1},   {1, 0, 2} },
   { { -1, -1, 1},  {0, 1, 2} },
   { { 1, -1, 1},   {1, 0, 2} },
   { { -1, 1, -1},  {0, 1, 2} },
   { { 1, 1, -1},   {1, 0, 2} },
   { { 1, -1, -1},  {0, 1, 2} },
   { { -1, -1, -1}, {1, 0, 2} }, */
#define SENSOR_LOG_LEVEL                     3 //INV_LOG_LEVEL_INFO
#define SENSOR_REG_DUMP                      0
// For Yokohama the max FIFO size is 2048 Bytes; common FIFO package is 16 Bytes; so the max number of FIFO pakage is ~128.
#define RESET_VALUE 0x00
#define MAX_RECV_PACKET                      128

#define BIT_ACC_LN_MODE                      0x03
#define BIT_ACC_LP_MODE                      0x02
#define BIT_ACC_OFF                          0x00

#define G                             		 9.80665
#define PI                            		 3.141592
#define TEMP_SENSITIVITY_1_BYTE		  		 0.4831f    // Temperature in Degrees Centigrade = (FIFO_TEMP_DATA / 2.07) + 25; for FIFO 1 byte temperature data conversion.
#define TEMP_SENSITIVITY_2_BYTE		  		 0.007548f  // Temperature in Degrees Centigrade = (TEMP_DATA / 132.48) + 25; for data register and FIFO high res. data conversion.
#define ROOM_TEMP_OFFSET              		 25         //Celsius degree

#if	IS_HIGH_RES_MODE
	#if ICM_42686
		#define KSCALE_ACC_FSR				  0.000598550f	// ACC_FSR * G / (1048576.0f/2); This value corresponds to ACCEL_RANGE_32G.
		#define KSCALE_GYRO_FSR 			  0.000133158f	// GYR_FSR * PI / (180.0f * (1048576.0f/2));  4000dps;
	#else
		#define KSCALE_ACC_FSR				  0.000299275f	// ACC_FSR * G / (1048576.0f/2); This value corresponds to ACCEL_RANGE_16G.
		#define KSCALE_GYRO_FSR 			  0.000066579f	// GYR_FSR * PI / (180.0f * (1048576.0f/2));  The value corresponds to 2000dps GYR_FSR;
	#endif
#else
	#if ICM_42686
		#define KSCALE_ACC_FSR				  0.009576807f	// ACC_FSR * G / (65536.0f/2); This value corresponds to ACCEL_RANGE_32G.
		#define KSCALE_GYRO_FSR 			  0.002130528f	// GYR_FSR * PI / (180.0f * (65536.0f/2));	4000dps;
	#else
		#define KSCALE_ACC_FSR           	  0.002394202f  // ACC_FSR * G / (65536.0f/2); This value corresponds to ACCEL_RANGE_8G.
		#define KSCALE_GYRO_FSR        		  0.001065264f  // GYR_FSR * PI / (180.0f * (65536.0f/2));  2000dps.
	#endif
#endif  // IS_HIGH_RES_MODE



enum inv_log_level {
    INV_LOG_LEVEL_OFF     = 0,
    INV_LOG_LEVEL_ERROR,
    INV_LOG_LEVEL_WARNING,
    INV_LOG_LEVEL_INFO,
    INV_LOG_LEVEL_MAX
};

#if 1
/* customer board, need invoke system marco*/
#define INV_LOG(loglevel,fmt,arg...)   printf("ICM4x6xx: "fmt"\r\n",##arg)
#else
/* smart motion board*/
#include "utils/Message.h"
#define INV_LOG           INV_MSG
#endif

typedef enum {
    ACC = 0,
    GYR,
    TEMP,
    #if SENSOR_TIMESTAMP_SUPPORT
    TS,
    #endif
    #if SUPPORT_WOM
    WOM,
    #endif
    #if SUPPORT_PEDOMETER
    PEDO,
    #endif
    NUM_OF_SENSOR,
} SensorType_t;

struct accGyroData {
    uint8_t sensType;
    int16_t x, y, z;
    int16_t temperature;
    uint8_t high_res[3];
    uint64_t timeStamp;
};

struct accGyroDataPacket {
    uint8_t accOutSize;
    uint8_t gyroOutSize;
    uint64_t timeStamp;
    float temperature;
    struct accGyroData outBuf[MAX_RECV_PACKET*2];
    uint32_t magicNum;
};

/**********************************************************************************************************************
 * Private Function Prototypes, should not called outside.
 **********************************************************************************************************************/
int inv_icm4x6xx_initialize(void);
void inv_icm4x6xx_set_serif(int (*read)(void *, uint8_t, uint8_t *, uint32_t),
                            int (*write)(void *, uint8_t,const uint8_t  *, uint32_t));
void inv_icm4x6xx_set_delay(void (*delay_ms)(uint16_t), void (*delay_us)(uint16_t));
int inv_icm4x6xx_acc_enable();
int inv_icm4x6xx_gyro_enable();

int inv_icm4x6xx_acc_disable();
int inv_icm4x6xx_gyro_disable();

int inv_icm4x6xx_acc_set_rate(float odr_hz, uint16_t watermark,float *hw_odr);
int inv_icm4x6xx_gyro_set_rate(float odr_hz, uint16_t watermark,float *hw_odr);

int inv_icm4x6xx_get_rawdata_interrupt(struct accGyroDataPacket *dataPacket);

/* more interface function for deep usage */
int inv_icm4x6xx_config_fifo_int(bool enable);

int inv_icm4x6xx_config_fifofull_int(bool enable);

int inv_icm4x6xx_config_drdy(bool enable);

int inv_icm4x6xx_set_acc_power_mode(uint8_t mode);
void inv_apply_mounting_matrix(int32_t raw[3]);

#if POLLING_MODE_SUPPORT
int inv_icm4x6xx_polling_rawdata(struct accGyroDataPacket *dataPacket);
#endif

#if SENSOR_TIMESTAMP_SUPPORT
int inv_icm4x6xx_get_sensor_timestamp(uint64_t *timestamp);
#endif


#if SUPPORT_SELFTEST
int inv_icm4x6xx_acc_selftest(bool *result);
int inv_icm4x6xx_gyro_selftest(bool *result);
#endif

#if SUPPORT_PEDOMETER
int inv_icm4x6xx_pedometer_enable();
int inv_icm4x6xx_pedometer_disable();
int inv_icm4x6xx_pedometer_get_stepCnt(uint64_t *step_cnt);
#endif

#if SUPPORT_WOM
int inv_icm4x6xx_wom_enable();
int inv_icm4x6xx_wom_disable();
int inv_icm4x6xx_wom_get_event(bool *detect);
#endif

#if SENSOR_REG_DUMP
void inv_icm4x6xx_dumpRegs();
#endif


#endif                                 /* RM_ICM426XX_H_*/

/*******************************************************************************************************************//**
 * @} (end addtogroup RM_ICM426XX)
 **********************************************************************************************************************/
