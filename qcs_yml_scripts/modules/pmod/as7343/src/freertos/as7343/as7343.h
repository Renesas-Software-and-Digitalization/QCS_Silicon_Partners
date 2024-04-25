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
#define AS7343_CFG_DEVICE_TYPE (1)


/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/* @brief Allowable gain multipliers for `setGain`
 *
 */
typedef enum {
  AS7343_GAIN_0_5X,
  AS7343_GAIN_1X,
  AS7343_GAIN_2X,
  AS7343_GAIN_4X,
  AS7343_GAIN_8X,
  AS7343_GAIN_16X,
  AS7343_GAIN_32X,
  AS7343_GAIN_64X,
  AS7343_GAIN_128X,
  AS7343_GAIN_256X,
  AS7343_GAIN_512X,
  AS7343_GAIN_1024X,
  AS7343_GAIN_2048X,
} AS7343_gain_t;

/**
 * @brief Available SMUX configuration commands
 *
 */
typedef enum {
  AS7343_SMUX_CMD_ROM_RESET, ///< ROM code initialization of SMUX
  AS7343_SMUX_CMD_READ,      ///< Read SMUX configuration to RAM from SMUX chain
  AS7343_SMUX_CMD_WRITE, ///< Write SMUX configuration from RAM to SMUX chain
} AS7343_smux_cmd_t;
/**
 * @brief ADC Channel specifiers for configuration
 *
 */
typedef enum {
  AS7343_ADC_CHANNEL_0,
  AS7343_ADC_CHANNEL_1,
  AS7343_ADC_CHANNEL_2,
  AS7343_ADC_CHANNEL_3,
  AS7343_ADC_CHANNEL_4,
  AS7343_ADC_CHANNEL_5,
} AS7343_adc_channel_t;
/**
 * @brief Spectral Channel specifiers for configuration and reading
 *
 */
typedef enum {
  AS7343_CHANNEL_450_FZ,
  AS7343_CHANNEL_555_FY,
  AS7343_CHANNEL_600_FXL,
  AS7343_CHANNEL_855_NIR,
  AS7343_CHANNEL_CLEAR_1,
  AS7343_CHANNEL_FD_1,
  AS7343_CHANNEL_425_F2,
  AS7343_CHANNEL_475_F3,
  AS7343_CHANNEL_515_F4,
  AS7343_CHANNEL_640_F6,
  AS7343_CHANNEL_CLEAR_0,
  AS7343_CHANNEL_FD_0,
  AS7343_CHANNEL_405_F1,
  AS7343_CHANNEL_550_F5,
  AS7343_CHANNEL_690_F7,
  AS7343_CHANNEL_745_F8,
  AS7343_CHANNEL_CLEAR,
  AS7343_CHANNEL_FD,
} AS7343_color_channel_t;

/**
 * @brief The number of measurement cycles with spectral data outside of a
 * threshold required to trigger an interrupt
 *
 */
typedef enum {
  AS7343_INT_COUNT_ALL, ///< 0
  AS7343_INT_COUNT_1,   ///< 1
  AS7343_INT_COUNT_2,   ///< 2
  AS7343_INT_COUNT_3,   ///< 3
  AS7343_INT_COUNT_5,   ///< 4
  AS7343_INT_COUNT_10,  ///< 5
  AS7343_INT_COUNT_15,  ///< 6
  AS7343_INT_COUNT_20,  ///< 7
  AS7343_INT_COUNT_25,  ///< 8
  AS7343_INT_COUNT_30,  ///< 9
  AS7343_INT_COUNT_35,  ///< 10
  AS7343_INT_COUNT_40,  ///< 11
  AS7343_INT_COUNT_45,  ///< 12
  AS7343_INT_COUNT_50,  ///< 13
  AS7343_INT_COUNT_55,  ///< 14
  AS7343_INT_COUNT_60,  ///< 15
} AS7343_int_cycle_count_t;

/**
 * @brief Pin directions to set how the GPIO pin is to be used
 *
 */
typedef enum {
  AS7343_GPIO_OUTPUT, ///< THhe GPIO pin is configured as an open drain output
  AS7343_GPIO_INPUT,  ///< The GPIO Pin is set as a high-impedence input
} AS7343_gpio_dir_t;

/**
 * @brief Wait states for async reading
 */
