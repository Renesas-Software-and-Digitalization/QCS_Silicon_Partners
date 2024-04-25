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

/**********************************************************************************************************************
 * Includes   <System Includes> , "Project Includes"
 *********************************************************************************************************************/
#include "inv_icm426xx.h"

#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>

/**********************************************************************************************************************
 * Macro definitions
 *********************************************************************************************************************/

#define GYRO_STALL_PATCH_ENABLE       1

#define NUM_TODISCARD                 0
#define SELFTEST_SAMPLES_NUMBER       200
#define ICM4X6XX_MAX_FIFO_SIZE        2000  // Max FIFO count size

#define max(x, y)                     (x > y ? x : y)
#define ARRAY_SIZE(a)                 (sizeof((a)) / sizeof((a)[0]))

#ifndef abs
#define abs(a)                        ((a) > 0 ? (a) : -(a)) /*!< Absolute value */
#endif

/* ICM4X6XX register BANK */
/* bank 0 */
#define REG_CHIP_CONFIG             0x11
#define BIT_SOFT_RESET_CONFIG       0x01
#define REG_DRIVE_CONFIG            0x13
#define BIT_SPI_SLEW_RATE_MASK      0x07
#define BIT_I2C_SLEW_RATE_MASK      0x38
#define BIT_I2C_SLEW_RATE_SHIFT     3
#define REG_INT_CONFIG              0x14
#define BIT_INT1_POLARITY_MASK      0x01
#define BIT_INT1_ACTIVE_HIGH        0x01
#define BIT_INT1_DRIVE_CIRCUIT_MASK 0x02
#define BIT_INT1_PUSH_PULL          0x02
#define BIT_INT1_MODE_MASK          0x04
#define BIT_INT1_LATCHED_MODE       0x04
#define BIT_INT2_POLARITY_MASK      0x08
#define BIT_INT2_ACTIVE_HIGH        0x08
#define BIT_INT2_DRIVE_CIRCUIT_MASK 0x10
#define BIT_INT2_PUSH_PULL          0x10
#define BIT_INT2_MODE_MASK          0x20
#define BIT_INT2_LATCHED_MODE       0x20
#define REG_FIFO_CONFIG             0x16
#define BIT_FIFO_MODE_SHIFT         6
#define BIT_FIFO_MODE_CTRL_MASK     (3 << BIT_FIFO_MODE_SHIFT)
#define BIT_FIFO_MODE_CTRL_BYPASS   (0 << BIT_FIFO_MODE_SHIFT)
#define BIT_FIFO_MODE_CTRL_STREAM   (1 << BIT_FIFO_MODE_SHIFT)
#define BIT_FIFO_MODE_CTRL_SNAPSHOT (2 << BIT_FIFO_MODE_SHIFT)
#define REG_TEMP_DATA1              0x1D
#define REG_ACCEL_DATA_X1           0x1F
#define REG_GYRO_DATA_X1            0x25
#define REG_INT_STATUS              0x2D
#define BIT_INT_FIFO_FULL           0x02
#define BIT_INT_FIFO_WM             0x04
#define BIT_INT_DRDY                0x08
#define BIT_RESET_DONE              0x10
#define REG_FIFO_COUNT_H            0x2E
#define REG_FIFO_DATA               0x30
#define REG_APEX_DATA0              0x31
#define REG_APEX_DATA2              0x33
#define REG_APEX_DATA3              0x34
#define BIT_DMP_IDLE                0x04
#define BIT_ACTIVITY_CLASS_MASK     0x03
#define BIT_ACTIVITY_CLASS_UNKNOWN  0x00
#define BIT_ACTIVITY_CLASS_WALK     0x01
#define BIT_ACTIVITY_CLASS_RUN      0x02
#define REG_INT_STATUS2             0x37
#define BIT_SMD_INT                 0x08
#define BIT_INT_WOM_MASK            0x07
#define BIT_INT_WOM_Z               0x04
#define BIT_INT_WOM_Y               0x02
#define BIT_INT_WOM_X               0x01
#define REG_INT_STATUS3             0x38
#define BIT_INT_STEP_DET            0x20
#define BIT_INT_STEP_CNT_OVF        0x10
#define BIT_INT_TILT_DET            0x08
#define BIT_INT_WAKE_DET            0x04
#define BIT_INT_LOW_G_DET           0x04
#define BIT_INT_SLEEP_DET           0x02
#define BIT_INT_HIGH_G_DET          0x02
#define BIT_INT_TAP_DET             0x01
#define REG_SIGNAL_PATH_RESET       0x4B
#define BIT_DMP_INIT_EN             0x40
#define BIT_DMP_MEM_RESET_EN        0x20
#define BIT_ABORT_AND_RESET         0x08
#define BITTMST_STROBE_EN			0x04
#define BIT_FIFO_FLUSH              0x02
#define REG_INTF_CONFIG0            0x4C
#define BIT_FIFO_HOLD_LAST_DATA_EN  0x80
#define BIT_FIFO_COUNT_REC_MASK     0x40
#define BIT_FIFO_COUNT_RECORD_MODE  0x40
#define BIT_FIFO_COUNT_ENDIAN_MASK  0x20
#define BIT_FIFO_COUNT_BIG_ENDIAN   0x20
#define BIT_SENSOR_DATA_ENDIAN_MASK 0x10
#define BIT_SENSOR_DATA_BIG_ENDIAN  0x10
#define BIT_UI_SIFS_CFG_MASK        0x03
#define BIT_UI_SIFS_CFG_I2C_DIS     0x03
#define BIT_UI_SIFS_CFG_SPI_DIS     0x02
#define REG_INTF_CONFIG1            0x4D
#define BIT_ACCEL_LP_CLK_SEL_MASK   0x08
#define BIT_ACCEL_LP_CLK_SEL_RC     0x08
#define BIT_CLKSEL_MASK             0x03
#define BIT_CLKSEL_RC               0x00
#define BIT_CLKSEL_PLL              0x01
#define BIT_CLKSEL_DIS              0x03
#define REG_PWR_MGMT0               0x4E
#define BIT_TEMP_DIS                0x20
#define BIT_IDLE_MASK               0x10
#define BIT_GYRO_MODE_MASK          0x0C
#define BIT_GYRO_MODE_STANDBY       0x04
#define BIT_GYRO_MODE_LN            0x0C
#define BIT_ACCEL_MODE_MASK         0x03
#define BIT_ACCEL_MODE_LP           0x02
#define BIT_ACCEL_MODE_LN           0x03
#define REG_GYRO_CONFIG0            0x4F
#define BIT_GYRO_FSR_MASK           0xE0
#define BIT_GYRO_FSR_SHIFT          5
#define BIT_GYRO_ODR_MASK           0x0F
#define REG_ACCEL_CONFIG0           0x50
#define BIT_ACCEL_FSR_MASK          0xE0
#define BIT_ACCEL_FSR_SHIFT         5
#define BIT_ACCEL_ODR_MASK          0x0F
#define REG_GYRO_CONFIG1            0x51
#define BIT_GYRO_AVG_FILT_8K_HZ     0x10
#define BIT_GYRO_FILT_ORD_SHIFT     0x02
#define BIT_GYRO_FILT_ORD_MASK      0x0C
#define REG_GYRO_ACCEL_CONFIG0      0x52
#define BIT_ACCEL_FILT_BW_MASK      0xF0
#define BIT_ACCEL_FILT_BW_SHIFT     4
#define BIT_GYRO_FILT_BW_MASK       0x0F
#define REG_ACCEL_CONFIG1           0x53
#define BIT_ACCEL_AVG_FILT_8K_HZ    0x01
#define BIT_ACCEL_FILT_ORD_SHIFT    0x03
#define BIT_ACCEL_FILT_ORD_MASK     0x18
#define REG_TMST_CONFIG				0x54
#define BIT_TMST_TO_REGS_EN   	    0x10
#define BIT_TMST_RES_16US   		0x08
#define BIT_TMST_DELTA_EN			0x04
#define BIT_TMST_FSYNC_EN			0x02
#define BIT_TMST_EN   			    0x01
#define REG_APEX_CONFIG0            0x56
#define BIT_DMP_POWER_SAVE_EN       0x80
#define BIT_PED_ENABLE_EN           0x20
#define BIT_R2W_EN_EN               0x08
#define BIT_LOW_G_EN                0x08
#define BIT_HIGH_G_EN               0x04
#define BIT_DMP_ODR_MASK            0x03
#define BIT_DMP_ODR_25HZ            0x00
#define BIT_DMP_ODR_50HZ            0x02
#define BIT_DMP_ODR_100HZ           0x03
#define REG_SMD_CONFIG              0x57
#define BIT_WOM_INT_MODE_MASK       0x08
#define BIT_WOM_INT_MODE_AND        0x08
#define BIT_WOM_MODE_MASK           0x04
#define BIT_WOM_MODE_COMPARE_PRE    0x04
#define BIT_SMD_MODE_MASK           0x03
#define BIT_SMD_MODE_WOM            0x01
#define BIT_SMD_MODE_SHORT          0x02
#define BIT_SMD_MODE_LONG           0x03
#define REG_FIFO_CONFIG_1           0x5F
#define BIT_FIFO_RESUME_PARTIAL_RD_MASK 0x40
#define BIT_FIFO_WM_GT_TH_MASK          0x20
#define BIT_FIFO_HIRES_EN_MASK          0x10
#define BIT_FIFO_HIRES_EN	            0x10
#define BIT_FIFO_TMST_FSYNC_EN_MASK     0x08
#define BIT_FIFO_TEMP_EN_MASK           0x04
#define BIT_FIFO_TEMP_EN_EN             0x04
#define BIT_FIFO_GYRO_EN_MASK           0x02
#define BIT_FIFO_GYRO_EN_EN             0x02
#define BIT_FIFO_ACCEL_EN_MASK          0x01
#define BIT_FIFO_ACCEL_EN_EN            0x01
#define REG_FIFO_WM_TH_L            0x60
#define REG_FIFO_WM_TH_H            0x61
#define REG_INT_CONFIG0             0x63
#define BIT_INT_FIFO_THS_CLR_MASK   0x0C
#define BIT_INT_FIFO_THS_CLR_SHIFT  2
#define BIT_INT_FIFO_FULL_CLR_MASK  0x03
#define REG_INT_CONFIG1             0x64
#define BIT_INT_ASY_RST_DIS         0x00
#define REG_INT_SOURCE0             0x65
#define BIT_INT1_DRDY_EN            0x08
#define BIT_INT1_WM_EN              0x04
#define BIT_INT1_FIFO_FULL_EN       0x02
#define REG_INT_SOURCE1             0x66
#define BIT_INT1_WOM_EN_MASK        0x07
#define BIT_INT1_SMD_EN             0x08
#define REG_INT_SOURCE3             0x68
#define BIT_INT2_DRDY_EN            0x08
#define BIT_INT2_WM_EN              0x04
#define BIT_INT2_FIFO_FULL_EN       0x02
#define REG_INT_SOURCE4             0x69
#define BIT_INT2_WOM_EN_MASK        0x07
#define BIT_INT2_SMD_EN             0x08
#define REG_SELF_TEST_CONFIG        0x70
#define BIT_ACCEL_ST_POWER_EN       0x40
#define BIT_ACCEL_ST_EN_MASK        0x38
#define BIT_ACCEL_Z_ST_EN           0x20
#define BIT_ACCEL_Y_ST_EN           0x10
#define BIT_ACCEL_X_ST_EN           0x08
#define BIT_GYRO_ST_EN_MASK         0x07
#define BIT_GYRO_Z_ST_EN            0x04
#define BIT_GYRO_Y_ST_EN            0x02
#define BIT_GYRO_X_ST_EN            0x01
#define REG_SCAN0                   0x71
#define BIT_DMP_MEM_ACCESS_EN       0x08
#define BIT_MEM_OTP_ACCESS_EN       0x04
#define BIT_FIFO_MEM_RD_SYS         0x02
#define BIT_FIFO_MEM_WR_SER         0x01
#define REG_MEM_BANK_SEL            0x72
#define REG_MEM_START_ADDR          0x73
#define REG_MEM_R_W                 0x74
#define REG_WHO_AM_I                0x75
#define REG_BANK_SEL                0x76

/* Bank 1 */
#define REG_XG_ST_DATA              0x5F
#define REG_YG_ST_DATA              0x60
#define REG_ZG_ST_DATA              0x61

#define REG_TMSTVAL0                0x62
#define REG_TMSTVAL1                0x63
#define REG_TMSTVAL2                0x64
#define REG_INTF_CONFIG4            0x7A
#define BIT_SPI_AP_4WIRE_EN         0x02
#define REG_INTF_CONFIG5            0x7B
#define BIT_PIN9_FUNC_INT2          0x00
#define BIT_PIN9_FUNC_FSYNC         0x02
#define BIT_PIN9_FUNC_CLKIN         0x04
#define BIT_PIN9_FUNC_MASK          0x06
#define REG_INTF_CONFIG6            0x7C
#define BIT_I3C_EN                  0x10
#define BIT_I3C_SDR_EN              0x01
#define BIT_I3C_DDR_EN              0x02

/* Bank 2 */
#define REG_XA_ST_DATA              0x3B
#define REG_YA_ST_DATA              0x3C
#define REG_ZA_ST_DATA              0x3D

/* Bank 4 */
#define REG_APEX_CONFIG1            0x40
#define BIT_LOW_ENERGY_AMP_TH_SEL_MASK   0xF0
#define BIT_LOW_ENERGY_AMP_TH_SEL_SHIFT  4
#define BIT_DMP_POWER_SAVE_TIME_SEL_MASK 0x0F
#define REG_APEX_CONFIG2            	 0x41
#define BIT_PED_AMP_TH_SEL_MASK          0xF0
#define BIT_PED_AMP_TH_SEL_SHIFT         4
#define BIT_PED_STEP_CNT_TH_SEL_MASK     0x0F
#define REG_APEX_CONFIG3            	 0x42
#define BIT_PED_STEP_DET_TH_SEL_MASK     0xE0
#define BIT_PED_STEP_DET_TH_SEL_SHIFT    5
#define BIT_PED_SB_TIMER_TH_SEL_MASK     0x1C
#define BIT_PED_SB_TIMER_TH_SEL_SHIFT    2
#define BIT_PED_HI_EN_TH_SEL_MASK        0x03
#define REG_APEX_CONFIG4            	 0x43
#define BIT_TILT_WAIT_TIME_SEL_MASK      0xC0
#define BIT_TILT_WAIT_TIME_SEL_SHIFT     6
#define BIT_SLEEP_TIME_OUT_MASK          0x38
#define BIT_SLEEP_TIME_OUT_SHIFT         3
#define BIT_LOWG_PEAK_TH_HYST_MASK       0x38
#define BIT_LOWG_PEAK_TH_HYST_SHIFT      3
#define REG_APEX_CONFIG5            	 0x44
#define BIT_LOWG_PEAK_TH_SHIFT           3
#define BIT_LOWG_PEAK_TH_MASK            0xF8
#define BIT_LOWG_TIME_TH_MASK            0x07
#define REG_APEX_CONFIG6            0x45
#define BIT_HIGHG_PEAK_TH_SHIFT     3
#define BIT_HIGHG_PEAK_TH_MASK      0xF8
#define REG_APEX_CONFIG9            0x48
#define BIT_SENSITIVITY_MODE_MASK   0x01
#define REG_ACCEL_WOM_X_THR         0x4A
#define REG_ACCEL_WOM_Y_THR         0x4B
#define REG_ACCEL_WOM_Z_THR         0x4C
#define REG_INT_SOURCE6             0x4D
#define BIT_INT1_STEP_DET_EN        0x20
#define BIT_INT1_STEP_CNT_OFL_EN    0x10
#define BIT_INT1_TILT_DET_EN        0x08
#define BIT_INT1_WAKE_DET_EN        0x04
#define BIT_INT1_LOW_G_DET_EN       0x04
#define BIT_INT1_SLEEP_DET_EN       0x02
#define BIT_INT1_HIGH_G_DET_EN      0x02
#define BIT_INT1_TAP_DET_EN         0x01
#define REG_INT_SOURCE7             0x4E
#define BIT_INT2_STEP_DET_EN        0x20
#define BIT_INT2_STEP_CNT_OFL_EN    0x10
#define BIT_INT2_TILT_DET_EN        0x08
#define BIT_INT2_WAKE_DET_EN        0x04
#define BIT_INT2_LOW_G_DET_EN       0x04
#define BIT_INT2_SLEEP_DET_EN       0x02
#define BIT_INT2_HIGH_G_DET_EN      0x02
#define BIT_INT2_TAP_DET_EN         0x01
#define BIT_INT_STATUS_DRDY         0x08

/* chip id config */
#define ICM40607_WHO_AM_I           0x38
#define ICM42602_WHO_AM_I           0x41
#define ICM42605_WHO_AM_I           0x42
#define ICM42688_WHO_AM_I           0x47
#define ICM42686_WHO_AM_I           0x44
#define ICM40608_WHO_AM_I           0x39
#define ICM42631_WHO_AM_I           0x5C

/* wom config */
/** Default value for the WOM threshold
 *  Resolution of the threshold is 1g/256 ~= 3.9mg
 */
#if SUPPORT_WOM
#define DEFAULT_WOM_THS_MG 60 >> 2
#endif
/* Error List */
#define SENSOR_WHOAMI_INVALID_ERROR     -1
#define SENSOR_CONVERT_INVALID_ERROR    -2
#define SENSOR_INIT_INVALID_ERROR       -3
#define INT_STATUS_READ_ERROR           -4
#define DRDY_UNEXPECT_INT_ERROR         -5
#define DRDY_DATA_READ_ERROR            -6
#define DRDY_DATA_CONVERT_ERROR         -7
#define FIFO_COUNT_INVALID_ERROR        -8
#define FIFO_OVERFLOW_ERROR             -9
#define FIFO_UNEXPECT_WM_ERROR          -10
#define FIFO_DATA_READ_ERROR            -11
#define FIFO_DATA_CONVERT_ERROR         -12  
#define FIFO_DATA_FULL_ERROR            -13
#if SUPPORT_SELFTEST
#define SELFTEST_INIT_ERROR             -14
#define SELFTEST_ACC_ENABLE_ERROR       -15
#define SELFTEST_ACC_READ_ERROR         -16
#define SELFTEST_ACC_ST_READ_ERROR      -17
#define SELFTEST_ACC_STCODE_READ_ERROR  -18
#define SELFTEST_GYR_ENABLE_ERROR       -19
#define SELFTEST_GYR_READ_ERROR         -20
#define SELFTEST_GYR_ST_READ_ERROR      -21
#define SELFTEST_GYR_STCODE_READ_ERROR  -22
#endif
#define DMP_TIMEOUT_ERROR               -23
#define DMP_IDLE_ERROR                  -24
#define DMP_SIZE_ERROR                  -25
#define DMP_SRAM_ERROR                  -26
#define DMP_STATUS_ERROR                -27
#define DMP_LOAD_ERROR                  -28
#if SUPPORT_PEDOMETER
#define PEDO_ENABLE_ERROR               -29
#endif
#if SUPPORT_WOM
#define WOM_ENABLE_ERROR                -32
#endif

