/*
 * as7331.h
 *
 *  Created on: Nov 13, 2023
 *      Author:
 */

/*******************************************************************************************************************//**
 * @addtogroup AS7331
 * @{
 **********************************************************************************************************************/

#ifndef AS7331_H
#define AS7331_H

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/


#if defined(__CCRX__) || defined(__ICCRX__) || defined(__RX__)
 #error "Not supported"
#elif defined(__CCRL__) || defined(__ICCRL__) || defined(__RL78__)
 #error "Not supported"
#else
#include "rm_comms_api.h"
/* Common macro for FSP header files. There is also a corresponding FSP_FOOTER macro at the end of this file. */
FSP_HEADER
#endif

/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#define AS7331_CFG_DEVICE_TYPE (1)


/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/
typedef enum {
    AS7331_CONT_MODE                = 0x00, // continuous mode
    AS7331_CMD_MODE                 = 0x01, // force mode, one-time measurement
    AS7331_SYNS_MODE                = 0x02,
    AS7331_SYND_MODE                = 0x03
} as7331_mode_t;


typedef enum  {
    AS7331_1024           = 0x00, // internal clock frequency, 1.024 MHz, etc
    AS7331_2048           = 0x01,
    AS7331_4096           = 0x02,
    AS7331_8192           = 0x03
} as7331_cclk_t;

/**********************************************************************************************************************
 * Exported global variables
 **********************************************************************************************************************/

/** @endcond */

/**********************************************************************************************************************
 * Public Function Prototypes
 **********************************************************************************************************************/
fsp_err_t AS7331_power_down(const rm_comms_instance_t *  p_i2c);
fsp_err_t AS7331_power_up(const rm_comms_instance_t *  p_i2c);
fsp_err_t AS7331_reset(const rm_comms_instance_t *  p_i2c);
uint8_t AS7331_get_chip_id(const rm_comms_instance_t *  p_i2c);
fsp_err_t AS7331_setConfigurationMode(const rm_comms_instance_t *  p_i2c);
fsp_err_t AS7331_setMeasurementMode(const rm_comms_instance_t *  p_i2c);
fsp_err_t  AS7331_init(const rm_comms_instance_t *  p_i2c, uint8_t mmode, uint8_t cclk, uint8_t sb, uint8_t breakTime, uint8_t gain, uint8_t time);
fsp_err_t AS7331_oneShot(const rm_comms_instance_t *  p_i2c);
fsp_err_t AS7331_getStatus(const rm_comms_instance_t *  p_i2c, uint16_t *p_status);
fsp_err_t AS7331_readAllData(const rm_comms_instance_t *  p_i2c, uint16_t data[4]);
fsp_err_t AS7331_readTempData(const rm_comms_instance_t *  p_i2c, uint16_t *p_data);
fsp_err_t AS7331_readUVAData(const rm_comms_instance_t *  p_i2c, uint16_t *p_data);
fsp_err_t AS7331_readUVBData(const rm_comms_instance_t *  p_i2c, uint16_t *p_data);
fsp_err_t AS7331_readUVCData(const rm_comms_instance_t *  p_i2c, uint16_t *p_data);

#if defined(__CCRX__) || defined(__ICCRX__) || defined(__RX__)
#elif defined(__CCRL__) || defined(__ICCRL__) || defined(__RL78__)
#else

/* Common macro for FSP header files. There is also a corresponding FSP_FOOTER macro at the end of this file. */
FSP_FOOTER
#endif

#endif                                 /* AS7331_H_*/

/*******************************************************************************************************************//**
 * @} (end addtogroup AS7331)
 **********************************************************************************************************************/