typedef enum {
  AS7343_WAITING_START, //
  AS7343_WAITING_LOW,   //
  AS7343_WAITING_HIGH,  //
  AS7343_WAITING_DONE,  //
} AS7343_waiting_t;

/** AS7331 intance */
typedef struct st_as7331_intance
{
    rm_comms_instance_t const  *p_i2c;
} as7343_instance_t;

typedef struct st_as7343_data
{
    uint16_t readings[12];
} as7343_data_t;

/**********************************************************************************************************************
 * Exported global variables
 **********************************************************************************************************************/

/** @endcond */

/**********************************************************************************************************************
 * Public Function Prototypes
 **********************************************************************************************************************/
fsp_err_t AS7343_begin(as7343_instance_t * const p_inst);
fsp_err_t AS7343_setATIME(as7343_instance_t * const p_inst, uint8_t atime_value);
fsp_err_t AS7343_setASTEP(as7343_instance_t * const p_inst, uint16_t astep_value);
fsp_err_t AS7343_setGain(as7343_instance_t * const p_inst, AS7343_gain_t gain_value);
fsp_err_t AS7343_readAllChannels(as7343_instance_t * const p_inst, uint16_t *readings_buffer);
uint16_t AS7343_getASTEP(as7343_instance_t * const p_inst);
uint8_t AS7343_getATIME(as7343_instance_t * const p_inst);
AS7343_gain_t AS7343_getGain(as7343_instance_t * const p_inst);
long AS7343_getTINT(as7343_instance_t * const p_inst);
double AS7343_toBasicCounts(as7343_instance_t * const p_inst, uint16_t raw);
//void delayForData(int waitTime = 0)
//uint16_t readChannel(AS7343_adc_channel_t channel);
//uint16_t getChannel(AS7343_color_channel_t channel);
fsp_err_t AS7343_startReading(as7343_instance_t * const p_inst);
bool AS7343_checkReadingProgress(as7343_instance_t * const p_inst);
bool AS7343_getAllChannels(uint16_t *readings_buffer);
uint16_t AS7343_detectFlickerHz(as7343_instance_t * const p_inst);
void AS7343_setup_F1F4_Clear_NIR(as7343_instance_t * const p_inst);
void AS7343_setup_F5F8_Clear_NIR(as7343_instance_t * const p_inst);
fsp_err_t AS7343_powerEnable(as7343_instance_t * const p_inst, bool enable_power);
fsp_err_t AS7343_enableSpectralMeasurement(as7343_instance_t * const p_inst, bool enable_measurement);
//bool setHighThreshold(uint16_t high_threshold);
//bool setLowThreshold(uint16_t low_threshold);
//uint16_t getHighThreshold(void);
//uint16_t getLowThreshold(void);
//bool enableSpectralInterrupt(bool enable_int);
//bool enableSystemInterrupt(bool enable_int);
//bool setAPERS(AS7343_int_cycle_count_t cycle_count);
//bool setSpectralThresholdChannel(AS7343_adc_channel_t channel);
//uint8_t getInterruptStatus(void);
//bool clearInterruptStatus(void);
//bool spectralInterruptTriggered(void);
//uint8_t spectralInterruptSource(void);
//bool spectralLowTriggered(void);
//bool spectralHighTriggered(void);
fsp_err_t AS7343_enableLED(as7343_instance_t * const p_inst, bool enable_led);
fsp_err_t AS7343_setLEDCurrent(as7343_instance_t * const p_inst, uint16_t led_current_ma);
//uint16_t AS7343_getLEDCurrent(as7343_instance_t * const p_inst);
void AS7343_disableAll(as7343_instance_t * const p_inst);
fsp_err_t AS7343_getIsDataReady(as7343_instance_t * const p_inst, uint32_t *p_status);
fsp_err_t AS7343_setBank(as7343_instance_t * const p_inst, bool low);
//AS7343_gpio_dir_t getGPIODirection(void);
//bool setGPIODirection(AS7343_gpio_dir_t gpio_direction);
//bool getGPIOInverted(void);
//bool setGPIOInverted(bool gpio_inverted);
//bool getGPIOValue(void);
//bool setGPIOValue(bool);
//bool digitalSaturation(void);
//bool analogSaturation(void);
//bool clearDigitalSaturationStatus(void);
//bool clearAnalogSaturationStatus(void);
fsp_err_t AS7343_Close(as7343_instance_t * const p_inst);


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