#define AXIS_X          0
#define AXIS_Y          1
#define AXIS_Z          2
#define AXES_NUM        3

#define SENSOR_HZ(_hz)  ((uint32_t)((_hz) * 1024.0f))

#define ACCEL_DATA_SIZE               6
#define GYRO_DATA_SIZE                6
#define TEMP_DATA_SIZE                2

#define FIFO_HEADER_SIZE              1
#define FIFO_ACCEL_DATA_SHIFT         1
#define FIFO_ACCEL_DATA_SIZE          ACCEL_DATA_SIZE
#define FIFO_GYRO_DATA_SHIFT          7
#define FIFO_GYRO_DATA_SIZE           GYRO_DATA_SIZE
#define FIFO_TEMP_DATA_SHIFT          13
#define FIFO_TEMP_DATA_SIZE           1
#define FIFO_TIMESTAMP_SIZE           2
#define FIFO_TEMP_HIGH_RES_SIZE       1
#define FIFO_ACCEL_GYRO_HIGH_RES_SIZE 3
#define FIFO_TIMESTAMP_DATA_SHIFT     (icm_dev.fifo_highres_enabled ? 15: 14)

#define FIFO_EMPTY_PACKAGE_SIZE       0
#define FIFO_8BYTES_PACKET_SIZE       (FIFO_HEADER_SIZE + FIFO_ACCEL_DATA_SIZE + FIFO_TEMP_DATA_SIZE)
#define FIFO_16BYTES_PACKET_SIZE      (FIFO_HEADER_SIZE + FIFO_ACCEL_DATA_SIZE + FIFO_GYRO_DATA_SIZE + FIFO_TEMP_DATA_SIZE + FIFO_TIMESTAMP_SIZE)
#define FIFO_20BYTES_PACKET_SIZE      (FIFO_HEADER_SIZE + FIFO_ACCEL_DATA_SIZE + FIFO_GYRO_DATA_SIZE + FIFO_TEMP_DATA_SIZE + FIFO_TIMESTAMP_SIZE +\
                                       FIFO_TEMP_HIGH_RES_SIZE + FIFO_ACCEL_GYRO_HIGH_RES_SIZE)
#define FIFO_HEADER_EMPTY_BIT         0x80
#define FIFO_HEADER_A_BIT             0x40
#define FIFO_HEADER_G_BIT             0x20
#define FIFO_HEADER_20_BIT            0x10

#define DRI_ACCEL_DATA_SHIFT          2
#define DRI_GYRO_DATA_SHIFT           8
#define DRI_14BYTES_PACKET_SIZE       (TEMP_DATA_SIZE + ACCEL_DATA_SIZE + GYRO_DATA_SIZE)

#define ICM4X6XX_ODR_12HZ_ACCEL_STD                    (0)
#define ICM4X6XX_ODR_25HZ_ACCEL_STD                    (1)
#define ICM4X6XX_ODR_50HZ_ACCEL_STD                    (1)
#define ICM4X6XX_ODR_100HZ_ACCEL_STD                   (3)
#define ICM4X6XX_ODR_200HZ_ACCEL_STD                   (6)
#define ICM4X6XX_ODR_500HZ_ACCEL_STD                   (15)
#define ICM4X6XX_ODR_1000HZ_ACCEL_STD                  (30)

#define ICM4X6XX_ODR_12HZ_GYRO_STD                     (2)
#define ICM4X6XX_ODR_25HZ_GYRO_STD                     (3)
#define ICM4X6XX_ODR_50HZ_GYRO_STD                     (5)
#define ICM4X6XX_ODR_100HZ_GYRO_STD                    (10)
#define ICM4X6XX_ODR_200HZ_GYRO_STD                    (20)
#define ICM4X6XX_ODR_500HZ_GYRO_STD                    (50)

/* register configuration for self-test procedure */
#define ST_GYRO_FSR        GYRO_RANGE_250DPS  // When this FSR changed, the equation in inv_icm4x6xx_selftest_gyro_result() also needs to be adjusted.
#define ST_ACCEL_FSR       ACCEL_RANGE_2G     // When this FSR changed, the equation in inv_icm4x6xx_selftest_acc_result() also needs to be adjusted.

/* Formula to get ST_OTP based on FS and ST_code */
#if defined(ICM_42686)
#define INV_ST_OTP_EQUATION(FS, ST_code) (uint32_t)((1310/pow(2,3-FS))*pow(1.01, ST_code-1)+0.5)
#else
#define INV_ST_OTP_EQUATION(FS, ST_code) (uint32_t)((2620/pow(2,3-FS))*pow(1.01, ST_code-1)+0.5)
#endif

/**********************************************************************************************************************
 * Local Typedef definitions
 *********************************************************************************************************************/
#if SUPPORT_PEDOMETER
/* pedo config */
/* LOW_ENERGY_AMP_TH_SEL */
typedef enum
{
    LOW_ENERGY_AMP_TH_SEL_1006632MG = (0 << BIT_LOW_ENERGY_AMP_TH_SEL_SHIFT),
    LOW_ENERGY_AMP_TH_SEL_1174405MG = (1 << BIT_LOW_ENERGY_AMP_TH_SEL_SHIFT),
    LOW_ENERGY_AMP_TH_SEL_1342177MG = (2 << BIT_LOW_ENERGY_AMP_TH_SEL_SHIFT),
    LOW_ENERGY_AMP_TH_SEL_1509949MG = (3 << BIT_LOW_ENERGY_AMP_TH_SEL_SHIFT),
    LOW_ENERGY_AMP_TH_SEL_1677721MG = (4 << BIT_LOW_ENERGY_AMP_TH_SEL_SHIFT),
    LOW_ENERGY_AMP_TH_SEL_1845493MG = (5 << BIT_LOW_ENERGY_AMP_TH_SEL_SHIFT),
    LOW_ENERGY_AMP_TH_SEL_2013265MG = (6 << BIT_LOW_ENERGY_AMP_TH_SEL_SHIFT),
    LOW_ENERGY_AMP_TH_SEL_2181038MG = (7 << BIT_LOW_ENERGY_AMP_TH_SEL_SHIFT),
    LOW_ENERGY_AMP_TH_SEL_2348810MG = (8 << BIT_LOW_ENERGY_AMP_TH_SEL_SHIFT),
    LOW_ENERGY_AMP_TH_SEL_2516582MG = (9 << BIT_LOW_ENERGY_AMP_TH_SEL_SHIFT),
    LOW_ENERGY_AMP_TH_SEL_2684354MG = (10 << BIT_LOW_ENERGY_AMP_TH_SEL_SHIFT),
    LOW_ENERGY_AMP_TH_SEL_2852126MG = (11 << BIT_LOW_ENERGY_AMP_TH_SEL_SHIFT),
    LOW_ENERGY_AMP_TH_SEL_3019898MG = (12 << BIT_LOW_ENERGY_AMP_TH_SEL_SHIFT),
    LOW_ENERGY_AMP_TH_SEL_3187671MG = (13 << BIT_LOW_ENERGY_AMP_TH_SEL_SHIFT),
    LOW_ENERGY_AMP_TH_SEL_3355443MG = (14 << BIT_LOW_ENERGY_AMP_TH_SEL_SHIFT),
    LOW_ENERGY_AMP_TH_SEL_3523215MG = (15 << BIT_LOW_ENERGY_AMP_TH_SEL_SHIFT)
} LOW_ENERGY_AMP_TH_t;


/* DMP_POWER_SAVE_TIME_SEL */
typedef enum
{
    DMP_POWER_SAVE_TIME_SEL_0S  = 0,
    DMP_POWER_SAVE_TIME_SEL_4S  = 1,
    DMP_POWER_SAVE_TIME_SEL_8S  = 2,
    DMP_POWER_SAVE_TIME_SEL_12S = 3,
    DMP_POWER_SAVE_TIME_SEL_16S = 4,
    DMP_POWER_SAVE_TIME_SEL_20S = 5,
    DMP_POWER_SAVE_TIME_SEL_24S = 6,
    DMP_POWER_SAVE_TIME_SEL_28S = 7,
    DMP_POWER_SAVE_TIME_SEL_32S = 8,
    DMP_POWER_SAVE_TIME_SEL_36S = 9,
    DMP_POWER_SAVE_TIME_SEL_40S = 10,
    DMP_POWER_SAVE_TIME_SEL_44S = 11,
    DMP_POWER_SAVE_TIME_SEL_48S = 12,
    DMP_POWER_SAVE_TIME_SEL_52S = 13,
    DMP_POWER_SAVE_TIME_SEL_56S = 14,
    DMP_POWER_SAVE_TIME_SEL_60S = 15
} DMP_POWER_SAVE_TIME_t;

/* PEDO_AMP_TH_SEL */
typedef enum
{
    PEDO_AMP_TH_1006632_MG = (0  << BIT_PED_AMP_TH_SEL_SHIFT),
    PEDO_AMP_TH_1140850_MG = (1  << BIT_PED_AMP_TH_SEL_SHIFT),
    PEDO_AMP_TH_1275068_MG = (2  << BIT_PED_AMP_TH_SEL_SHIFT),
    PEDO_AMP_TH_1409286_MG = (3  << BIT_PED_AMP_TH_SEL_SHIFT),
    PEDO_AMP_TH_1543503_MG = (4  << BIT_PED_AMP_TH_SEL_SHIFT),
    PEDO_AMP_TH_1677721_MG = (5  << BIT_PED_AMP_TH_SEL_SHIFT),
    PEDO_AMP_TH_1811939_MG = (6  << BIT_PED_AMP_TH_SEL_SHIFT),
    PEDO_AMP_TH_1946157_MG = (7  << BIT_PED_AMP_TH_SEL_SHIFT),
    PEDO_AMP_TH_2080374_MG = (8  << BIT_PED_AMP_TH_SEL_SHIFT),
    PEDO_AMP_TH_2214592_MG = (9  << BIT_PED_AMP_TH_SEL_SHIFT),
    PEDO_AMP_TH_2348810_MG = (10 << BIT_PED_AMP_TH_SEL_SHIFT),
    PEDO_AMP_TH_2483027_MG = (11 << BIT_PED_AMP_TH_SEL_SHIFT),
    PEDO_AMP_TH_2617245_MG = (12 << BIT_PED_AMP_TH_SEL_SHIFT),
    PEDO_AMP_TH_2751463_MG = (13 << BIT_PED_AMP_TH_SEL_SHIFT),
    PEDO_AMP_TH_2885681_MG = (14 << BIT_PED_AMP_TH_SEL_SHIFT),
    PEDO_AMP_TH_3019898_MG = (15 << BIT_PED_AMP_TH_SEL_SHIFT)
} PEDO_AMP_TH_t;

/* PED_STEP_CNT_TH_SEL */
typedef enum
{
    PED_STEP_CNT_TH_0_STEP = 0,
    PED_STEP_CNT_TH_1_STEP = 1,
    PED_STEP_CNT_TH_2_STEP = 2,
    PED_STEP_CNT_TH_3_STEP = 3,
    PED_STEP_CNT_TH_4_STEP = 4,
    PED_STEP_CNT_TH_5_STEP = 5,
    PED_STEP_CNT_TH_6_STEP = 6,
    PED_STEP_CNT_TH_7_STEP = 7,
    PED_STEP_CNT_TH_8_STEP = 8,
    PED_STEP_CNT_TH_9_STEP = 9,
    PED_STEP_CNT_TH_10_STEP = 10,
    PED_STEP_CNT_TH_11_STEP = 11,
    PED_STEP_CNT_TH_12_STEP = 12,
    PED_STEP_CNT_TH_13_STEP = 13,
    PED_STEP_CNT_TH_14_STEP = 14,
    PED_STEP_CNT_TH_15_STEP = 15,
} PED_STEP_CNT_TH_t;

/* PED_STEP_DET_TH_SEL */
typedef enum
{
    PED_STEP_DET_TH_0_STEP = (0 << BIT_PED_STEP_DET_TH_SEL_SHIFT),
    PED_STEP_DET_TH_1_STEP = (1 << BIT_PED_STEP_DET_TH_SEL_SHIFT),
    PED_STEP_DET_TH_2_STEP = (2 << BIT_PED_STEP_DET_TH_SEL_SHIFT),
    PED_STEP_DET_TH_3_STEP = (3 << BIT_PED_STEP_DET_TH_SEL_SHIFT),
    PED_STEP_DET_TH_4_STEP = (4 << BIT_PED_STEP_DET_TH_SEL_SHIFT),
    PED_STEP_DET_TH_5_STEP = (5 << BIT_PED_STEP_DET_TH_SEL_SHIFT),
    PED_STEP_DET_TH_6_STEP = (6 << BIT_PED_STEP_DET_TH_SEL_SHIFT),
    PED_STEP_DET_TH_7_STEP = (7 << BIT_PED_STEP_DET_TH_SEL_SHIFT),
} PED_STEP_DET_TH_t;

/* PEDO_SB_TIMER_TH_SEL */
typedef enum
{
    PEDO_SB_TIMER_TH_50_SAMPLES  = (0  << BIT_PED_SB_TIMER_TH_SEL_SHIFT),
    PEDO_SB_TIMER_TH_75_SAMPLES  = (1  << BIT_PED_SB_TIMER_TH_SEL_SHIFT),
    PEDO_SB_TIMER_TH_100_SAMPLES = (2  << BIT_PED_SB_TIMER_TH_SEL_SHIFT),
    PEDO_SB_TIMER_TH_125_SAMPLES = (3  << BIT_PED_SB_TIMER_TH_SEL_SHIFT),
    PEDO_SB_TIMER_TH_150_SAMPLES = (4  << BIT_PED_SB_TIMER_TH_SEL_SHIFT),
    PEDO_SB_TIMER_TH_175_SAMPLES = (5  << BIT_PED_SB_TIMER_TH_SEL_SHIFT),
    PEDO_SB_TIMER_TH_200_SAMPLES = (6  << BIT_PED_SB_TIMER_TH_SEL_SHIFT),
    PEDO_SB_TIMER_TH_225_SAMPLES = (7  << BIT_PED_SB_TIMER_TH_SEL_SHIFT)
} PEDO_SB_TIMER_TH_t;

/* PEDO_HI_ENRGY_TH_SEL */
typedef enum

    PEDO_HI_ENRGY_TH_90  = 0,
    PEDO_HI_ENRGY_TH_107 = 1,
    PEDO_HI_ENRGY_TH_136 = 2,
    PEDO_HI_ENRGY_TH_159 = 3
} PEDO_HI_ENRGY_TH_t;
#endif

typedef enum {
    ODR_8KHZ    = 3,
    ODR_4KHZ    = 4,
    ODR_2KHZ    = 5,
    ODR_1KHZ    = 6,
    ODR_200HZ   = 7,
    ODR_100HZ   = 8,
    ODR_50HZ    = 9,
    ODR_25HZ    = 10,
    ODR_12_5HZ  = 11,
    ODR_500HZ   = 15,
} icm4x6xx_sensor_odr_t;

typedef enum {
#if defined(ICM_42686)
	ACCEL_RANGE_32G = (0 << BIT_ACCEL_FSR_SHIFT),
	ACCEL_RANGE_16G = (1 << BIT_ACCEL_FSR_SHIFT),
	ACCEL_RANGE_8G	= (2 << BIT_ACCEL_FSR_SHIFT),
	ACCEL_RANGE_4G	= (3 << BIT_ACCEL_FSR_SHIFT),
	ACCEL_RANGE_2G	= (4 << BIT_ACCEL_FSR_SHIFT),
#else
    ACCEL_RANGE_16G = (0 << BIT_ACCEL_FSR_SHIFT),
    ACCEL_RANGE_8G  = (1 << BIT_ACCEL_FSR_SHIFT),
    ACCEL_RANGE_4G  = (2 << BIT_ACCEL_FSR_SHIFT),
    ACCEL_RANGE_2G  = (3 << BIT_ACCEL_FSR_SHIFT),
#endif
} icm4x6xx_accel_fsr_t;

typedef enum {
#if defined(ICM_42686)
    GYRO_RANGE_4000DPS   = (0 << BIT_GYRO_FSR_SHIFT),
    GYRO_RANGE_2000DPS   = (1 << BIT_GYRO_FSR_SHIFT),
    GYRO_RANGE_1000DPS   = (2 << BIT_GYRO_FSR_SHIFT),
    GYRO_RANGE_500DPS    = (3 << BIT_GYRO_FSR_SHIFT),
    GYRO_RANGE_250DPS    = (4 << BIT_GYRO_FSR_SHIFT),
    GYRO_RANGE_125DPS    = (5 << BIT_GYRO_FSR_SHIFT),
    GYRO_RANGE_62_5DPS   = (6 << BIT_GYRO_FSR_SHIFT),
    GYRO_RANGE_31_25DPS  = (7 << BIT_GYRO_FSR_SHIFT),
#else
    GYRO_RANGE_2000DPS   = (0 << BIT_GYRO_FSR_SHIFT),
    GYRO_RANGE_1000DPS   = (1 << BIT_GYRO_FSR_SHIFT),
    GYRO_RANGE_500DPS    = (2 << BIT_GYRO_FSR_SHIFT),
    GYRO_RANGE_250DPS    = (3 << BIT_GYRO_FSR_SHIFT),
    GYRO_RANGE_125DPS    = (4 << BIT_GYRO_FSR_SHIFT),
    GYRO_RANGE_62_5DPS   = (5 << BIT_GYRO_FSR_SHIFT),
    GYRO_RANGE_31_25DPS  = (6 << BIT_GYRO_FSR_SHIFT),
    GYRO_RANGE_15_625DPS = (7 << BIT_GYRO_FSR_SHIFT),
#endif
} icm4x6xx_gyro_fsr_t;

/* Filter order */
typedef enum {
    FIRST_ORDER = 0,    // 1st order
    SEC_ORDER   = 1,    // 2nd order
    THIRD_ORDER = 2,    // 3rd order
} icm4x6xx_filter_order_t;

typedef enum {
    I2C_SLEW_RATE_20_60NS = 0 << BIT_I2C_SLEW_RATE_SHIFT,
    I2C_SLEW_RATE_12_36NS = 1 << BIT_I2C_SLEW_RATE_SHIFT,
    I2C_SLEW_RATE_6_18NS  = 2 << BIT_I2C_SLEW_RATE_SHIFT,
    I2C_SLEW_RATE_4_12NS  = 3 << BIT_I2C_SLEW_RATE_SHIFT,
    I2C_SLEW_RATE_2_6NS   = 4 << BIT_I2C_SLEW_RATE_SHIFT,
    I2C_SLEW_RATE_2NS     = 5 << BIT_I2C_SLEW_RATE_SHIFT,
    SPI_SLEW_RATE_20_60NS = 0,
    SPI_SLEW_RATE_12_36NS = 1,
    SPI_SLEW_RATE_6_18NS  = 2,
    SPI_SLEW_RATE_4_12NS  = 3,
    SPI_SLEW_RATE_2_6NS   = 4,
    SPI_SLEW_RATE_2NS     = 5,
} icm4x6xx_slew_rate_t;

/* sensor bandwidth */
typedef enum {
    BW_ODR_DIV_2  = 0,
    BW_ODR_DIV_4  = 1,
    BW_ODR_DIV_5  = 2,
    BW_ODR_DIV_8  = 3,
    BW_ODR_DIV_10 = 4,
    BW_ODR_DIV_16 = 5,
    BW_ODR_DIV_20 = 6,
    BW_ODR_DIV_40 = 7,
} icm4x6xx_bandwidth_t;


/**********************************************************************************************************************
 * Exported global variables
 *********************************************************************************************************************/
//for SPI mode we could support to 500hz in fifo mode
#if SPI_MODE_EN
static uint8_t ICM4X6XX_ODR_MAPPING[] = {
    ODR_12_5HZ,
    ODR_25HZ,
    ODR_50HZ,
    ODR_100HZ,
    ODR_200HZ,
    ODR_500HZ
};

/* Support odr range 25HZ - 500HZ */
static uint32_t ICM4X6XXHWRates[] = {
    SENSOR_HZ(12.5f),
    SENSOR_HZ(25.0f),
    SENSOR_HZ(50.0f),
    SENSOR_HZ(100.0f),
    SENSOR_HZ(200.0f),
    SENSOR_HZ(500.0f),
};
#else
// for I2C bus with slow speed, it could only support to 200hz odr 
static uint8_t ICM4X6XX_ODR_MAPPING[] = {
    ODR_12_5HZ,
    ODR_25HZ,
    ODR_50HZ,
    ODR_100HZ,
    ODR_200HZ,
};

/* Support odr range 25HZ - 500HZ */
static uint32_t ICM4X6XXHWRates[] = {
    SENSOR_HZ(12.5f),
    SENSOR_HZ(25.0f),
    SENSOR_HZ(50.0f),
    SENSOR_HZ(100.0f),
    SENSOR_HZ(200.0f),
};

#endif

static uint8_t icm4x6xx_accel_discard[] = {
    ICM4X6XX_ODR_12HZ_ACCEL_STD,     /* 12.5Hz */
    ICM4X6XX_ODR_25HZ_ACCEL_STD,     /* 26Hz */
    ICM4X6XX_ODR_50HZ_ACCEL_STD,     /* 52Hz */
    ICM4X6XX_ODR_100HZ_ACCEL_STD,    /* 104Hz */
    ICM4X6XX_ODR_200HZ_ACCEL_STD,    /* 208Hz */
    ICM4X6XX_ODR_500HZ_ACCEL_STD,    /* 416Hz */
    ICM4X6XX_ODR_1000HZ_ACCEL_STD,
};

static uint8_t icm4x6xx_gyro_discard[] = {
    ICM4X6XX_ODR_12HZ_GYRO_STD,     /* 12.5Hz */
    ICM4X6XX_ODR_25HZ_GYRO_STD,     /* 26Hz */
    ICM4X6XX_ODR_50HZ_GYRO_STD,     /* 52Hz */
    ICM4X6XX_ODR_100HZ_GYRO_STD,    /* 104Hz */
    ICM4X6XX_ODR_200HZ_GYRO_STD,    /* 208Hz */
    ICM4X6XX_ODR_500HZ_GYRO_STD,    /* 416Hz */
};

/* chip type */
typedef enum {
    UNVALID_TYPE = 0,
    ICM40607,
    ICM40608,
    ICM42602,
    ICM42605,
    ICM42688,
    ICM42686,
	ICM42631,
} chip_type_t;

struct sensorConvert {
    int8_t    sign[3];
    uint8_t   axis[3];
};

struct sensorConvert map[] = {
    { { 1, 1, 1},    {0, 1, 2} },
    { { -1, 1, 1},   {1, 0, 2} },
    { { -1, -1, 1},  {0, 1, 2} },
    { { 1, -1, 1},   {1, 0, 2} },

    { { -1, 1, -1},  {0, 1, 2} },
    { { 1, 1, -1},   {1, 0, 2} },
    { { 1, -1, -1},  {0, 1, 2} },
    { { -1, -1, -1}, {1, 0, 2} },
};

typedef struct {
    uint32_t rate; //the rate from up layer want
    uint32_t hwRate; // the rate currently set
    uint32_t preRealRate; //the rate from sensor list sync with upper layer want
    uint32_t samplesToDiscard; // depends on acc or gyro start up time to valid data
    uint16_t wm; //the watermark from user
    bool powered;
    bool configed;
    bool needDiscardSample;
} ICM4X6XXSensor_t;

typedef struct inv_icm4x6xx {
    ICM4X6XXSensor_t sensors[NUM_OF_SENSOR];
    int  (*read_reg)(void * context, uint8_t reg, uint8_t * buf, uint32_t len);
    int  (*write_reg)(void * context, uint8_t reg, const uint8_t * buf, uint32_t len);

    void (*delay_ms)(uint16_t);
#if SUPPORT_DELAY_US
    void (*delay_us)(uint16_t);
#endif
    
    chip_type_t product;
    float    chip_temper;

    /* For save reg status */
    uint8_t  int_cfg;
    uint8_t  int_src0;
    uint8_t  pwr_sta;
    uint8_t  acc_cfg0;
    uint8_t  gyro_cfg0;

    /* For fifo */
    uint16_t watermark;
    uint8_t  dataBuf[ICM4X6XX_MAX_FIFO_SIZE];
    uint32_t fifoDataToRead;
    bool     fifo_mode_en;
    bool     fifo_highres_enabled;            /**< FIFO packets are 20 bytes long */
	bool     polling_data_en;
    uint16_t fifo_package_size;
    uint16_t dri_package_size;

    bool     init_cfg;
    /* For sensor oriention convert, sync to Andriod coordinate */
    struct sensorConvert cvt;

#if SENSOR_TIMESTAMP_SUPPORT
    uint16_t pre_fifo_ts;
    uint64_t totol_sensor_ts;   
#endif

#if SUPPORT_SELFTEST  
    int      avg[AXES_NUM];
    int      acc_avg[AXES_NUM];
    int      acc_avg_st[AXES_NUM];
    int      gyro_avg[AXES_NUM];
    int      gyro_avg_st[AXES_NUM];

    uint8_t  gyro_config;
    uint8_t  accel_config;
    uint8_t  st_rawdata_reg;
    uint8_t  acc_st_code[AXES_NUM];
    uint8_t  gyro_st_code[AXES_NUM];
#endif

#if (SUPPORT_PEDOMETER | SUPPORT_WOM)
    bool     dmp_power_save;
    bool     apex_enable;
#endif
} inv_icm4x6xx_t;

inv_icm4x6xx_t icm_dev;

/**********************************************************************************************************************
 * Hardware register function definiton
 *********************************************************************************************************************/
static int inv_icm4x6xx_convert_rawdata(struct accGyroDataPacket *packet);
int inv_icm4x6xx_read_rawdata();

#if SENSOR_TIMESTAMP_SUPPORT
static int inv_icm4x6xx_timestamp_enable(bool enable,bool bit_16_us);
#endif

void inv_icm4x6xx_set_serif(int (*read)(void *, uint8_t, uint8_t *, uint32_t),
                            int (*write)(void *, uint8_t, const uint8_t *, uint32_t))
{
    if (read == NULL || write == NULL) {
        INV_LOG(INV_LOG_LEVEL_ERROR, "Read/Write API NULL POINTER!!");
        exit(-1);
    }
    icm_dev.read_reg = read;
    icm_dev.write_reg = write;
}

static int inv_read(uint8_t addr, uint32_t len, uint8_t *buf)
{   
    return icm_dev.read_reg(0, addr, buf, len);
}

static int inv_write(uint8_t addr, uint8_t buf)
{
    uint8_t data = buf;
    return icm_dev.write_reg(0, addr, &data, 1);
}

void inv_icm4x6xx_set_delay(void (*delay_ms)(uint16_t), void (*delay_us)(uint16_t))
{
#if SUPPORT_DELAY_US
    if (delay_ms == NULL || delay_us == NULL) {
        INV_LOG(INV_LOG_LEVEL_ERROR, "DELAY API NULL POINTER!!");
        exit(-1);
    }
    icm_dev.delay_ms = delay_ms;
    icm_dev.delay_us = delay_us;
#else
 if (delay_ms == NULL ) {
        INV_LOG(INV_LOG_LEVEL_ERROR, "DELAY MS API NULL POINTER!!");
        exit(-1);
    }
    icm_dev.delay_ms = delay_ms;
 #endif
}

static void inv_delay_ms(uint16_t ms)
{
    icm_dev.delay_ms(ms);
}

static void inv_delay_us(uint16_t us)
{
#if SUPPORT_DELAY_US
    icm_dev.delay_us(us);
#else
	icm_dev.delay_ms(1);       // When there is no microsecond delay per platform, we delay minimal 1ms.
#endif
}

static void inv_icm4x6xx_get_whoami()
{
    int ret = 0;
    uint8_t data;

    for (int j = 0; j < 3; j++) {
        ret = inv_read(REG_WHO_AM_I, 1, &data);
        INV_LOG(SENSOR_LOG_LEVEL, "Chip Id is 0x%x ret is %d", data, ret);
    }

    switch (data) {
    case ICM42602_WHO_AM_I:
        {
            icm_dev.product = ICM42602;
            INV_LOG(SENSOR_LOG_LEVEL, "ICM42602 detected");
        }
        break;
    case ICM40607_WHO_AM_I:
        {
            icm_dev.product = ICM40607;
            INV_LOG(SENSOR_LOG_LEVEL, "ICM40607 detected");
        }
        break;
    case ICM42605_WHO_AM_I:
        {
            icm_dev.product = ICM42605;
            INV_LOG(SENSOR_LOG_LEVEL, "ICM42605 detected");
        }
        break;
    case ICM42688_WHO_AM_I:
        {
            icm_dev.product = ICM42688;
            INV_LOG(SENSOR_LOG_LEVEL, "ICM42688 detected");
        }
        break;
    case ICM42686_WHO_AM_I:
        {
            icm_dev.product = ICM42686;
            INV_LOG(SENSOR_LOG_LEVEL, "ICM42686 detected");
        }
        break;
	case ICM40608_WHO_AM_I:
        {
            icm_dev.product = ICM40608;
            INV_LOG(SENSOR_LOG_LEVEL, "ICM40608 detected\r\n");
        }
        break;
	case ICM42631_WHO_AM_I:
	{
		icm_dev.product = ICM42631;
		INV_LOG(SENSOR_LOG_LEVEL, "ICM40608 detected\r\n");
	}
	break;
    default:
        {
            icm_dev.product = UNVALID_TYPE;
            INV_LOG(SENSOR_LOG_LEVEL, "Chip not supported");
        }
        return;
    }
}

static int inv_icm4x6xx_reset_check()
{
    int ret = 0;
    uint8_t data = 0;
    
    INV_LOG(SENSOR_LOG_LEVEL, "%s", __func__);
    
    ret += inv_read(REG_CHIP_CONFIG, 1, &data);
    data |= (uint8_t)BIT_SOFT_RESET_CONFIG;
    ret += inv_write(REG_CHIP_CONFIG, data);
    
    inv_delay_ms(150);
    ret += inv_read(REG_INT_STATUS, 1, &data);
    if (data & BIT_RESET_DONE)
        INV_LOG(SENSOR_LOG_LEVEL, "ResetCheck Done 0x%x", data);
    else
        INV_LOG(SENSOR_LOG_LEVEL, "ResetCheck Fail 0x%x", data);
    
    return ret;
}

static int inv_icm4x6xx_get_convert(int direction, struct sensorConvert *cvt)
{
    struct sensorConvert *src;
    
    if (NULL == cvt)
        return -1;
    else if ((direction >= (int) sizeof(map) / (int) sizeof(map[0])) || (direction < 0))
        return -1;

    //*cvt = map[direction];
    src = &map[direction];
    memcpy(cvt, src, sizeof(struct sensorConvert));
    
    return 0;
}

#if 0
static int inv_icm4x6xx_check_fifo_format()
{
    int ret = 0;
    uint8_t fifo_header = 0;
    uint8_t value_20bit = FIFO_HEADER_A_BIT | FIFO_HEADER_G_BIT | FIFO_HEADER_20_BIT;
    uint8_t value_16bit = FIFO_HEADER_A_BIT | FIFO_HEADER_G_BIT;
    
    INV_LOG(SENSOR_LOG_LEVEL, "fifo header 0x%x", fifo_header);

    ret += inv_read(REG_FIFO_DATA, 1, &fifo_header);
    if (fifo_header & FIFO_HEADER_EMPTY_BIT)
        icm_dev.fifo_package_size = FIFO_EMPTY_PACKAGE_SIZE;
    else if ((fifo_header & value_20bit) == value_20bit)
        icm_dev.fifo_package_size = FIFO_20BYTES_PACKET_SIZE;
    else if ((fifo_header & value_16bit) == value_16bit)
        icm_dev.fifo_package_size = FIFO_16BYTES_PACKET_SIZE;
    else if (fifo_header & FIFO_HEADER_A_BIT)
        icm_dev.fifo_package_size = FIFO_8BYTES_PACKET_SIZE;
    else if (fifo_header & FIFO_HEADER_G_BIT)
        icm_dev.fifo_package_size = FIFO_8BYTES_PACKET_SIZE;
    else {
        icm_dev.fifo_package_size = FIFO_EMPTY_PACKAGE_SIZE;
        INV_LOG(SENSOR_LOG_LEVEL, "unkown header 0x%x", fifo_header);
    }

    return ret;   
}
#endif

static int inv_icm4x6xx_init_config()
{
    int ret = 0;
    uint8_t data;
    
    INV_LOG(SENSOR_LOG_LEVEL, "%s", __func__);

    icm_dev.fifo_mode_en = FIFO_WM_MODE_EN;
    icm_dev.fifo_highres_enabled = IS_HIGH_RES_MODE;
    
    ret += inv_icm4x6xx_reset_check();
    
    /* en byte mode & little endian mode & disable spi or i2c interface */
    if (SPI_MODE_EN) {
        INV_LOG(SENSOR_LOG_LEVEL, "SPI BUS");
        data = (uint8_t)(I2C_SLEW_RATE_20_60NS | SPI_SLEW_RATE_4_12NS);
        ret += inv_write(REG_DRIVE_CONFIG, data);
        
        if( (ICM42602==icm_dev.product) || (ICM42605==icm_dev.product) || (ICM42686==icm_dev.product) || (ICM42688==icm_dev.product) )
		{
		    ret += inv_write(REG_BANK_SEL, 1);
			ret = inv_read(REG_INTF_CONFIG6, 1, &data); 
			data |= (uint8_t)(BIT_I3C_SDR_EN | BIT_I3C_DDR_EN);
			ret += inv_write(REG_INTF_CONFIG6, data);
			//ret += inv_write(REG_INTF_CONFIG4, BIT_SPI_AP_4WIRE_EN);
			ret += inv_write(REG_BANK_SEL, 0);
		}
        
        data = (uint8_t)(BIT_UI_SIFS_CFG_I2C_DIS | BIT_FIFO_HOLD_LAST_DATA_EN);
    } else {
        INV_LOG(SENSOR_LOG_LEVEL, "I2C BUS");

		data = (uint8_t)(I2C_SLEW_RATE_20_60NS | SPI_SLEW_RATE_12_36NS);
        ret += inv_write(REG_DRIVE_CONFIG, data);
		
        #if 0
		if( (ICM42602==icm_dev.product) || (ICM42605==icm_dev.product) || (ICM42686==icm_dev.product) || (ICM42688==icm_dev.product) )
		{
		    ret += inv_write(REG_BANK_SEL, 1);
			ret += inv_read(REG_INTF_CONFIG6, 1, &data);
			data &= (uint8_t)~(BIT_I3C_SDR_EN | BIT_I3C_DDR_EN);
        	ret += inv_write(REG_INTF_CONFIG6, data);  
			ret += inv_write(REG_BANK_SEL, 0);
		}
        #endif
        data = (uint8_t)(BIT_UI_SIFS_CFG_SPI_DIS | BIT_FIFO_HOLD_LAST_DATA_EN);
    }
    ret += inv_write(REG_INTF_CONFIG0, data);

    // INT1 is configured here (Push pull).
    /* INT setting.  pulse mode active high by default */
    icm_dev.int_cfg = (uint8_t)BIT_INT1_PUSH_PULL;
    #if INT_LATCH_EN
    icm_dev.int_cfg |= (uint8_t)BIT_INT1_LATCHED_MODE;
    #endif
    #if INT_ACTIVE_HIGH_EN
    icm_dev.int_cfg |= (uint8_t)BIT_INT1_ACTIVE_HIGH;
    #endif
    INV_LOG(SENSOR_LOG_LEVEL, "INT1 cfg 0x%x", icm_dev.int_cfg);
    ret += inv_write(REG_INT_CONFIG, icm_dev.int_cfg);
    
    /* gyr and acc odr & fs */
    // If FSR changed, please change the definition of KSCALE_ACC_FSR and KSCALE_GYRO_FSR as well.
#if defined(ICM_42686)
    icm_dev.gyro_cfg0 = ((uint8_t)ODR_50HZ) | ((uint8_t)GYRO_RANGE_4000DPS);  
    icm_dev.acc_cfg0 = ((uint8_t)ODR_50HZ) | ((uint8_t)ACCEL_RANGE_32G);
#else
    icm_dev.gyro_cfg0 = ((uint8_t)ODR_50HZ) | ((uint8_t)GYRO_RANGE_2000DPS);

    // For ICM-42688, if IS_HIGH_RES_MODE is enabled, ACC FSR should be 16G; Gyro FSR should be 2000dps.
    if(icm_dev.fifo_highres_enabled)  
    	icm_dev.acc_cfg0 = ((uint8_t)ODR_50HZ) | ((uint8_t)ACCEL_RANGE_16G);
    else
    	icm_dev.acc_cfg0 = ((uint8_t)ODR_50HZ) | ((uint8_t)ACCEL_RANGE_8G);
#endif

    ret += inv_write(REG_GYRO_CONFIG0, icm_dev.gyro_cfg0);
    ret += inv_write(REG_ACCEL_CONFIG0, icm_dev.acc_cfg0);
    /* acc & gyro BW */
    data = (uint8_t)((BW_ODR_DIV_2 << BIT_ACCEL_FILT_BW_SHIFT) | BW_ODR_DIV_4);
    ret += inv_write(REG_GYRO_ACCEL_CONFIG0, data);
    /* acc & gyro UI filter @ 3th order */
    data = (uint8_t)(BIT_GYRO_AVG_FILT_8K_HZ | (THIRD_ORDER << BIT_GYRO_FILT_ORD_SHIFT));
    ret += inv_write(REG_GYRO_CONFIG1, data);
    data = (uint8_t)(BIT_ACCEL_AVG_FILT_8K_HZ | (THIRD_ORDER << BIT_ACCEL_FILT_ORD_SHIFT));
    ret += inv_write(REG_ACCEL_CONFIG1, data);
    
    if (true == icm_dev.fifo_mode_en)
    {
        /* FIFO mode configuration */
        data = (uint8_t)(BIT_FIFO_WM_GT_TH_MASK | BIT_FIFO_TMST_FSYNC_EN_MASK | \
                         BIT_FIFO_TEMP_EN_EN| BIT_FIFO_GYRO_EN_EN | BIT_FIFO_ACCEL_EN_EN);

        if(icm_dev.fifo_highres_enabled)
        	data |= BIT_FIFO_HIRES_EN;
        ret += inv_write(REG_FIFO_CONFIG_1, data);
        
        if(icm_dev.fifo_highres_enabled)
        	icm_dev.fifo_package_size = FIFO_20BYTES_PACKET_SIZE;
        else
        	icm_dev.fifo_package_size = FIFO_16BYTES_PACKET_SIZE;
        	
        /* set wm_th=16 as default for byte mode */
        ret += inv_write(REG_FIFO_WM_TH_L, 0x10);/* byte mode */
        ret += inv_write(REG_FIFO_WM_TH_H, 0x00);
        /* bypass fifo */
        ret += inv_write(REG_FIFO_CONFIG, BIT_FIFO_MODE_CTRL_BYPASS);
        /* enable fifo full & WM INT */
        icm_dev.int_src0 |= (uint8_t)(BIT_INT1_FIFO_FULL_EN | BIT_INT1_WM_EN);
        ret += inv_write(REG_INT_SOURCE0, icm_dev.int_src0);
        INV_LOG(SENSOR_LOG_LEVEL, "FIFO MODE INT_SRC0 0x%x", icm_dev.int_src0);
    }
    else
    {
        //DRDY mode
        /* enable DRDY INT */
        icm_dev.int_src0 |= (uint8_t)BIT_INT1_DRDY_EN;
        ret += inv_write(REG_INT_SOURCE0, icm_dev.int_src0);
        icm_dev.dri_package_size = DRI_14BYTES_PACKET_SIZE;
        INV_LOG(SENSOR_LOG_LEVEL, "DRI MODE INT_SRC0 0x%x", icm_dev.int_src0);
    }
    
    /* async reset */
    ret += inv_write(REG_INT_CONFIG1, (uint8_t)BIT_INT_ASY_RST_DIS); //The field int_asy_rst_disable must be 0 for icm4x6xx

    /* get sensor default setting */
    ret += inv_read(REG_PWR_MGMT0, 1, &icm_dev.pwr_sta);

    #if GYRO_STALL_PATCH_ENABLE
    /* fix gyro stall issue */
    ret += inv_write(REG_BANK_SEL, 3);
    uint8_t reg_2e, reg_32, reg_37, reg_3c;
    ret += inv_read(0x2e, 1, &reg_2e); 
    ret += inv_read(0x32, 1, &reg_32); 
    ret += inv_read(0x37, 1, &reg_37); 
    ret += inv_read(0x3c, 1, &reg_3c); 

    data = 0xfd & reg_2e; 
    ret += inv_write(0x2e, data);
    data = 0x9f & reg_32; 
    ret += inv_write(0x32, data);
    data = 0x9f & reg_37; 
    ret += inv_write(0x37, data);
    data = 0x9f & reg_3c; 
    ret += inv_write(0x3c, data);
    ret += inv_write(REG_BANK_SEL, 0);
    #endif
    
    return ret;
}

int inv_icm4x6xx_initialize()
{
    int ret = 0;

    icm_dev.product = UNVALID_TYPE;
    icm_dev.int_cfg = 0;
    icm_dev.int_src0 = 0;
    icm_dev.pwr_sta = 0;
    icm_dev.acc_cfg0 = 0;
    icm_dev.gyro_cfg0 = 0;
    icm_dev.init_cfg = false;
#if (SUPPORT_PEDOMETER | SUPPORT_WOM)
    icm_dev.dmp_power_save = false;
    icm_dev.apex_enable = true;
#endif
    icm_dev.fifo_package_size = 0;
    icm_dev.dri_package_size = 0;
    icm_dev.polling_data_en = false;

#if SENSOR_TIMESTAMP_SUPPORT
    icm_dev.pre_fifo_ts = 0;
    icm_dev.totol_sensor_ts = 0;
#endif

    inv_icm4x6xx_get_whoami();
    
    if (icm_dev.product == UNVALID_TYPE)
        return SENSOR_WHOAMI_INVALID_ERROR;

    ret += inv_icm4x6xx_get_convert(SENSOR_DIRECTION, &icm_dev.cvt);
    if (ret != 0)
        return SENSOR_CONVERT_INVALID_ERROR;

    INV_LOG(SENSOR_LOG_LEVEL, "sensor axis[0]:%d, axis[1]:%d, axis[2]:%d, sign[0]:%d, sign[1]:%d, sign[2]:%d",
          icm_dev.cvt.axis[AXIS_X], icm_dev.cvt.axis[AXIS_Y], icm_dev.cvt.axis[AXIS_Z],
          icm_dev.cvt.sign[AXIS_X], icm_dev.cvt.sign[AXIS_Y], icm_dev.cvt.sign[AXIS_Z]);
    
    ret += inv_icm4x6xx_init_config();
#if SENSOR_TIMESTAMP_SUPPORT
    ret += inv_icm4x6xx_timestamp_enable(SENSOR_TIMESTAMP_SUPPORT,SUPPORT_TIMESTAMP_16US);
#endif    
    if (ret == 0) {
        icm_dev.init_cfg = true;
        INV_LOG(SENSOR_LOG_LEVEL, "Initialize Success");
    } else {
        INV_LOG(SENSOR_LOG_LEVEL, "Initialize Failed %d", ret);
        ret = SENSOR_INIT_INVALID_ERROR;
    }

    return ret;
}

static int inv_icm4x6xx_set_odr(int index)
{
    int ret = 0;
    uint8_t regValue = 0;
    
    regValue = ICM4X6XX_ODR_MAPPING[index];
    INV_LOG(SENSOR_LOG_LEVEL, "Odr Reg value 0x%x", regValue);
    icm_dev.gyro_cfg0 &= (uint8_t)~BIT_GYRO_ODR_MASK;
    icm_dev.gyro_cfg0 |= regValue;
    ret += inv_write(REG_GYRO_CONFIG0, icm_dev.gyro_cfg0);
    icm_dev.acc_cfg0 &= (uint8_t)~BIT_ACCEL_ODR_MASK;
    icm_dev.acc_cfg0 |= regValue;
    ret += inv_write(REG_ACCEL_CONFIG0, icm_dev.acc_cfg0);

    return ret;
}

static int inv_icm4x6xx_cal_odr(uint32_t *rate, uint32_t *report_rate)
{
    uint8_t i;

    for (i = 0; i < (ARRAY_SIZE(ICM4X6XXHWRates)); i++) {
        if (*rate <= ICM4X6XXHWRates[i]) {
            *report_rate = ICM4X6XXHWRates[i];
            break;
        }
    }

    if (*rate > ICM4X6XXHWRates[(ARRAY_SIZE(ICM4X6XXHWRates) - 1)]) {
        i = (ARRAY_SIZE(ICM4X6XXHWRates) - 1);
        *report_rate = ICM4X6XXHWRates[i];
    }

    return i;
}

static uint16_t inv_icm4x6xx_cal_wm(uint16_t watermark)
{
    uint8_t min_watermark = 2;
    uint8_t max_watermark ;
    uint16_t real_watermark = 0;

#if SPI_MODE_EN
    if(icm_dev.sensors[GYR].hwRate >= SENSOR_HZ(500.0f) || icm_dev.sensors[ACC].hwRate >= SENSOR_HZ(500.0f))
        min_watermark = 4;
#else
    if(icm_dev.sensors[GYR].hwRate >= SENSOR_HZ(200.0f) || icm_dev.sensors[ACC].hwRate >= SENSOR_HZ(200.0f))
        min_watermark = 4;
#endif
    // For Yokohama the max FIFO size is 2048 Bytes, common FIFO package 16 Bytes, so the max number of FIFO pakage is 128.
    max_watermark = 70 < (MAX_RECV_PACKET/2) ? 70 : (MAX_RECV_PACKET/2); /*64*/

    real_watermark = watermark;
    real_watermark = real_watermark < min_watermark ? min_watermark : real_watermark;
    real_watermark = real_watermark > max_watermark ? max_watermark : real_watermark;
    
    return real_watermark * icm_dev.fifo_package_size;/* byte mode*/
}

static int inv_icm4x6xx_read_fifo()
{
    int ret = 0;
    uint8_t count[2];
    ret += inv_read(REG_FIFO_COUNT_H, 2, count);
    icm_dev.fifoDataToRead = (((uint32_t)count[1]) <<8) | (uint32_t) count[0];
    
    //INV_LOG(SENSOR_LOG_LEVEL, "Fifo count is %d", icm_dev.fifoDataToRead);

    if (icm_dev.fifoDataToRead <= 0 || icm_dev.fifoDataToRead > ICM4X6XX_MAX_FIFO_SIZE)
        return FIFO_COUNT_INVALID_ERROR;

    ret += inv_read(REG_FIFO_DATA, icm_dev.fifoDataToRead, icm_dev.dataBuf);

    return ret;
}

static int inv_icm4x6xx_config_fifo(bool enable)
{
    int ret = 0;

    if (true == enable) {
        uint8_t  buffer[2];
        uint16_t watermarkReg;

        watermarkReg = icm_dev.watermark;

        if (watermarkReg < icm_dev.fifo_package_size)
            watermarkReg = icm_dev.fifo_package_size;
    
        INV_LOG(SENSOR_LOG_LEVEL, "%s watermarkReg %d", __func__, watermarkReg);
    
        buffer[0] = (uint8_t)(watermarkReg & 0x00FF);
        buffer[1] = (uint8_t)((watermarkReg & 0xFF00) >> 8);
        /* set bypass mode to reset fifo */
        ret += inv_write(REG_FIFO_CONFIG, BIT_FIFO_MODE_CTRL_BYPASS);
        /* set threshold */
        ret += inv_write(REG_FIFO_WM_TH_L, buffer[0]);
        ret += inv_write(REG_FIFO_WM_TH_H, buffer[1]);
        if ((true == icm_dev.sensors[ACC].configed) ||
            (true == icm_dev.sensors[GYR].configed)) {
            /* set fifo stream mode */
            ret += inv_write(REG_FIFO_CONFIG, BIT_FIFO_MODE_CTRL_STREAM);
            INV_LOG(SENSOR_LOG_LEVEL, "Reset, TH_L:0x%x, TH_H:0x%x", buffer[0], buffer[1]);
        } else {
            /* set fifo bypass mode as no sensor configured */
            INV_LOG(SENSOR_LOG_LEVEL, "Fifo to Bypass Mode");
            ret += inv_write(REG_FIFO_CONFIG, BIT_FIFO_MODE_CTRL_BYPASS);
        }
              
    } else {
        if ((false == icm_dev.sensors[ACC].configed) &&
            (false == icm_dev.sensors[GYR].configed)) {
            /* set fifo bypass mode as no sensor configured */
            INV_LOG(SENSOR_LOG_LEVEL, "Fifo to Bypass Mode");
            ret += inv_write(REG_FIFO_CONFIG, BIT_FIFO_MODE_CTRL_BYPASS);
        }
    }

    return ret;
}

int inv_icm4x6xx_config_drdy(bool enable)
{
    int ret = 0;

    if (enable &&
        (true == icm_dev.sensors[ACC].configed ||
        true == icm_dev.sensors[GYR].configed)) {
        if (!(icm_dev.int_src0 & BIT_INT1_DRDY_EN)) {
            icm_dev.int_src0 |= (uint8_t)BIT_INT1_DRDY_EN;
            ret += inv_write(REG_INT_SOURCE0, icm_dev.int_src0);
            INV_LOG(SENSOR_LOG_LEVEL, "Enable DRDY INT1 0x%x", icm_dev.int_src0);
        }
    } else {
        icm_dev.int_src0 &= (uint8_t)~BIT_INT1_DRDY_EN;
        ret += inv_write(REG_INT_SOURCE0, icm_dev.int_src0);
        INV_LOG(SENSOR_LOG_LEVEL, "Disable DRDY INT1 0x%x", icm_dev.int_src0);
    }

    return ret;
}

int inv_icm4x6xx_config_fifo_int(bool enable)
{
    int ret = 0;

    if (enable &&
        (true == icm_dev.sensors[ACC].configed ||
        true == icm_dev.sensors[GYR].configed)) {
        if (!(icm_dev.int_src0 & BIT_INT1_WM_EN)) {
            icm_dev.int_src0 |= (uint8_t)BIT_INT1_WM_EN;
            ret += inv_write(REG_INT_SOURCE0, icm_dev.int_src0);
            INV_LOG(SENSOR_LOG_LEVEL, "Enable FIFO WM INT1 0x%x", icm_dev.int_src0);
        }
    } else {
        icm_dev.int_src0 &= (uint8_t)~BIT_INT1_WM_EN;
        ret += inv_write(REG_INT_SOURCE0, icm_dev.int_src0);
        INV_LOG(SENSOR_LOG_LEVEL, "Disable FIFO WM INT1 0x%x", icm_dev.int_src0);
    }

    return ret;
}

int inv_icm4x6xx_config_fifofull_int(bool enable)
{
    int ret = 0;

    if (enable &&
        (true == icm_dev.sensors[ACC].configed ||
        true == icm_dev.sensors[GYR].configed)) {
        if (!(icm_dev.int_src0 & BIT_INT1_FIFO_FULL_EN)) {
            icm_dev.int_src0 |= (uint8_t)BIT_INT1_FIFO_FULL_EN;
            ret += inv_write(REG_INT_SOURCE0, icm_dev.int_src0);
            INV_LOG(SENSOR_LOG_LEVEL, "Enable FIFO WM INT1 0x%x", icm_dev.int_src0);
        }
    } else {
        icm_dev.int_src0 &= (uint8_t)~BIT_INT1_FIFO_FULL_EN;
        ret += inv_write(REG_INT_SOURCE0, icm_dev.int_src0);
        INV_LOG(SENSOR_LOG_LEVEL, "Disable FIFO WM INT1 0x%x", icm_dev.int_src0);
    }

    return ret;
}

#if SENSOR_TIMESTAMP_SUPPORT
static int inv_icm4x6xx_timestamp_enable(bool enable,bool bit_16_us)
{
    int ret = 0;
	uint8_t data;
	
    INV_LOG(SENSOR_LOG_LEVEL, "%s", __func__);

	ret = inv_read(REG_TMST_CONFIG,1, &data);

	INV_LOG(SENSOR_LOG_LEVEL, "READ TMST 0x%x", data);
	//clear all setting in TMST reg
    data &= (uint8_t)~(BIT_TMST_TO_REGS_EN|BIT_TMST_RES_16US|BIT_TMST_DELTA_EN|BIT_TMST_DELTA_EN|BIT_TMST_FSYNC_EN|BIT_TMST_EN);

	if(enable){
    	data |=  BIT_TMST_TO_REGS_EN| BIT_TMST_EN;

	    if(bit_16_us)
			data |= BIT_TMST_RES_16US;
	}
	
    ret = inv_write(REG_TMST_CONFIG, data);

	ret = inv_read(REG_TMST_CONFIG,1, &data);

    INV_LOG(SENSOR_LOG_LEVEL, "TMST 0x%x", data);
    
    return ret;
}

int inv_icm4x6xx_get_sensor_timestamp(uint64_t *timestamp){

	int ret = 0;
	uint8_t	 data;
	uint8_t  dataBuf[3] ={0};
	uint32_t timestamp_reg;

	/* Enable timestamp counter to be latched in timestamp register */
	ret = inv_read(REG_SIGNAL_PATH_RESET,1, &data);
	data |= BITTMST_STROBE_EN;
	ret += inv_write(REG_SIGNAL_PATH_RESET, data);
	
    ret += inv_write(REG_BANK_SEL, 1);  
	ret += inv_read(REG_TMSTVAL0, 3, dataBuf);
	ret += inv_write(REG_BANK_SEL, 0); 

	timestamp_reg =  ((uint32_t)(dataBuf[2] & 0x0F) << 16) + ((uint32_t)dataBuf[1] << 8) + dataBuf[0];
	/*INV_LOG(SENSOR_LOG_LEVEL, "READ timestamp reg %d", timestamp_reg);*/
	
    #if SUPPORT_TIMESTAMP_16US
		*timestamp = timestamp_reg *16;
	#else
		*timestamp = timestamp_reg;
	#endif
	return ret;

}
#endif

int inv_icm4x6xx_acc_enable()
{
    int ret = 0;
    
    INV_LOG(SENSOR_LOG_LEVEL, "%s", __func__);
    
    icm_dev.sensors[ACC].powered = true;
    icm_dev.pwr_sta &= (uint8_t)(~BIT_ACCEL_MODE_MASK);
    icm_dev.pwr_sta |= BIT_ACCEL_MODE_LN;
    ret = inv_write(REG_PWR_MGMT0, icm_dev.pwr_sta);
    //inv_delay_ms(40);
    INV_LOG(SENSOR_LOG_LEVEL, "ACC PWR 0x%x", icm_dev.pwr_sta);
    
    #if SENSOR_REG_DUMP
    inv_icm4x6xx_dumpRegs();
    #endif
    
    return ret;
}

int inv_icm4x6xx_set_acc_power_mode(uint8_t mode)
{
	 int ret = 0;
     uint8_t data;

	 //INVLOG(LOG_INFO, "ICM4X6XX set power mode=%d\n", mode);
	if(icm_dev.sensors[ACC].hwRate > SENSOR_HZ(500.0f) || icm_dev.sensors[GYR].hwRate > SENSOR_HZ(500.0f)){
		if(BIT_ACC_LP_MODE ==mode){
			 INV_LOG(SENSOR_LOG_LEVEL, "Do not support LP Mode in high ODR");
			 return 0;
			}	
	}
	
    if(BIT_ACC_LN_MODE == mode){
         /* acc & gyro BW acc BW /2 in LN Mode*/
	  /* acc & gyro BW */
    	data = (uint8_t)((BW_ODR_DIV_2 << BIT_ACCEL_FILT_BW_SHIFT) | BW_ODR_DIV_4);
    	ret += inv_write(REG_GYRO_ACCEL_CONFIG0, data);
    	}   
    else if(BIT_ACC_LP_MODE == mode){
         /* acc & gyro BW acc BW /4 in LP Mode*/
		data = (uint8_t)((BW_ODR_DIV_4 << BIT_ACCEL_FILT_BW_SHIFT) | BW_ODR_DIV_4);
    	ret += inv_write(REG_GYRO_ACCEL_CONFIG0, data);
    }
   // else if(BIT_ACC_OFF == mode)
   //     INVLOG(LOG_INFO, "ICM4X6XX set acc off mode"); 
    else{
		ret = 1;
      //  INVLOG(LOG_INFO, "invalid mode select\n");
    	}
    icm_dev.pwr_sta &= 0xFC;
    icm_dev.pwr_sta |= mode;
    // set 10ms for accel start-up time, note:at least 1ms here to sync fly changes
    ret += inv_write(REG_PWR_MGMT0, icm_dev.pwr_sta);
    inv_delay_ms(30);

	return ret;

}

int inv_icm4x6xx_acc_disable()
{
    int ret = 0;
    int odr_index = 0;
    uint32_t sampleRate = 0;
    bool accelOdrChanged = false;
    bool watermarkChanged = false;

    INV_LOG(SENSOR_LOG_LEVEL, "%s", __func__);
    
    icm_dev.sensors[ACC].preRealRate = 0;
    icm_dev.sensors[ACC].hwRate = 0;
    icm_dev.sensors[ACC].needDiscardSample = false;
    icm_dev.sensors[ACC].samplesToDiscard = 0;
    icm_dev.sensors[ACC].wm = 0;
        
    if ((false == icm_dev.sensors[GYR].powered) 
        #if SUPPORT_PEDOMETER
        && (false == icm_dev.sensors[PEDO].powered)
        #endif
        #if SUPPORT_WOM
        && (false == icm_dev.sensors[WOM].powered)
        #endif 
        ) {
        icm_dev.pwr_sta &= (uint8_t)~(BIT_ACCEL_MODE_MASK | BIT_GYRO_MODE_MASK);
        ret += inv_write(REG_PWR_MGMT0, icm_dev.pwr_sta);
        inv_delay_us(200); //spec: 200us

        INV_LOG(SENSOR_LOG_LEVEL, "acc off pwr: 0x%x", icm_dev.pwr_sta);
    } else if (true == icm_dev.sensors[GYR].powered) {  //  update gyro old odr
        if (icm_dev.sensors[GYR].hwRate != icm_dev.sensors[GYR].preRealRate) {
            icm_dev.sensors[GYR].hwRate = icm_dev.sensors[GYR].preRealRate;
            odr_index = inv_icm4x6xx_cal_odr(&icm_dev.sensors[GYR].hwRate, &sampleRate);
            ret += inv_icm4x6xx_set_odr(odr_index);
            INV_LOG(SENSOR_LOG_LEVEL, "gyro revert rate to preRealRate: %f Hz", 
                  ((float)icm_dev.sensors[GYR].hwRate/1024.0f));
            accelOdrChanged = true;
        }
        if (icm_dev.fifo_mode_en && (icm_dev.watermark != icm_dev.sensors[GYR].wm)) {
            icm_dev.watermark = icm_dev.sensors[GYR].wm;
            watermarkChanged = true;
            INV_LOG(SENSOR_LOG_LEVEL, "watermark revert to: %d", icm_dev.watermark/icm_dev.fifo_package_size);
        }
            
        if (true 
            #if SUPPORT_PEDOMETER
            && (false == icm_dev.sensors[PEDO].powered)
            #endif
            #if SUPPORT_WOM
            && (false == icm_dev.sensors[WOM].powered)
            #endif 
            ) {
            icm_dev.pwr_sta &= (uint8_t)~(BIT_ACCEL_MODE_MASK);
            ret += inv_write(REG_PWR_MGMT0, icm_dev.pwr_sta);
            inv_delay_us(200); //spec: 200us

            INV_LOG(SENSOR_LOG_LEVEL, "gyro on and acc off pwr: 0x%x", icm_dev.pwr_sta);
        } else {
            INV_LOG(SENSOR_LOG_LEVEL, "gyro on and apex on pwr: 0x%x", icm_dev.pwr_sta);
        }
    }

    icm_dev.sensors[ACC].powered = false;
    icm_dev.sensors[ACC].configed = false;

    if (true == icm_dev.fifo_mode_en) {
        inv_icm4x6xx_config_fifo(accelOdrChanged | watermarkChanged);
    } else {
        inv_icm4x6xx_config_drdy(accelOdrChanged);
    }
    
    #if 0
    if (accelOdrChanged | watermarkChanged) {
        //do nothing
    }
    #endif
    
    return ret;
}

int inv_icm4x6xx_gyro_enable()
{
    int ret = 0;

    INV_LOG(SENSOR_LOG_LEVEL, "%s", __func__);
    
    icm_dev.sensors[GYR].powered = true;
    icm_dev.pwr_sta &= (uint8_t)~BIT_GYRO_MODE_MASK;
    icm_dev.pwr_sta |= BIT_GYRO_MODE_LN;
    ret = inv_write(REG_PWR_MGMT0, icm_dev.pwr_sta);
    //100ms here to discard unvalid data
    //inv_delay_ms(100);
    INV_LOG(SENSOR_LOG_LEVEL, "GYR PWR 0x%x", icm_dev.pwr_sta);
    
    #if SENSOR_REG_DUMP
    inv_icm4x6xx_dumpRegs();
    #endif

    return ret;
}

int inv_icm4x6xx_gyro_disable()
{
    int ret = 0;
    int odr_index = 0;
    uint32_t sampleRate = 0;
    bool gyroOdrChanged = false;
    bool watermarkChanged = false;

    INV_LOG(SENSOR_LOG_LEVEL, "%s", __func__);
    
    icm_dev.sensors[GYR].preRealRate = 0;
    icm_dev.sensors[GYR].hwRate = 0;
    icm_dev.sensors[GYR].needDiscardSample = false;
    icm_dev.sensors[GYR].samplesToDiscard = 0;
    icm_dev.sensors[GYR].wm = 0;

    if (true == icm_dev.sensors[ACC].powered) {  //  update ACC old ord
        if(icm_dev.sensors[ACC].hwRate != icm_dev.sensors[ACC].preRealRate) {
            icm_dev.sensors[ACC].hwRate = icm_dev.sensors[ACC].preRealRate;
            odr_index = inv_icm4x6xx_cal_odr(&icm_dev.sensors[ACC].hwRate, &sampleRate);
            ret += inv_icm4x6xx_set_odr(odr_index);
            gyroOdrChanged = true;
            INV_LOG(SENSOR_LOG_LEVEL, "acc revert rate to preRealRate: %f Hz",
                ((float)icm_dev.sensors[ACC].hwRate/1024.0f));
            if (icm_dev.fifo_mode_en && (icm_dev.watermark != icm_dev.sensors[ACC].wm)) {
                icm_dev.watermark = icm_dev.sensors[ACC].wm;
                watermarkChanged = true;
                INV_LOG(SENSOR_LOG_LEVEL, "watermark revert to: %d", icm_dev.watermark/icm_dev.fifo_package_size);
            }
        }
        icm_dev.pwr_sta &= (uint8_t)~BIT_GYRO_MODE_MASK; // Gyro OFF  also keep acc in LN Mode
        ret += inv_write(REG_PWR_MGMT0, icm_dev.pwr_sta);
        inv_delay_ms(150);
        INV_LOG(SENSOR_LOG_LEVEL, "acc on and gyro off pwr: 0x%x", icm_dev.pwr_sta);
    } else {
        /* Gyro OFF. In case apex on not ~(BIT_GYRO_MODE_MASK | BIT_ACCEL_MODE_MASK)*/
        icm_dev.pwr_sta &= (uint8_t)~BIT_GYRO_MODE_MASK; 
        ret += inv_write(REG_PWR_MGMT0, icm_dev.pwr_sta);
        inv_delay_ms(150);
        INV_LOG(SENSOR_LOG_LEVEL, "gyro off pwr: 0x%x", icm_dev.pwr_sta);
    }
    
    icm_dev.sensors[GYR].powered = false;
    icm_dev.sensors[GYR].configed = false;

    if (true == icm_dev.fifo_mode_en) {
        inv_icm4x6xx_config_fifo(gyroOdrChanged | watermarkChanged);
    } else {
        inv_icm4x6xx_config_drdy(gyroOdrChanged);
    }
    #if 0
    if (gyroOdrChanged | watermarkChanged) {
        //do nothing
    }
    #endif
    
    return ret;
}

int inv_icm4x6xx_acc_set_rate(float odr_hz, uint16_t watermark,float *hw_odr)
{
    int ret = 0;
    int odr_index = 0;
    uint32_t sampleRate = 0;
    uint32_t maxRate = 0;
    bool accelOdrChanged = false;
    bool watermarkChanged = false;

    INV_LOG(SENSOR_LOG_LEVEL, "%s odr_hz %f wm %d", __func__, odr_hz, watermark);
    icm_dev.sensors[ACC].rate = SENSOR_HZ(odr_hz);
    
    #if (SUPPORT_PEDOMETER | SUPPORT_WOM)
    if ((true == icm_dev.apex_enable) &&
        (icm_dev.sensors[ACC].rate < SENSOR_HZ(50))) {
        INV_LOG(SENSOR_LOG_LEVEL, "APEX Enabled, Acc min odr to 50Hz!!");
        icm_dev.sensors[ACC].rate = SENSOR_HZ(50);  
    }
    #endif
    if (icm_dev.sensors[ACC].preRealRate == 0) {
        icm_dev.sensors[ACC].needDiscardSample = true;
        //icm_dev.sensors[ACC].samplesToDiscard = NUM_TODISCARD;
    } else {
        icm_dev.sensors[ACC].needDiscardSample = false;
    }

    odr_index = inv_icm4x6xx_cal_odr(&icm_dev.sensors[ACC].rate, &sampleRate);
    icm_dev.sensors[ACC].preRealRate = sampleRate;

    /* if gyr configed ,compare maxRate with acc and gyr rate */
    if (true == icm_dev.sensors[GYR].configed) {
        maxRate = max(sampleRate, icm_dev.sensors[GYR].preRealRate);// choose with preRealRate
        if (maxRate != icm_dev.sensors[ACC].hwRate ||
            maxRate != icm_dev.sensors[GYR].hwRate) {
            icm_dev.sensors[ACC].hwRate = maxRate;
            icm_dev.sensors[GYR].hwRate = maxRate;
            INV_LOG(SENSOR_LOG_LEVEL, "New Acc/Gyro config Rate %f Hz",
                ((float)icm_dev.sensors[ACC].hwRate/1024.0f));
            odr_index = inv_icm4x6xx_cal_odr(&maxRate, &sampleRate);
            ret += inv_icm4x6xx_set_odr(odr_index);
            accelOdrChanged = true;
        } else
            accelOdrChanged = false;
    } else {
        if ((sampleRate != icm_dev.sensors[ACC].hwRate)) {
            icm_dev.sensors[ACC].hwRate = sampleRate;
            INV_LOG(SENSOR_LOG_LEVEL, "New Acc config Rate %f Hz",
                ((float)icm_dev.sensors[ACC].hwRate/1024.0f));
            ret += inv_icm4x6xx_set_odr(odr_index);
            accelOdrChanged = true;
        } else
            accelOdrChanged = false;
    }
    icm_dev.sensors[ACC].configed = true;
	*hw_odr = (float)(icm_dev.sensors[ACC].hwRate / 1024 );

    if(true == icm_dev.sensors[ACC].needDiscardSample)
        icm_dev.sensors[ACC].samplesToDiscard = icm4x6xx_accel_discard[odr_index];

    //For fifo mode
    if (true == icm_dev.fifo_mode_en) {
        icm_dev.sensors[ACC].wm = inv_icm4x6xx_cal_wm(watermark);
        if (icm_dev.sensors[ACC].wm != icm_dev.watermark) {
            watermarkChanged = true;
            icm_dev.watermark = icm_dev.sensors[ACC].wm;
            INV_LOG(SENSOR_LOG_LEVEL, "New Watermark is %d", icm_dev.watermark/icm_dev.fifo_package_size);
        } else {
            watermarkChanged = false;
        }
        inv_icm4x6xx_config_fifo(accelOdrChanged | watermarkChanged);
    } else {
        inv_icm4x6xx_config_drdy(accelOdrChanged);
    }

    #if 0
    if(accelOdrChanged || watermarkChanged){    
        //* Clear the interrupt */
        //do nothing right now
    }
    #endif

    return ret;
}

int inv_icm4x6xx_gyro_set_rate(float odr_hz, uint16_t watermark,float *hw_odr)
{
    int ret = 0;
    int odr_index = 0;
    uint32_t sampleRate = 0;
    uint32_t maxRate = 0;
    bool gyroOdrChanged = false;
    bool watermarkChanged = false;
    
    INV_LOG(SENSOR_LOG_LEVEL, "%s odr_hz %f wm %d", __func__, odr_hz, watermark);
    icm_dev.sensors[GYR].rate = SENSOR_HZ(odr_hz);

    if (icm_dev.sensors[GYR].preRealRate == 0) {
        icm_dev.sensors[GYR].needDiscardSample = true;
        //icm_dev.sensors[GYR].samplesToDiscard = NUM_TODISCARD;
    }else{
        icm_dev.sensors[GYR].needDiscardSample = false;
    }


    /* get hw sample rate */
    odr_index = inv_icm4x6xx_cal_odr(&icm_dev.sensors[GYR].rate, &sampleRate);

    icm_dev.sensors[GYR].preRealRate = sampleRate;

    /* if acc configed ,compare maxRate with acc and gyr rate */
    if (true == icm_dev.sensors[ACC].configed) {
        maxRate = max(sampleRate, icm_dev.sensors[ACC].preRealRate);
        if (maxRate != icm_dev.sensors[ACC].hwRate ||
            maxRate != icm_dev.sensors[GYR].hwRate) {
            icm_dev.sensors[ACC].hwRate = maxRate;
            icm_dev.sensors[GYR].hwRate = maxRate;
            INV_LOG(SENSOR_LOG_LEVEL, "New Gyro/Acc config Rate %f Hz",
                ((float)icm_dev.sensors[GYR].hwRate/1024.0f));
            /* update new odr */
            odr_index = inv_icm4x6xx_cal_odr(&maxRate, &sampleRate);
            ret += inv_icm4x6xx_set_odr(odr_index);
            gyroOdrChanged = true;
        } else
            gyroOdrChanged = false;
    } else {
        if ((sampleRate != icm_dev.sensors[GYR].hwRate)) {
            icm_dev.sensors[GYR].hwRate = sampleRate;
            INV_LOG(SENSOR_LOG_LEVEL, "New Gyro config Rate %f Hz",
                ((float)icm_dev.sensors[GYR].hwRate/1024.0f));
            /* update new odr */
            ret += inv_icm4x6xx_set_odr(odr_index);
            gyroOdrChanged = true;
        } else
            gyroOdrChanged = false;
    }

    icm_dev.sensors[GYR].configed = true;
	*hw_odr = (float)(icm_dev.sensors[GYR].hwRate / 1024);
		
    if(true == icm_dev.sensors[GYR].needDiscardSample)
        icm_dev.sensors[GYR].samplesToDiscard = icm4x6xx_gyro_discard[odr_index];
    
    //For fifo mode
    if (true == icm_dev.fifo_mode_en) {
        icm_dev.sensors[GYR].wm = inv_icm4x6xx_cal_wm(watermark);
        if (icm_dev.sensors[GYR].wm != icm_dev.watermark) {
            watermarkChanged = true;
            icm_dev.watermark = icm_dev.sensors[GYR].wm;
            INV_LOG(SENSOR_LOG_LEVEL, "New Watermark is %d", icm_dev.watermark/icm_dev.fifo_package_size);
        } else {
            watermarkChanged = false;
        }
        inv_icm4x6xx_config_fifo(gyroOdrChanged | watermarkChanged);
    } else {
        inv_icm4x6xx_config_drdy(gyroOdrChanged);
    }

    #if 0
    if(gyroOdrChanged || watermarkChanged)
    {
    
        //* Clear the interrupt */
        //do nothing right now
    }
    #endif
    
    return ret;
}

static void inv_icm4x6xx_parse_rawdata(uint8_t *buf, SensorType_t sensorType, struct accGyroData *data)
{
	uint8_t high_res_rel_addr=0;
	int i=0;

	// Output raw A+G data.	
	if( TEMP==sensorType ) 
	{
		if (true == icm_dev.fifo_mode_en)
		{
			if( icm_dev.fifo_highres_enabled)
				/* Use little endian mode */
				data->temperature = (int16_t)(buf[0] | buf[1] << 8);
			else data->temperature = (int8_t)buf[0];
		} 
		else 
			/* Use little endian mode */
			data->temperature = (int16_t)(buf[0] | buf[1] << 8);

		return;
	}
	
#if SENSOR_TIMESTAMP_SUPPORT
    uint16_t cur_fifo_ts = 0;

    if( TS==sensorType)
    {
        /* Use little endian mode */
        cur_fifo_ts = (uint16_t)(buf[0] | buf[1] << 8);

        //INV_LOG(SENSOR_LOG_LEVEL, "TS reg %x %x", buf[1],buf[0]);

        if (cur_fifo_ts != icm_dev.pre_fifo_ts) {
            /* Check for possible overflow */
            if (cur_fifo_ts < icm_dev.pre_fifo_ts) {
                icm_dev.totol_sensor_ts += (uint64_t)(cur_fifo_ts + (0xFFFF - icm_dev.pre_fifo_ts));
            } else {
                icm_dev.totol_sensor_ts +=  (uint64_t)((cur_fifo_ts - icm_dev.pre_fifo_ts));
            }

            icm_dev.pre_fifo_ts = cur_fifo_ts;
            //INV_LOG(SENSOR_LOG_LEVEL, "totol %lld cur %d, pre %d",  icm_dev.totol_sensor_ts,cur_fifo_ts,icm_dev.pre_fifo_ts);
        }

        #if SUPPORT_TIMESTAMP_16US
		data->timeStamp  = icm_dev.totol_sensor_ts *16;
	    #else
		data->timeStamp  = icm_dev.totol_sensor_ts;
	    #endif
    }
#endif

	if( (ACC==sensorType)||(GYR==sensorType) ) 
	{
		/* Use little endian mode */
		data->x = (int16_t)(buf[0] | buf[1] << 8);
		data->y = (int16_t)(buf[2] | buf[3] << 8);
		data->z = (int16_t)(buf[4] | buf[5] << 8);
		data->sensType = sensorType;
	
		if( (icm_dev.fifo_highres_enabled)&&(icm_dev.fifo_mode_en) )
		{
			if( ACC==sensorType )
				high_res_rel_addr = FIFO_20BYTES_PACKET_SIZE-FIFO_ACCEL_GYRO_HIGH_RES_SIZE-FIFO_HEADER_SIZE; /*20-3-1 =16; relative to Accel X. */
			else
				high_res_rel_addr = FIFO_GYRO_DATA_SIZE+FIFO_TEMP_DATA_SIZE+FIFO_TEMP_HIGH_RES_SIZE+FIFO_TIMESTAMP_SIZE; /*6+1+1+2 =10; relative to Gyro X. */
	
			data->high_res[0] = buf[high_res_rel_addr]; 
			data->high_res[1] = buf[high_res_rel_addr+1];
			data->high_res[2] = buf[high_res_rel_addr+2];	
	
			if( ACC==sensorType )
			{
				for (i=0; i<3; i++)
					data->high_res[i] = (data->high_res[i] & 0xF0 )>>4;
			}
			else
			{
				for (i=0; i<3; i++)
					data->high_res[i] &= 0x0F;
			}
		}
	} 
}

static int inv_icm4x6xx_convert_rawdata(struct accGyroDataPacket *packet)
{
    int ret = 0;
    uint32_t i = 0;
    uint8_t accEventSize = 0;
    uint8_t gyroEventSize = 0;
    uint8_t accEventSize_Discard = 0;
    uint8_t gyroEventSize_Discard = 0;
    uint64_t tick_ts = 0;
    
    struct accGyroData *data = packet->outBuf;

    if (true == icm_dev.fifo_mode_en && false == icm_dev.polling_data_en) { //fifo mode
        for (i = 0; i < icm_dev.fifoDataToRead; i += icm_dev.fifo_package_size) {
            //INV_LOG(SENSOR_LOG_LEVEL, "Fifo head format is 0x%x", icm_dev.dataBuf[i]);
            if ((accEventSize + gyroEventSize) < MAX_RECV_PACKET*2) {

                #if SENSOR_TIMESTAMP_SUPPORT
	            // Get timestamp from sensor register.
	            if ((true == icm_dev.sensors[ACC].configed) ||
                    (true == icm_dev.sensors[GYR].configed)){

                    //inv_imu_parse_rawdata(&(icm_dev.dataBuf[i + FIFO_TEMP_DATA_SHIFT]),FIFO_TEMP_DATA_SHIFT, TEMP, &data[accEventSize + gyroEventSize]);
                    //temper = data[accEventSize+gyroEventSize].temperature;      // Copy temperature data to gyro record
                    // data[accEventSize+gyroEventSize-2].temperature = data[accEventSize+gyroEventSize-1].temperature;
                    inv_icm4x6xx_parse_rawdata(&(icm_dev.dataBuf[i + FIFO_TIMESTAMP_DATA_SHIFT]),TS, &data[accEventSize + gyroEventSize]);
                    // Copy TS data to gyro record
                    tick_ts = data[accEventSize + gyroEventSize].timeStamp;
                }
	            #endif
                if ((true == icm_dev.sensors[ACC].powered) &&
                    (true == icm_dev.sensors[ACC].configed)) {
                    if (icm_dev.sensors[ACC].samplesToDiscard) {
                        icm_dev.sensors[ACC].samplesToDiscard--;
                        accEventSize_Discard++;
                    } else {
                        inv_icm4x6xx_parse_rawdata(&(icm_dev.dataBuf[i + FIFO_ACCEL_DATA_SHIFT]), ACC, &data[accEventSize + gyroEventSize]);
                        data[accEventSize + gyroEventSize].timeStamp = tick_ts;
                        accEventSize++;
                    }
                }
                if ((true == icm_dev.sensors[GYR].powered) &&
                    (true == icm_dev.sensors[GYR].configed)) {
                    if (icm_dev.sensors[GYR].samplesToDiscard) {
                        icm_dev.sensors[GYR].samplesToDiscard--;
                        gyroEventSize_Discard++;
                    } else {
                        inv_icm4x6xx_parse_rawdata(&(icm_dev.dataBuf[i + FIFO_GYRO_DATA_SHIFT]), GYR, &data[accEventSize + gyroEventSize]);
                        data[accEventSize + gyroEventSize].timeStamp = tick_ts;
                        gyroEventSize++;
                    }
                }
                if ((true == icm_dev.sensors[ACC].configed) ||
                    (true == icm_dev.sensors[GYR].configed))
                    inv_icm4x6xx_parse_rawdata(&(icm_dev.dataBuf[i + FIFO_TEMP_DATA_SHIFT]), TEMP, &data[accEventSize + gyroEventSize-1]);

                // Copy temperature data to ACC record
                data[accEventSize+gyroEventSize-2].temperature = data[accEventSize+gyroEventSize-1].temperature;
            } else {
                INV_LOG(INV_LOG_LEVEL_ERROR, "outBuf full, accEventSize = %d, gyroEventSize = %d", accEventSize, gyroEventSize);
                ret = FIFO_DATA_FULL_ERROR;
            }
        }
    } else { //dri mode or polling mode 
        if ((true == icm_dev.sensors[ACC].configed) &&
            (true == icm_dev.sensors[ACC].powered)) {
            if (icm_dev.sensors[ACC].samplesToDiscard) {
                icm_dev.sensors[ACC].samplesToDiscard--;
                accEventSize_Discard++;
            } else {
                inv_icm4x6xx_parse_rawdata(&(icm_dev.dataBuf[DRI_ACCEL_DATA_SHIFT]), ACC, &data[accEventSize + gyroEventSize]);
                data[accEventSize + gyroEventSize].timeStamp = tick_ts;
                accEventSize++;
            }
        }
        if ((true == icm_dev.sensors[GYR].configed) &&
            (true == icm_dev.sensors[GYR].powered)) {
            if (icm_dev.sensors[GYR].samplesToDiscard) {
                icm_dev.sensors[GYR].samplesToDiscard--;
                gyroEventSize_Discard++;
            } else {
                inv_icm4x6xx_parse_rawdata(&(icm_dev.dataBuf[DRI_GYRO_DATA_SHIFT]), GYR, &data[accEventSize + gyroEventSize]);
                data[accEventSize + gyroEventSize].timeStamp = tick_ts;
                gyroEventSize++;
            }
        }
        if ((true == icm_dev.sensors[ACC].configed) ||
            (true == icm_dev.sensors[GYR].configed))
        {
            inv_icm4x6xx_parse_rawdata(&(icm_dev.dataBuf[0]), TEMP, &data[accEventSize + gyroEventSize-1]);

            // Copy temperature data to ACC record
            data[accEventSize+gyroEventSize-2].temperature = data[accEventSize+gyroEventSize-1].temperature;
        }
    }

    packet->accOutSize = accEventSize;
    packet->gyroOutSize = gyroEventSize;
    packet->temperature = icm_dev.chip_temper;
    packet->timeStamp = 0;
    
    return ret;
}

int inv_icm4x6xx_read_rawdata()
{
    int ret = 0;
    
    ret += inv_read(REG_TEMP_DATA1, icm_dev.dri_package_size, icm_dev.dataBuf);
    
    return ret;
}

#if POLLING_MODE_SUPPORT
int inv_icm4x6xx_polling_rawdata(struct accGyroDataPacket *dataPacket)
{
    int ret = 0;

	icm_dev.dri_package_size = DRI_14BYTES_PACKET_SIZE;
	icm_dev.polling_data_en = true;  // enable polling flag when polling data
	
	if (dataPacket == NULL) {
        INV_LOG(INV_LOG_LEVEL_ERROR, "DATAPACKET NULL POINTER!!");
        exit(-1);
    }

	ret += inv_icm4x6xx_read_rawdata();

	if (ret != 0) {
        INV_LOG(INV_LOG_LEVEL_ERROR, "Read Raw Data Error %d", ret);
        return DRDY_DATA_READ_ERROR;
        }
    ret += inv_icm4x6xx_convert_rawdata(dataPacket);

	if (ret != 0) {
        INV_LOG(INV_LOG_LEVEL_ERROR, "Convert Raw Data Error %d", ret);
        return DRDY_DATA_CONVERT_ERROR;
        }
	icm_dev.polling_data_en = false;  // disable polling flag when finish polling data
	return ret;
}
#endif

int inv_icm4x6xx_get_rawdata_interrupt(struct accGyroDataPacket *dataPacket)
{
    if (dataPacket == NULL) {
        INV_LOG(INV_LOG_LEVEL_ERROR, "DATAPACKET NULL POINTER!!");
        exit(-1);
    }
    uint8_t int_status;
    int ret = 0;
	bool pre_polling_mode = icm_dev.polling_data_en;
	
    ret += inv_read(REG_INT_STATUS, 1, &int_status);
    //INV_LOG(SENSOR_LOG_LEVEL, "INT STATUS 0x%x", int_status);
    if (ret != 0)
        return INT_STATUS_READ_ERROR;
    
    if ((int_status & BIT_INT_DRDY) && (false == icm_dev.fifo_mode_en)) {
        INV_LOG(SENSOR_LOG_LEVEL, "DRDY INT Detected");
        if ((false == icm_dev.sensors[ACC].configed) &&
            (false == icm_dev.sensors[GYR].configed)) {
            INV_LOG(INV_LOG_LEVEL_ERROR, "Unexpected DRDY INTR fired");
            return DRDY_UNEXPECT_INT_ERROR;
        }
        ret += inv_icm4x6xx_read_rawdata();
        if (ret != 0) {
            INV_LOG(INV_LOG_LEVEL_ERROR, "Read Raw Data Error %d", ret);
            return DRDY_DATA_READ_ERROR;
        }
        ret += inv_icm4x6xx_convert_rawdata(dataPacket);
        if (ret != 0) {
            INV_LOG(INV_LOG_LEVEL_ERROR, "Convert Raw Data Error %d", ret);
            return DRDY_DATA_CONVERT_ERROR;
        }
    }
    
    if (int_status & BIT_INT_FIFO_FULL) {
        INV_LOG(INV_LOG_LEVEL_ERROR, "FIFO Overflow!!!");
        // reset fifo
        ret += inv_write(REG_FIFO_CONFIG, BIT_FIFO_MODE_CTRL_BYPASS);
        //  set fifo stream mode
        ret += inv_write(REG_FIFO_CONFIG, BIT_FIFO_MODE_CTRL_STREAM);
        return FIFO_OVERFLOW_ERROR;
    } else if (int_status & BIT_INT_FIFO_WM) {
		
		icm_dev.polling_data_en = false;  // disable polling flag when operate fifo data
		
        //INV_LOG(SENSOR_LOG_LEVEL, "WM INT Detected");
        if ((false == icm_dev.sensors[ACC].configed) &&
            (false == icm_dev.sensors[GYR].configed)) {
            //INV_LOG(SENSOR_LOG_LEVEL, "Unexpected FIFO WM INTR fired");
            //reset fifo
            ret += inv_write(REG_FIFO_CONFIG, BIT_FIFO_MODE_CTRL_BYPASS);
            return FIFO_UNEXPECT_WM_ERROR;
        }
        ret += inv_icm4x6xx_read_fifo();
        if (ret != 0) {
            INV_LOG(INV_LOG_LEVEL_ERROR, "Fifo Data Read Error %d", ret);
            return FIFO_DATA_READ_ERROR;
        }
        ret += inv_icm4x6xx_convert_rawdata(dataPacket);
		icm_dev.polling_data_en = pre_polling_mode;  //recover pre polling status
		
        if (ret != 0) {
            INV_LOG(INV_LOG_LEVEL_ERROR, "Fifo Data Convert Error %d", ret);
            return FIFO_DATA_CONVERT_ERROR;
        }
    }
    /* else: FIFO threshold was not reached and FIFO was not full */
    
    return ret;
}

void inv_apply_mounting_matrix(int32_t raw[3])
{
	int32_t data[3];
	int i=0;

	for (i=0; i<3; i++)
		data[icm_dev.cvt.axis[i]] = icm_dev.cvt.sign[i] * raw[i];

	for (i=0; i<3; i++)
		raw[i]= data[i];
}

#if SUPPORT_SELFTEST
static int inv_icm4x6xx_selftest_init()
{
    int ret = 0;
    uint8_t data = 0;
    
    INV_LOG(SENSOR_LOG_LEVEL, "%s", __func__);

    if (false == icm_dev.init_cfg) {
        inv_icm4x6xx_get_whoami();

        if (icm_dev.product == UNVALID_TYPE)
            return SENSOR_WHOAMI_INVALID_ERROR;
        
        ret += inv_icm4x6xx_reset_check();
        
        /* en byte mode & little endian mode & disable spi or i2c interface */
        if (SPI_MODE_EN) {
            INV_LOG(SENSOR_LOG_LEVEL, "SPI BUS");
            data = (uint8_t)(I2C_SLEW_RATE_20_60NS | SPI_SLEW_RATE_2NS);
            ret += inv_write(REG_DRIVE_CONFIG, data);
            
            ret += inv_write(REG_BANK_SEL, 1);
            data = inv_read(REG_INTF_CONFIG6, 1, &data);
            data |= (uint8_t)(BIT_I3C_SDR_EN | BIT_I3C_DDR_EN);
            ret += inv_write(REG_INTF_CONFIG6, data);
            ret += inv_write(REG_BANK_SEL, 0);
            
            data = (uint8_t)(BIT_UI_SIFS_CFG_I2C_DIS | BIT_FIFO_HOLD_LAST_DATA_EN);
        } else {
            INV_LOG(SENSOR_LOG_LEVEL, "I2C BUS");
            ret += inv_write(REG_BANK_SEL, 1);
            data = inv_read(REG_INTF_CONFIG6, 1, &data);
            data &= (uint8_t)~(BIT_I3C_SDR_EN | BIT_I3C_DDR_EN);
            ret += inv_write(REG_INTF_CONFIG6, data);
            ret += inv_write(REG_BANK_SEL, 0);

            data = (uint8_t)(BIT_UI_SIFS_CFG_SPI_DIS | BIT_FIFO_HOLD_LAST_DATA_EN);
        }
        ret += inv_write(REG_INTF_CONFIG0, data);
    }

    /* gyr odr & fs  250 dps 1khz */
    /* why not (uint8_t)(ODR_1KHZ | GYRO_RANGE_250DPS) 
       to fix compile error on some mcu */
    icm_dev.gyro_cfg0 = (uint8_t)(ODR_1KHZ) | (uint8_t)(ST_GYRO_FSR);
    icm_dev.acc_cfg0 = (uint8_t)(ODR_1KHZ) | (uint8_t)(ST_ACCEL_FSR);    
    ret += inv_write(REG_GYRO_CONFIG0, icm_dev.gyro_cfg0);
    ret += inv_write(REG_ACCEL_CONFIG0, icm_dev.acc_cfg0);
    /* acc & gyro BW  both BW = ODR/10 */
    data = (uint8_t)((BW_ODR_DIV_10 << BIT_ACCEL_FILT_BW_SHIFT) | BW_ODR_DIV_10);
    ret += inv_write(REG_GYRO_ACCEL_CONFIG0, data);
    /* acc & gyro UI filter @ 3th order */
    data = (uint8_t)(BIT_GYRO_AVG_FILT_8K_HZ | (THIRD_ORDER << BIT_GYRO_FILT_ORD_SHIFT));
    ret += inv_write(REG_GYRO_CONFIG1, data);
    data = (uint8_t)(BIT_ACCEL_AVG_FILT_8K_HZ | (THIRD_ORDER << BIT_ACCEL_FILT_ORD_SHIFT));
    ret += inv_write(REG_ACCEL_CONFIG1, data);

    /* async reset */
    ret += inv_write(REG_INT_CONFIG1, (uint8_t)BIT_INT_ASY_RST_DIS); //The field int_asy_rst_disable must be 0 for icm4x6xx

    //reset selftest value flag
    memset(icm_dev.avg, 0, sizeof(icm_dev.avg));
    memset(icm_dev.acc_avg, 0, sizeof(icm_dev.acc_avg));
    memset(icm_dev.acc_avg_st, 0, sizeof(icm_dev.acc_avg_st));
    memset(icm_dev.gyro_avg, 0, sizeof(icm_dev.gyro_avg));
    memset(icm_dev.gyro_avg_st, 0, sizeof(icm_dev.gyro_avg_st));
    
    return ret;
}

static int inv_icm4x6xx_selftest_read_rawdata( uint8_t data_size )
{
    int i=0, j=0, ret = 0;
    int sum[AXES_NUM] ={0};
    int16_t raw_data[AXES_NUM] = {0};
    int sample_read = 0; /* Number of sample read */
	int sample_discarded = 0; /* Number of sample discarded */ 
    int timeout = 300;

    do
    {
		uint8_t int_status;
		ret += inv_read(REG_INT_STATUS, 1, &int_status);
		
		if (int_status & BIT_INT_STATUS_DRDY) {
			int16_t sensor_data[3] = {0}; 

			/* Read data */
			ret += inv_read( icm_dev.st_rawdata_reg, data_size, icm_dev.dataBuf);

			if (ret != 0)
			{
		        INV_LOG(INV_LOG_LEVEL_ERROR, "inv_icm4x6xx_selftest_read_rawdata Error!!");
		        return ret;
			}
			
			/* Use little endian mode */
			raw_data[AXIS_X] = (icm_dev.dataBuf[0] | icm_dev.dataBuf[1] << 8);
			raw_data[AXIS_Y] = (icm_dev.dataBuf[2] | icm_dev.dataBuf[3] << 8);
			raw_data[AXIS_Z] = (icm_dev.dataBuf[4] | icm_dev.dataBuf[5] << 8);

			if ( (raw_data[0]!= -32768 )&& (raw_data[1] != -32768) && (raw_data[2] != -32768) )
			{
				for (j = 0; j < AXES_NUM; j++)				
					sum[j] += raw_data[j];
			}
			else
				sample_discarded++;

			sample_read++;
		}
		
	    // Delay 1ms to wait for new sample.
        inv_delay_ms(1);

		timeout--;
	} while((sample_read < SELFTEST_SAMPLES_NUMBER) && (timeout > 0));
		
    /* Get samples' average on all axes */
    sample_read -= sample_discarded;
    for (j = 0; j < AXES_NUM; j++) {
        sum[j] = sum[j] / sample_read;
        icm_dev.avg[j] = sum[j];
    }

    return ret;
}

static bool inv_icm4x6xx_selftest_acc_result()
{
    bool test_result = true;

    uint32_t st_response =0;
    uint32_t st_otp[3] = {0};
    int i=0;

    for ( i = 0; i < 3; i++) {
    	int fs_sel = ST_ACCEL_FSR >> BIT_ACCEL_FSR_SHIFT;
        st_otp[i] = INV_ST_OTP_EQUATION(fs_sel, icm_dev.acc_st_code[i]);
    }
 
    INV_LOG(SENSOR_LOG_LEVEL, "accel st_otp %d %d %d",
        st_otp[0], st_otp[1], st_otp[2]);

    /* Check selftest result according AN-000150 */
    for ( i = 0; i < 3; i++) {
        st_response = abs(icm_dev.acc_avg_st[i] - icm_dev.acc_avg[i]);
        INV_LOG(SENSOR_LOG_LEVEL, "accel st_response %d st_otp %d", st_response, st_otp[i]);
        if( st_otp[i] ==0 )
        {
        	test_result= false;
	        break;
        }
        
        if ( icm_dev.acc_st_code[i]!=0 ) {
            float ratio = ((float)st_response) / ((float)st_otp[i]);
            if ((ratio >= 1.5f) || (ratio <= 0.5f)) {
                INV_LOG(INV_LOG_LEVEL_ERROR, "accel st test fail, ratio %d", ratio);
                test_result = false;
                break;
            }
        }else{
            /* compare 50mg, 1200mg to LSB according AN-000150
            * convert 50msg, 1200mg to LSB with fsr +/-2g */
            if ((st_response < ((50 * 32768 / 2) / 1000)) ||
                (st_response > ((1200 * 32768 / 2) / 1000))) {
                INV_LOG(INV_LOG_LEVEL_ERROR, "accel st test fail, st_response %d",st_response);
                test_result = false;
                break;
            }
        }
    }

    INV_LOG(SENSOR_LOG_LEVEL, "Acc SelfTest Result: %d", test_result);
    
    return test_result;
}

static bool inv_icm4x6xx_selftest_gyro_result()
{
    bool test_result = true;
    uint32_t st_response = 0;
    uint32_t st_otp[3] = {0};
    int i=0;

	uint32_t gyro_sensitivity_1dps = 32768 / 250;  // FSR: 250 dps;

    for ( i = 0; i < 3; i++) {
        int fs_sel = ST_GYRO_FSR >> BIT_GYRO_FSR_SHIFT;
        st_otp[i] = INV_ST_OTP_EQUATION(fs_sel, icm_dev.gyro_st_code[i]);
    }
 
    INV_LOG(SENSOR_LOG_LEVEL, "gyro st_otp %d %d %d", 
        st_otp[0], st_otp[1], st_otp[2]);

    /* Check selftest result according AN-000150 */
    for ( i = 0; i < 3; i++) {
        st_response = abs(icm_dev.gyro_avg_st[i] - icm_dev.gyro_avg[i]);
        INV_LOG(SENSOR_LOG_LEVEL, "gyro st_response %d st_otp %d", st_response, st_otp[i]);

        if( st_otp[i] ==0 )
        {
        	test_result= false;
	        break;
        }
        
        if (icm_dev.gyro_st_code[i]!=0) {
            float ratio = ((float)st_response) / ((float)st_otp[i]);
            if (ratio <= 0.5f)	{
                INV_LOG(INV_LOG_LEVEL_ERROR, "gyro st test fail, ratio %d", ratio);
                test_result = false;
                break;
            }
        } else {
            /* compare 60dps with LSB according AN-000150
             * convert 60dps to LSB with fsr +/-250dps */
            if (st_response < (60 * gyro_sensitivity_1dps)) {
                INV_LOG(SENSOR_LOG_LEVEL, "gyro st test fail, st_response %d", st_response);
                test_result = false;
                break;
             }
        }
    }

	for (i = 0; i < 3; i++) {
		if (abs(icm_dev.gyro_avg[i]) > (int)(20 * gyro_sensitivity_1dps))
			test_result = false;
	}

    INV_LOG(SENSOR_LOG_LEVEL, "Gyro SelfTest Result: %d", test_result);
    
    return test_result;
}

static int inv_icm4x6xx_selftest_recover()
{
    int odr_index = 0;
    uint32_t sampleRate = 0;
    uint32_t maxRate = 0;
    int ret = 0;

    INV_LOG(SENSOR_LOG_LEVEL, "%s", __func__);

    if (true == icm_dev.init_cfg) {
        INV_LOG(SENSOR_LOG_LEVEL, "Selftest: Re-init configuration");
        ret += inv_icm4x6xx_init_config();
    }
    
    if (false == icm_dev.sensors[ACC].powered && 
        false == icm_dev.sensors[GYR].powered
        #if SUPPORT_PEDOMETER
        && (false == icm_dev.sensors[PEDO].powered)
        #endif
        #if SUPPORT_WOM
        && (false == icm_dev.sensors[WOM].powered)
        #endif
        ) {
        INV_LOG(SENSOR_LOG_LEVEL, "Selftest: no sensor need recover");
        return ret;
    }

    if (true == icm_dev.sensors[ACC].configed ||
        true == icm_dev.sensors[GYR].configed) {
        //2st recover odr 
        INV_LOG(SENSOR_LOG_LEVEL, "AcchwRate %f Hz GyrhwRate %f Hz",
        icm_dev.sensors[ACC].hwRate/1024.0f, icm_dev.sensors[GYR].hwRate/1024.0f);
    
        if (true == icm_dev.sensors[ACC].configed &&
            true == icm_dev.sensors[GYR].configed) {
            INV_LOG(SENSOR_LOG_LEVEL, "recover sensor A&G application");
            maxRate = max( icm_dev.sensors[ACC].hwRate, icm_dev.sensors[GYR].hwRate);
            icm_dev.sensors[ACC].hwRate = maxRate;
            icm_dev.sensors[GYR].hwRate = maxRate;  
        } else if (true == icm_dev.sensors[ACC].configed) {
            INV_LOG(SENSOR_LOG_LEVEL, "recover sensor Acc application");
            maxRate = icm_dev.sensors[ACC].hwRate;
        } else if (true == icm_dev.sensors[GYR].configed) {
            INV_LOG(SENSOR_LOG_LEVEL, "recover sensor Gyro application\n");
            maxRate = icm_dev.sensors[GYR].hwRate;
        } 

        odr_index = inv_icm4x6xx_cal_odr(&maxRate, &sampleRate);
        ret += inv_icm4x6xx_set_odr(odr_index);
        if (true == icm_dev.fifo_mode_en)
            ret += inv_icm4x6xx_config_fifo(true); 
        else
            ret += inv_icm4x6xx_config_drdy(true);
    }

    //1st reopen sensor 
    if (true == icm_dev.sensors[ACC].powered) {
        icm_dev.pwr_sta &= (uint8_t)~BIT_ACCEL_MODE_MASK;
        icm_dev.pwr_sta |= BIT_ACCEL_MODE_LN;
        ret += inv_write(REG_PWR_MGMT0, icm_dev.pwr_sta);
        //200us here to sync fly changes
        inv_delay_us(200);
    }

    if (true == icm_dev.sensors[GYR].powered) {    
        icm_dev.pwr_sta &= (uint8_t)~BIT_GYRO_MODE_MASK;
        icm_dev.pwr_sta |= BIT_GYRO_MODE_LN;
        ret += inv_write(REG_PWR_MGMT0, icm_dev.pwr_sta);
        //200us here to sync fly changes
        inv_delay_us(200);
    }

    #if SUPPORT_PEDOMETER
    if (true == icm_dev.sensors[PEDO].powered) {
        INV_LOG(SENSOR_LOG_LEVEL, "Re-enable Pedo");
        ret += inv_icm4x6xx_pedometer_enable();
    }
    #endif
    #if SUPPORT_WOM
    if (true == icm_dev.sensors[WOM].powered) {
        INV_LOG(SENSOR_LOG_LEVEL, "Re-enable WOM");
        ret += inv_icm4x6xx_wom_enable();
    }
    #endif
    
    return ret;
}

int inv_icm4x6xx_acc_selftest(bool *result)
{
    if (result == NULL) {
        INV_LOG(INV_LOG_LEVEL_ERROR, "RESULT NULL POINTER!!");
        exit(-1);
    }
    
    int ret = 0;
	uint8_t data_size= ACCEL_DATA_SIZE;

    INV_LOG(SENSOR_LOG_LEVEL, "%s", __func__);
    
    ret += inv_icm4x6xx_selftest_init();
    if (ret != 0)
        return SELFTEST_INIT_ERROR;
    
    icm_dev.pwr_sta &= (uint8_t)~BIT_ACCEL_MODE_MASK;
    icm_dev.pwr_sta |= (uint8_t)BIT_ACCEL_MODE_LN;
    //set 100 ms for accel start-up time in selftest mode
    ret += inv_write(REG_PWR_MGMT0, icm_dev.pwr_sta);
    inv_delay_ms(100);

    if (ret != 0)
        return SELFTEST_ACC_ENABLE_ERROR;

    INV_LOG(SENSOR_LOG_LEVEL, "Selftest Read acc");
    icm_dev.st_rawdata_reg = REG_ACCEL_DATA_X1;

    ret += inv_icm4x6xx_selftest_read_rawdata( data_size );
    if (ret != 0)
        return SELFTEST_ACC_READ_ERROR;

    icm_dev.acc_avg[0] = icm_dev.avg[0];
    icm_dev.acc_avg[1] = icm_dev.avg[1];
    icm_dev.acc_avg[2] = icm_dev.avg[2];

    INV_LOG(SENSOR_LOG_LEVEL, "acc avg %d %d %d",
        icm_dev.acc_avg[0], icm_dev.acc_avg[1], icm_dev.acc_avg[2]);

    INV_LOG(SENSOR_LOG_LEVEL, "Selftest Read acc In SelfTest Mode");
    //enable selftest mode 
    icm_dev.accel_config = (uint8_t)(BIT_ACCEL_ST_POWER_EN | BIT_ACCEL_ST_EN_MASK);
    ret += inv_write(REG_SELF_TEST_CONFIG, icm_dev.accel_config);  //delay 200ms
    inv_delay_ms(200);

    ret += inv_icm4x6xx_selftest_read_rawdata( data_size );

    if (ret != 0)
        return SELFTEST_ACC_ST_READ_ERROR;
    
    icm_dev.acc_avg_st[0] = icm_dev.avg[0];
    icm_dev.acc_avg_st[1] = icm_dev.avg[1];
    icm_dev.acc_avg_st[2] = icm_dev.avg[2];

    INV_LOG(SENSOR_LOG_LEVEL, "acc_avg_st %d %d %d",
        icm_dev.acc_avg_st[0], icm_dev.acc_avg_st[1], icm_dev.acc_avg_st[2]);

    ret += inv_write(REG_BANK_SEL, 2);
    /* read acc st code in bank 2 */
    ret += inv_read(REG_XA_ST_DATA, AXES_NUM, icm_dev.acc_st_code);
    ret += inv_write(REG_BANK_SEL, 0);
    if (ret != 0)
        return SELFTEST_ACC_STCODE_READ_ERROR;
    
    INV_LOG(SENSOR_LOG_LEVEL, "acc chip_st_code %d %d %d", 
        icm_dev.acc_st_code[0], icm_dev.acc_st_code[1], icm_dev.acc_st_code[2]);
    
    *result = inv_icm4x6xx_selftest_acc_result();

    ret += inv_write(REG_SELF_TEST_CONFIG, (uint8_t)~BIT_ACCEL_ST_POWER_EN);
    icm_dev.pwr_sta &= (uint8_t)~BIT_ACCEL_MODE_MASK;
    ret += inv_write(REG_PWR_MGMT0, icm_dev.pwr_sta);
    inv_delay_us(200);
    
    int recover = 0;
    recover += inv_icm4x6xx_selftest_recover();
    if (recover != 0)
        INV_LOG(INV_LOG_LEVEL_ERROR, "Acc Selftest: Recover Failed %d", recover);
    
    return ret;
}

int inv_icm4x6xx_gyro_selftest(bool *result)
{
    if (result == NULL) {
        INV_LOG(INV_LOG_LEVEL_ERROR, "RESULT NULL POINTER!!");
        exit(-1);
    }
    
    int ret = 0;
	uint8_t data_size= GYRO_DATA_SIZE;

    INV_LOG(SENSOR_LOG_LEVEL, "%s", __func__);
    
    ret += inv_icm4x6xx_selftest_init();
    if (ret != 0)
        return SELFTEST_INIT_ERROR;
    
    icm_dev.pwr_sta &= (uint8_t)~BIT_GYRO_MODE_MASK;
    icm_dev.pwr_sta |= (uint8_t)BIT_GYRO_MODE_LN;
    // set 100 ms for gyro start-up time in selftest mode
    ret += inv_write(REG_PWR_MGMT0, icm_dev.pwr_sta);
    inv_delay_ms(100);

    if (ret != 0)
        return SELFTEST_GYR_ENABLE_ERROR;

    INV_LOG(SENSOR_LOG_LEVEL, "Selftest Read gyro");
    icm_dev.st_rawdata_reg = REG_GYRO_DATA_X1;

    ret += inv_icm4x6xx_selftest_read_rawdata( data_size );
    if (ret != 0)
        return SELFTEST_GYR_READ_ERROR;

    icm_dev.gyro_avg[0] = icm_dev.avg[0];
    icm_dev.gyro_avg[1] = icm_dev.avg[1];
    icm_dev.gyro_avg[2] = icm_dev.avg[2];

    INV_LOG(SENSOR_LOG_LEVEL, "gyro avg %d %d %d", 
        icm_dev.gyro_avg[0], icm_dev.gyro_avg[1], icm_dev.gyro_avg[2]);

    INV_LOG(SENSOR_LOG_LEVEL, "Selftest Read gyro In SelfTest Mode");
    //enable selftest mode of gyro 
    icm_dev.gyro_config = (uint8_t)BIT_GYRO_ST_EN_MASK;
    ret += inv_write(REG_SELF_TEST_CONFIG, icm_dev.gyro_config);    //delay 200ms
    inv_delay_ms(200);

    ret += inv_icm4x6xx_selftest_read_rawdata( data_size );

    if (ret != 0)
        return SELFTEST_GYR_ST_READ_ERROR;
    
    icm_dev.gyro_avg_st[0] = icm_dev.avg[0];
    icm_dev.gyro_avg_st[1] = icm_dev.avg[1];
    icm_dev.gyro_avg_st[2] = icm_dev.avg[2];

    INV_LOG(SENSOR_LOG_LEVEL, "gyro_avg_st %d %d %d",
        icm_dev.gyro_avg_st[0], icm_dev.gyro_avg_st[1], icm_dev.gyro_avg_st[2]);

    ret += inv_write(REG_BANK_SEL, 1);
    /* read gyro st code in bank 1 */
    ret += inv_read(REG_XG_ST_DATA, AXES_NUM, icm_dev.gyro_st_code);
    ret += inv_write(REG_BANK_SEL, 0);
    
    if (ret != 0)
        return SELFTEST_GYR_STCODE_READ_ERROR;
    
    INV_LOG(SENSOR_LOG_LEVEL, "gyro chip_st_code %d %d %d", 
        icm_dev.gyro_st_code[0], icm_dev.gyro_st_code[1], icm_dev.gyro_st_code[2]);
    
    *result = inv_icm4x6xx_selftest_gyro_result();
    
	ret += inv_write(REG_SELF_TEST_CONFIG, (uint8_t)~BIT_GYRO_ST_EN_MASK);
    icm_dev.pwr_sta &= (uint8_t)~BIT_GYRO_MODE_MASK;
    ret += inv_write(REG_PWR_MGMT0, icm_dev.pwr_sta);
    inv_delay_us(200);
    
    int recover = 0;
    recover += inv_icm4x6xx_selftest_recover();
    if (recover != 0)
        INV_LOG(INV_LOG_LEVEL_ERROR, "Gyro Selftest: Recover Failed %d", recover);

    return ret;
}
#endif

#if SUPPORT_PEDOMETER
static int inv_icm4x6xx_dmp_reset()
{
    int ret = 0;
    uint8_t data = 0;
#if SUPPORT_DELAY_US
    int timeout = 5000; /*50ms*/
#else
    int timeout = 50;   /*50ms*/
#endif

    ret += inv_read(REG_SIGNAL_PATH_RESET, 1, &data);
    data |= (uint8_t)BIT_DMP_MEM_RESET_EN;
    ret += inv_write(REG_SIGNAL_PATH_RESET, data);

    /* Make sure reset procedure has finished by reading back mem_reset_en bit */
    do {
        ret += inv_read(REG_SIGNAL_PATH_RESET, 1, &data);
        inv_delay_us(10);
    } while ((data & BIT_DMP_MEM_RESET_EN) && timeout--);

    if (timeout <= 0)
        return DMP_TIMEOUT_ERROR;

    return ret;
}

static int inv_icm4x6xx_dmp_enable()
{
    int ret = 0;
    uint8_t data = 0;
#if SUPPORT_DELAY_US
	int timeout = 5000; /*50ms*/
#else
	int timeout = 50;	/*50ms*/
#endif

    ret += inv_read(REG_SIGNAL_PATH_RESET, 1, &data);
    data |= (uint8_t)BIT_DMP_INIT_EN;
    ret += inv_write(REG_SIGNAL_PATH_RESET, data);

    inv_delay_ms(5);

    /* DMP INIT is updated every accel ODR Wait for DMP idle */
	do {
	    ret += inv_read(REG_APEX_DATA3, 1, &data);
        inv_delay_us(10);
	} while ((0 == (data & BIT_DMP_IDLE)) && timeout--);

	if (timeout <= 0)
		return DMP_TIMEOUT_ERROR;

    return ret;
}

static int inv_icm4x6xx_dmp_set_odr()
{
    int ret = 0;
    uint8_t data = 0;

    /* dmp odr */
    ret += inv_read(REG_APEX_CONFIG0, 1, &data);
    if (true == icm_dev.dmp_power_save) {
        data |= (uint8_t)BIT_DMP_POWER_SAVE_EN;
        INV_LOG(SENSOR_LOG_LEVEL, "DMP POWER SAVE Enable 0x%x", data);
    } else {
        data &= (uint8_t)~BIT_DMP_POWER_SAVE_EN;
        INV_LOG(SENSOR_LOG_LEVEL, "DMP POWER SAVE Disable 0x%x", data);
    }

    data &= (uint8_t)~BIT_DMP_ODR_MASK;
    data |= (uint8_t)BIT_DMP_ODR_50HZ; //set dmp odr 50hz
    ret += inv_write(REG_APEX_CONFIG0, data);   

    return ret;
}

int inv_icm4x6xx_apex_config(SensorType_t index)
{
    int ret = 0;
    uint8_t data = 0;
    
    INV_LOG(SENSOR_LOG_LEVEL, "%s ", __func__);

    /* DMP cannot be configured if it is running, hence make sure all APEX algorithms are off */
#if 0
    //Really need?
    ret += inv_read(REG_APEX_CONFIG0, 1, &data);
    if ((data & BIT_PED_ENABLE_EN) ||
        (data & BIT_LOW_G_EN) ||
        (data & BIT_HIGH_G_EN)) {
        INV_LOG(INV_LOG_LEVEL_ERROR, "APEX Algo already running 0x%x!!", data);
        return DMP_STATUS_ERROR;
    }
#endif

    switch (index) {
    #if SUPPORT_PEDOMETER
    case PEDO:
        {
            INV_LOG(SENSOR_LOG_LEVEL, "APEX Case -- PEDO");
            ret += inv_write(REG_BANK_SEL, 4);
            data = (uint8_t)(LOW_ENERGY_AMP_TH_SEL_2684354MG) | (uint8_t)(DMP_POWER_SAVE_TIME_SEL_8S);
            ret += inv_write(REG_APEX_CONFIG1, data);

            data = (uint8_t)(PEDO_AMP_TH_2080374_MG) | (uint8_t)(PED_STEP_CNT_TH_2_STEP);
            ret += inv_write(REG_APEX_CONFIG2, data);

            data = (uint8_t)(PED_STEP_DET_TH_2_STEP) | (uint8_t)(PEDO_SB_TIMER_TH_150_SAMPLES) | (uint8_t)(PEDO_HI_ENRGY_TH_107);
            ret += inv_write(REG_APEX_CONFIG3, data);

            data = (uint8_t)~BIT_SENSITIVITY_MODE_MASK;
            ret += inv_write(REG_APEX_CONFIG9, data);

            ret += inv_write(REG_BANK_SEL, 0);
        }
        break;
    #endif
    default:
        break;
    }
    
    return ret;
}
#endif

#if SUPPORT_PEDOMETER
int inv_icm4x6xx_pedometer_enable()
{
    int ret = 0;
    uint8_t data = 0;

    INV_LOG(SENSOR_LOG_LEVEL, "%s", __func__);

    // if acc is not in streaming mode enable acc
    if (false == icm_dev.sensors[ACC].powered) {
        icm_dev.pwr_sta &= ~BIT_ACCEL_MODE_MASK;
        icm_dev.pwr_sta |= BIT_ACCEL_MODE_LN;
        ret += inv_write(REG_PWR_MGMT0, icm_dev.pwr_sta);
        inv_delay_us(200);
        INV_LOG(SENSOR_LOG_LEVEL, "pedometer enable, acc on");
    }
    
    /* Make sure Pedo is disabled before init */
    ret += inv_read(REG_APEX_CONFIG0, 1, &data);
    data &= (uint8_t)~BIT_PED_ENABLE_EN;
    ret += inv_write(REG_APEX_CONFIG0, data);

    ret += inv_icm4x6xx_dmp_set_odr();

    /* Initialize APEX Hw */
    ret += inv_icm4x6xx_dmp_reset();

    ret += inv_icm4x6xx_apex_config(PEDO);
    
    ret += inv_icm4x6xx_dmp_enable();
    
    if (true == icm_dev.dmp_power_save) {
        ret += inv_read(REG_SMD_CONFIG, 1, &data);
        data &= (uint8_t)~BIT_SMD_MODE_MASK;
        data |= (uint8_t)(BIT_SMD_MODE_WOM | BIT_WOM_MODE_COMPARE_PRE);
        INV_LOG(SENSOR_LOG_LEVEL, "SMD_CFG is 0x%x", data);
        ret += inv_write(REG_SMD_CONFIG, data);
    }

    ret += inv_write(REG_BANK_SEL, 4);
    ret += inv_read(REG_INT_SOURCE6, 1, &data);     
    data |= (uint8_t)BIT_INT1_STEP_DET_EN;
    ret += inv_write(REG_INT_SOURCE6, data);
    ret += inv_write(REG_BANK_SEL, 0);
    
    ret += inv_read(REG_APEX_CONFIG0, 1, &data);
    data |= (uint8_t)BIT_PED_ENABLE_EN;
    ret += inv_write(REG_APEX_CONFIG0, data);

    if (ret != 0)
        return PEDO_ENABLE_ERROR;

    icm_dev.sensors[PEDO].powered = true;

#if SENSOR_REG_DUMP
    inv_icm4x6xx_dumpRegs();
#endif
    
    return ret;
}

int inv_icm4x6xx_pedometer_disable()
{
    int ret = 0;
    uint8_t data;

    INV_LOG(SENSOR_LOG_LEVEL, "%s", __func__);

    if (true == icm_dev.dmp_power_save) {
        ret += inv_read(REG_SMD_CONFIG, 1, &data);
        data &= (uint8_t)~BIT_SMD_MODE_MASK;
        ret += inv_write(REG_SMD_CONFIG, data);
    }
    
    ret += inv_read(REG_APEX_CONFIG0, 1, &data);
    data &= (uint8_t)~BIT_PED_ENABLE_EN;
    ret += inv_write(REG_APEX_CONFIG0, data);

    if (false == icm_dev.sensors[ACC].powered
        #if SUPPORT_WOM
        && false == icm_dev.sensors[WOM].powered
        #endif
        ) {
        icm_dev.pwr_sta &= ~BIT_ACCEL_MODE_MASK;
        ret += inv_write(REG_PWR_MGMT0, icm_dev.pwr_sta);
        inv_delay_us(200);
        INV_LOG(SENSOR_LOG_LEVEL, "pedometer disable, acc off pwr: 0x%x", icm_dev.pwr_sta);
    }
    
    icm_dev.sensors[PEDO].powered = false;
    
    return ret;
}

int inv_icm4x6xx_pedometer_get_stepCnt(uint64_t *step_cnt)
{
    if (step_cnt == NULL) {
        INV_LOG(INV_LOG_LEVEL_ERROR, "STEP_CNT NULL POINTER!!");
        exit(-1);
    }
    
    int ret = 0;
    uint8_t int_status = 0;
    uint8_t data[2] = {0};
    uint16_t step_data = 0;
    uint8_t step_cadence = 0;
    uint8_t activity_class = 0;
	uint8_t dmpODR=0;

    float cadence_step_per_sec = 0;
    float nb_samples = 0;

    ret += inv_read(REG_INT_STATUS3, 1, &int_status);
    INV_LOG(SENSOR_LOG_LEVEL, "INT STATUS3 0x%x", int_status);
    if (ret != 0)
        return INT_STATUS_READ_ERROR;
    
    if (int_status & BIT_INT_STEP_DET) {
        static uint8_t step_cnt_ovflw = 0;

        if (int_status & BIT_INT_STEP_CNT_OVF) {
            step_cnt_ovflw++;
            INV_LOG(SENSOR_LOG_LEVEL, "step_cnt_ovflw %d", step_cnt_ovflw);
        }
        
        memset(data, 0, sizeof(data));
        ret += inv_read(REG_APEX_DATA0, 2, data);
        step_data = (((uint16_t)data[1]) << 8) | data[0];
        ret += inv_read(REG_APEX_DATA2, 1, &step_cadence);
        ret += inv_read(REG_APEX_DATA3, 1, &activity_class);
    
        /* Converting u6.2 to float */
        nb_samples = (step_cadence >> 2) + (float)(step_cadence & 0x03)*0.25;
        
		ret += inv_read(REG_APEX_CONFIG0, 1, &dmpODR);
		if ( BIT_DMP_ODR_25HZ== (dmpODR & BIT_DMP_ODR_MASK) )
			dmpODR = 25;
		else
			dmpODR = 50;
			
        cadence_step_per_sec = (float)dmpODR / nb_samples;
        
        *step_cnt = step_data + step_cnt_ovflw * 65535;

        activity_class &= BIT_ACTIVITY_CLASS_MASK;

        switch (activity_class) {
        case BIT_ACTIVITY_CLASS_WALK:
            INV_LOG(SENSOR_LOG_LEVEL, "%lld steps - cadence: %.2f steps/sec - WALK", *step_cnt, cadence_step_per_sec);
            break;
        case BIT_ACTIVITY_CLASS_RUN:
            INV_LOG(SENSOR_LOG_LEVEL, "%lld steps - cadence: %.2f steps/sec - RUN", *step_cnt, cadence_step_per_sec);
            break;
        default:
            INV_LOG(SENSOR_LOG_LEVEL, "%lld steps - cadence: %.2f steps/sec - OTHER", *step_cnt, cadence_step_per_sec);
            break;
        }
}

    return ret;
}
#endif

#if SUPPORT_WOM
int inv_icm4x6xx_wom_enable()
{
    int ret = 0;
    uint8_t data = 0;

    // if acc is not in streaming mode enable acc
    if (!icm_dev.sensors[ACC].powered) {
        icm_dev.pwr_sta &= ~BIT_ACCEL_MODE_MASK;
        icm_dev.pwr_sta |= BIT_ACCEL_MODE_LN;
        ret += inv_write(REG_PWR_MGMT0, icm_dev.pwr_sta);
        inv_delay_us(200);
        INV_LOG(SENSOR_LOG_LEVEL, "wom enable, acc on");
    }
    
    /* set X,Y,Z threshold */   
    ret += inv_write(REG_BANK_SEL, 4);// Bank 4
    ret += inv_write(REG_ACCEL_WOM_X_THR, DEFAULT_WOM_THS_MG);
    ret += inv_write(REG_ACCEL_WOM_Y_THR, DEFAULT_WOM_THS_MG);
    ret += inv_write(REG_ACCEL_WOM_Z_THR, DEFAULT_WOM_THS_MG);
    ret += inv_write(REG_BANK_SEL, 0);// Bank 0

    inv_delay_ms(5);
    ret += inv_read(REG_INT_SOURCE1, 1, &data);
    data |= (uint8_t)BIT_INT1_WOM_EN_MASK;
    ret += inv_write(REG_INT_SOURCE1, data);

    inv_delay_ms(60);

    ret += inv_read(REG_SMD_CONFIG, 1, &data);
    data &= (uint8_t)~BIT_SMD_MODE_MASK;
    data |= (uint8_t)BIT_SMD_MODE_WOM;
    data |= (uint8_t)BIT_WOM_MODE_COMPARE_PRE;
    INV_LOG(SENSOR_LOG_LEVEL, "SMD_CFG is 0x%x", data);
    ret += inv_write(REG_SMD_CONFIG, data);
    
    if (ret != 0)
        return WOM_ENABLE_ERROR;

    icm_dev.sensors[WOM].powered = true;

    #if SENSOR_REG_DUMP
    inv_icm4x6xx_dumpRegs();
    #endif
    
    return ret;
}

int inv_icm4x6xx_wom_disable()
{
    int ret = 0;
    uint8_t data = 0;

    INV_LOG(SENSOR_LOG_LEVEL, "%s", __func__);
    
    ret += inv_read(REG_INT_SOURCE1, 1, &data);
    data &= (uint8_t)~BIT_INT1_WOM_EN_MASK;
    ret += inv_write(REG_INT_SOURCE1, data);

    ret += inv_read(REG_SMD_CONFIG, 1, &data);
    data &= (uint8_t)~BIT_SMD_MODE_MASK;
    ret += inv_write(REG_SMD_CONFIG, data);

    if (false == icm_dev.sensors[ACC].powered 
        #if SUPPORT_PEDOMETER
        && false == icm_dev.sensors[PEDO].powered
        #endif
        ) {
        icm_dev.pwr_sta &= ~BIT_ACCEL_MODE_MASK;
        ret += inv_write(REG_PWR_MGMT0, icm_dev.pwr_sta);
        inv_delay_us(200);
        INV_LOG(SENSOR_LOG_LEVEL, "wom disable, acc off pwr: 0x%x", icm_dev.pwr_sta);
    }
    
    icm_dev.sensors[WOM].powered = false;
    
    return ret;
}

int inv_icm4x6xx_wom_get_event(bool *event)
{
    if (event == NULL) {
        INV_LOG(INV_LOG_LEVEL_ERROR, "EVENT NULL POINTER!!");
        exit(-1);
    }
    
    int ret = 0;
    uint8_t int_status = 0;

    ret += inv_read(REG_INT_STATUS2, 1, &int_status);
    INV_LOG(SENSOR_LOG_LEVEL, "INT_STATUS2 0x%x", int_status);
    if (int_status & BIT_INT_WOM_MASK) {
        INV_LOG(SENSOR_LOG_LEVEL, "Motion Detected !!");
        *event = true;
    }
    
    return ret;
}
#endif

#if SENSOR_REG_DUMP
void inv_icm4x6xx_dumpRegs()
{
    uint8_t data = 0;
    uint16_t size = 0;
    uint8_t reg_map_bank_0[] = {
        REG_DRIVE_CONFIG,
        REG_INT_CONFIG,
        REG_PWR_MGMT0,
        REG_GYRO_CONFIG0,
        REG_ACCEL_CONFIG0,
        REG_TMST_CONFIG,
#if (SUPPORT_PEDOMETER | SUPPORT_WOM)
        REG_APEX_CONFIG0,
#endif
        REG_FIFO_CONFIG,
        REG_FIFO_CONFIG_1,
        REG_FIFO_COUNT_H,
        REG_FIFO_COUNT_H + 1,
        REG_FIFO_WM_TH_L,
        REG_FIFO_WM_TH_H,
        REG_INT_STATUS,
        REG_INT_STATUS2,
        REG_INT_STATUS3,
        REG_INT_SOURCE0,
        REG_INT_SOURCE1,
        REG_INT_SOURCE3,
        REG_INT_SOURCE4,
        REG_INT_CONFIG0,
        REG_INT_CONFIG1,
        REG_INTF_CONFIG0,
        REG_INTF_CONFIG1,
        REG_SMD_CONFIG,
        REG_GYRO_ACCEL_CONFIG0
    };

    size = ARRAY_SIZE(reg_map_bank_0);
    for (int i = 0; i < size; i++)
    {
        inv_read(reg_map_bank_0[i], 1, &data);
        INV_LOG(SENSOR_LOG_LEVEL, "bank0 reg[0x%x] 0x%x", reg_map_bank_0[i], data);
    }
    
#if (SUPPORT_PEDOMETER | SUPPORT_WOM)
    /* Apex config */
    uint8_t reg_map_bank_4[] = {
        REG_APEX_CONFIG1,
        REG_APEX_CONFIG2,
        REG_APEX_CONFIG3,
        REG_APEX_CONFIG4,
        REG_APEX_CONFIG5,
        REG_APEX_CONFIG6,
        REG_APEX_CONFIG9,
        REG_ACCEL_WOM_X_THR,
        REG_ACCEL_WOM_Y_THR,
        REG_ACCEL_WOM_Z_THR,
        REG_INT_SOURCE6
    };

    size = ARRAY_SIZE(reg_map_bank_4);
    inv_write(REG_BANK_SEL, 4);
    for (int i = 0; i < size; i++)
    {
        inv_read(reg_map_bank_4[i], 1, &data);
        INV_LOG(SENSOR_LOG_LEVEL, "bank4 reg[0x%x] 0x%x", reg_map_bank_4[i], data);
    }
    inv_write(REG_BANK_SEL, 0);
#endif
}
#endif
