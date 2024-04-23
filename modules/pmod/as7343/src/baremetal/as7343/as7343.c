/*
 * as7331.c
 *
 *  Created on: Nov 13, 2023
 *      Author:
 */

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "as7343.h"

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

#define AS7343_TIMEOUT                            (1000)

#define AS7343_I2CADDR_DEFAULT 0x39 ///< AS7343 default i2c address
#define AS7343_CHIP_ID 0x81         ///< AS7343 default device id from datasheet

#define AS7343_WHOAMI 0x5A ///< Chip ID register

#define AS7343_ASTATUS 0x60     ///< AS7343_ASTATUS (unused)
#define AS7343_CH0_DATA_L_ 0x61 ///< AS7343_CH0_DATA_L (unused)
#define AS7343_CH0_DATA_H_ 0x62 ///< AS7343_CH0_DATA_H (unused)
#define AS7343_ITIME_L 0x63     ///< AS7343_ITIME_L (unused)
#define AS7343_ITIME_M 0x64     ///< AS7343_ITIME_M (unused)
#define AS7343_ITIME_H 0x65     ///< AS7343_ITIME_H (unused)
#define AS7343_CONFIG 0x70 ///< Enables LED control and sets light sensing mode
#define AS7343_STAT 0x71   ///< AS7343_STAT (unused)
#define AS7343_EDGE 0x72   ///< AS7343_EDGE (unused)
#define AS7343_GPIO 0x73   ///< Connects photo diode to GPIO or INT pins
#define AS7343_LED 0xCD    ///< LED Register; Enables and sets current limit
#define AS7343_ENABLE                                                          \
		0x80 ///< Main enable register. Controls SMUX, Flicker Detection, Spectral
///< Measurements and Power
#define AS7343_ATIME 0x81       ///< Sets ADC integration step count
#define AS7343_WTIME 0x83       ///< AS7343_WTIME (unused)
#define AS7343_SP_LOW_TH_L 0x84 ///< Spectral measurement Low Threshold low byte
#define AS7343_SP_LOW_TH_H                                                     \
		0x85 ///< Spectral measurement Low Threshold high byte
#define AS7343_SP_HIGH_TH_L                                                    \
		0x86 ///< Spectral measurement High Threshold low byte
#define AS7343_SP_HIGH_TH_H                                                    \
		0x87                    ///< Spectral measurement High Threshold low byte
#define AS7343_AUXID 0x58 ///< AS7343_AUXID (unused)
#define AS7343_REVID 0x59 ///< AS7343_REVID (unused)
#define AS7343_ID 0x92    ///< AS7343_ID (unused)
#define AS7343_STATUS                                                          \
		0x93 ///< Interrupt status registers. Indicates the occourance of an interrupt
#define AS7343_ASTATUS_ 0x94   ///< AS7343_ASTATUS, same as 0x60 (unused)
#define AS7343_CH0_DATA_L 0x95 ///< ADC Channel Data
#define AS7343_CH0_DATA_H 0x96 ///< ADC Channel Data
#define AS7343_CH1_DATA_L 0x97 ///< ADC Channel Data
#define AS7343_CH1_DATA_H 0x98 ///< ADC Channel Data
#define AS7343_CH2_DATA_L 0x99 ///< ADC Channel Data
#define AS7343_CH2_DATA_H 0x9A ///< ADC Channel Data
#define AS7343_CH3_DATA_L 0x9B ///< ADC Channel Data
#define AS7343_CH3_DATA_H 0x9C ///< ADC Channel Data
#define AS7343_CH4_DATA_L 0x9D ///< ADC Channel Data
#define AS7343_CH4_DATA_H 0x9E ///< ADC Channel Data
#define AS7343_CH5_DATA_L 0x9F ///< ADC Channel Data
#define AS7343_CH5_DATA_H 0xA0 ///< ADC Channel Data
#define AS7343_CH6_DATA_L 0xA1 ///< ADC Channel Data
#define AS7343_CH6_DATA_H 0xA2 ///< ADC Channel Data
#define AS7343_CH7_DATA_L 0xA3 ///< ADC Channel Data
#define AS7343_CH7_DATA_H 0xA4 ///< ADC Channel Data
#define AS7343_CH8_DATA_L 0xA5 ///< ADC Channel Data
#define AS7343_CH8_DATA_H 0xA6 ///< ADC Channel Data
#define AS7343_CH9_DATA_L 0xA7 ///< ADC Channel Data
#define AS7343_CH9_DATA_H 0xA8 ///< ADC Channel Data
#define AS7343_CH10_DATA_L 0xA9 ///< ADC Channel Data
#define AS7343_CH10_DATA_H 0xAA ///< ADC Channel Data
#define AS7343_CH11_DATA_L 0xAB ///< ADC Channel Data
#define AS7343_CH11_DATA_H 0xAC ///< ADC Channel Data
#define AS7343_CH12_DATA_L 0xAD ///< ADC Channel Data
#define AS7343_CH12_DATA_H 0xAE ///< ADC Channel Data
#define AS7343_CH13_DATA_L 0xAF ///< ADC Channel Data
#define AS7343_CH13_DATA_H 0xB0 ///< ADC Channel Data
#define AS7343_CH14_DATA_L 0xB1 ///< ADC Channel Data
#define AS7343_CH14_DATA_H 0xB2 ///< ADC Channel Data
#define AS7343_CH15_DATA_L 0xB3 ///< ADC Channel Data
#define AS7343_CH15_DATA_H 0xB4 ///< ADC Channel Data
#define AS7343_CH16_DATA_L 0xB5 ///< ADC Channel Data
#define AS7343_CH16_DATA_H 0xB6 ///< ADC Channel Data
#define AS7343_CH17_DATA_L 0xB7 ///< ADC Channel Data
#define AS7343_CH17_DATA_H 0xB8 ///< ADC Channel Data

#define AS7343_STATUS2 0x90 ///< Measurement status flags; saturation, validity
#define AS7343_STATUS3                                                         \
		0x91 ///< Spectral interrupt source, high or low threshold
#define AS7343_STATUS5 0xBB ///< AS7343_STATUS5 (unused)
#define AS7343_STATUS4 0xBC ///< AS7343_STATUS6 (unused)
#define AS7343_CFG0                                                            \
		0xBF ///< Sets Low power mode, Register bank, and Trigger lengthening
#define AS7343_CFG1 0xC6 ///< Controls ADC Gain
#define AS7343_CFG3 0xC7 ///< AS7343_CFG3 (unused)
#define AS7343_CFG6 0xF5 ///< Used to configure Smux
#define AS7343_CFG8 0xC9 ///< AS7343_CFG8 (unused)
#define AS7343_CFG9                                                            \
		0xCA ///< Enables flicker detection and smux command completion system
///< interrupts
#define AS7343_CFG10 0x65 ///< AS7343_CFG10 (unused)
#define AS7343_CFG12                                                           \
		0x66 ///< Spectral threshold channel for interrupts, persistence and auto-gain
#define AS7343_CFG20 0xD6 //< FIFO and auto SMUX
#define AS7343_PERS                                                            \
		0xCF ///< Number of measurement cycles outside thresholds to trigger an
///< interupt
#define AS7343_GPIO2                                                           \
		0x6B ///< GPIO Settings and status: polarity, direction, sets output, reads
///< input
#define AS7343_ASTEP_L 0xD4      ///< Integration step size ow byte
#define AS7343_ASTEP_H 0xD5      ///< Integration step size high byte
#define AS7343_AGC_GAIN_MAX 0xD7 ///< AS7343_AGC_GAIN_MAX (unused)
#define AS7343_AZ_CONFIG 0xDE    ///< AS7343_AZ_CONFIG (unused)
#define AS7343_FD_TIME1 0xE0 ///< Flicker detection integration time low byte
#define AS7343_FD_TIME2 0xE2 ///< Flicker detection gain and high nibble
#define AS7343_FD_CFG0 0xDF  ///< AS7343_FD_CFG0 (unused)
#define AS7343_FD_STATUS                                                       \
		0xE3 ///< Flicker detection status; measurement valid, saturation, flicker
///< type
#define AS7343_INTENAB 0xF9  ///< Enables individual interrupt types
#define AS7343_CONTROL 0xFA  ///< Auto-zero, fifo clear, clear SAI active
#define AS7343_FIFO_MAP 0xFC ///< AS7343_FIFO_MAP (unused)
#define AS7343_FIFO_LVL 0xFD ///< AS7343_FIFO_LVL (unused)
#define AS7343_FDATA_L 0xFE  ///< AS7343_FDATA_L (unused)
#define AS7343_FDATA_H 0xFF  ///< AS7343_FDATA_H (unused)

#define AS7343_SPECTRAL_INT_HIGH_MSK                                           \
		0b00100000 ///< bitmask to check for a high threshold interrupt
#define AS7343_SPECTRAL_INT_LOW_MSK                                            \
		0b00010000 ///< bitmask to check for a low threshold interrupt




/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Global function prototypes
 **********************************************************************************************************************/
#if (BSP_CFG_RTOS == 0)
static bool g_comm_i2c_flag = false;
static rm_comms_event_t g_comm_i2c_event;
#endif

/***********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/
static bool as7343_init(as7343_instance_t * const p_inst);
static fsp_err_t as7343_read (as7343_instance_t * const p_inst, rm_comms_write_read_params_t write_read_params);
static fsp_err_t as7343_write (as7343_instance_t * const p_inst, uint8_t * const p_src, uint8_t const bytes);
static fsp_err_t as7343_write_bit(as7343_instance_t * const p_inst, uint8_t *p_reg, uint8_t bits, uint8_t shift, uint32_t data);
static fsp_err_t as7343_read_bit(as7343_instance_t * const p_inst, uint8_t *p_reg, uint8_t bits, uint8_t shift, uint32_t *p_data);
static fsp_err_t as7343_delay_us (uint32_t const delay_us);
void as7343_callback(rm_comms_callback_args_t *p_args);

static fsp_err_t as7343_enableSMUX(as7343_instance_t * const p_inst);
fsp_err_t as7343_enableFlickerDetection(as7343_instance_t * const p_inst, bool enable_fd);
static void as7343_FDConfig(as7343_instance_t * const p_inst);
int8_t as7343_getFlickerDetectStatus(as7343_instance_t * const p_inst);
static bool as7343_setSMUXCommand(as7343_instance_t * const p_inst, AS7343_smux_cmd_t command);
static void as7343_writeRegister(as7343_instance_t * const p_inst, uint8_t addr, uint8_t val);
static void as7343_setSMUXLowChannels(as7343_instance_t * const p_inst, bool f1_f4);

/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/
static uint16_t _channel_readings[18];
static AS7343_waiting_t _readingState;

/***********************************************************************************************************************
 * Global variables
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Functions
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @brief Opens and configures the AS7331 Middle module. Implements @ref as7343_api_t::open.
 *
 *
 * @retval FSP_SUCCESS              AS7331 successfully configured.
 * @retval FSP_ERR_ASSERTION        Null pointer, or one or more configuration options is invalid.
 * @retval FSP_ERR_ALREADY_OPEN     Module is already open.  This module can only be opened once.
 **********************************************************************************************************************/
fsp_err_t AS7343_begin(as7343_instance_t * const p_inst)
{
    fsp_err_t err = FSP_SUCCESS;
    rm_comms_instance_t *p_i2c = (rm_comms_instance_t *) p_inst->p_i2c;
    uint8_t write_data[2];

    /* Open Communications middleware */
    err =  p_i2c->p_api->open(p_i2c->p_ctrl,
                              p_i2c->p_cfg);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    write_data[0] = 0xBF;
    write_data[1]  = 0;
    as7343_write(p_inst, write_data, sizeof(write_data));
    write_data[0] = 0x80;
    write_data[1]  = 1;
    as7343_write(p_inst, write_data, sizeof(write_data));

    as7343_init(p_inst);

    return FSP_SUCCESS;
}

fsp_err_t AS7343_setATIME(as7343_instance_t * const p_inst, uint8_t atime_value)
{
    uint8_t write_data[2];

    write_data[0] = AS7343_ATIME;
    write_data[1]  = atime_value;
    return as7343_write(p_inst, write_data, sizeof(write_data));
}
fsp_err_t AS7343_setASTEP(as7343_instance_t * const p_inst, uint16_t astep_value)
{
    uint8_t write_data[3];

    write_data[0] = AS7343_ASTEP_L;
    write_data[1]  = (uint8_t)astep_value;
    write_data[2]  = (uint8_t)(astep_value>>8);
    return as7343_write(p_inst, write_data, sizeof(write_data));
}

fsp_err_t AS7343_setGain(as7343_instance_t * const p_inst, AS7343_gain_t gain_value)
{
    uint8_t write_data[2];

    write_data[0] = AS7343_CFG1;
    write_data[1]  = (uint8_t)gain_value;
    return as7343_write(p_inst, write_data, sizeof(write_data));
}

/**
 * @brief fills the provided buffer with the current measurements for Spectral
 * channels F1-8, Clear and NIR
 *
 * @param readings_buffer Pointer to a buffer of length 10 or more to fill with
 * sensor data
 * @return true: success false: failure
 */
fsp_err_t AS7343_readAllChannels(as7343_instance_t * const p_inst, uint16_t *readings_buffer)
{
    rm_comms_write_read_params_t write_read_params;
    uint8_t reg[2];
    uint32_t status = 0;
    //enableSMUX();
    //clearAnalogSaturationStatus();
    //clearDigitalSaturationStatus();
    AS7343_enableSpectralMeasurement(p_inst, true); // Start integration
    AS7343_getIsDataReady(p_inst, &status);
    while(status == false)
    {
        R_BSP_SoftwareDelay(1, BSP_DELAY_UNITS_MILLISECONDS);
        AS7343_getIsDataReady(p_inst, &status);
    }

    reg[0] = AS7343_CH0_DATA_L;
    write_read_params.p_src      = reg;
    write_read_params.src_bytes  = 1;
    write_read_params.p_dest     = (uint8_t *)readings_buffer;
    write_read_params.dest_bytes = 24;
    return as7343_read(p_inst, write_read_params);
}

/**
 * @brief Returns the integration time step size
 *
 * Step size is `(astep_value+1) * 2.78 uS`
 *
 * @return uint16_t The current integration time step size
 */
uint16_t AS7343_getASTEP(as7343_instance_t * const p_inst) {
    rm_comms_write_read_params_t write_read_params;
    uint8_t reg[2];
    uint16_t data = 0;

    reg[0] = AS7343_ASTEP_L;
    write_read_params.p_src      = reg;
    write_read_params.src_bytes  = 1;
    write_read_params.p_dest     = (uint8_t *)&data;
    write_read_params.dest_bytes = 2;
    as7343_read(p_inst, write_read_params);

    return data;
}

/**
 * @brief Returns the integration time step count
 *
 * Total integration time will be `(ATIME + 1) * (ASTEP + 1) * 2.78µS`
 *
 * @return uint8_t The current integration time step count
 */
uint8_t AS7343_getATIME(as7343_instance_t * const p_inst) {
    rm_comms_write_read_params_t write_read_params;
    uint8_t reg[2];
    uint8_t data = 0;

    reg[0] = AS7343_ATIME;
    write_read_params.p_src      = reg;
    write_read_params.src_bytes  = 1;
    write_read_params.p_dest     = (uint8_t *)&data;
    write_read_params.dest_bytes = 1;
    as7343_read(p_inst, write_read_params);

    return data;
}

/**
 * @brief Returns the ADC gain multiplier
 *
 * @return AS7343_gain_t The current ADC gain multiplier
 */
AS7343_gain_t AS7343_getGain(as7343_instance_t * const p_inst) {
    rm_comms_write_read_params_t write_read_params;
    uint8_t reg[2];
    AS7343_gain_t data = AS7343_GAIN_0_5X;

    reg[0] = AS7343_CFG1;
    write_read_params.p_src      = reg;
    write_read_params.src_bytes  = 1;
    write_read_params.p_dest     = (uint8_t *)data;
    write_read_params.dest_bytes = 2;
    as7343_read(p_inst, write_read_params);

    return data;
}

/**
 * @brief Returns the integration time
 *
 * The integration time is `(ATIME + 1) * (ASTEP + 1) * 2.78µS`
 *
 * @return long The current integration time in ms
 */
long AS7343_getTINT(as7343_instance_t * const p_inst) {
  uint16_t astep = AS7343_getASTEP(p_inst);
  uint8_t atime = AS7343_getATIME(p_inst);

  return (long)((atime + 1) * (astep + 1) * 2.78 / 1000);
}

/**
 * @brief Converts raw ADC values to basic counts
 *
 * The basic counts are `RAW/(GAIN * TINT)`
 *
 * @param raw The raw ADC values to convert
 *
 * @return float The basic counts
 */
double AS7343_toBasicCounts(as7343_instance_t * const p_inst, uint16_t raw) {
  float gain_val = 0;
  AS7343_gain_t gain = AS7343_getGain(p_inst);
  switch (gain) {
  case AS7343_GAIN_0_5X:
    gain_val = 0.5;
    break;
  case AS7343_GAIN_1X:
    gain_val = 1;
    break;
  case AS7343_GAIN_2X:
    gain_val = 2;
    break;
  case AS7343_GAIN_4X:
    gain_val = 4;
    break;
  case AS7343_GAIN_8X:
    gain_val = 8;
    break;
  case AS7343_GAIN_16X:
    gain_val = 16;
    break;
  case AS7343_GAIN_32X:
    gain_val = 32;
    break;
  case AS7343_GAIN_64X:
    gain_val = 64;
    break;
  case AS7343_GAIN_128X:
    gain_val = 128;
    break;
  case AS7343_GAIN_256X:
    gain_val = 256;
    break;
  case AS7343_GAIN_512X:
    gain_val = 512;
    break;
  case AS7343_GAIN_1024X:
      gain_val = 1024;
      break;
  case AS7343_GAIN_2048X:
        gain_val = 2048;
        break;
  }
  return (double)(raw / (gain_val * (AS7343_getATIME(p_inst) + 1) * (AS7343_getASTEP(p_inst) + 1) * 2.78 / 1000));
}

/**
 * @brief Detect a flickering light
 * @return The frequency of a detected flicker or 1 if a flicker of
 * unknown frequency is detected
 */
uint16_t AS7343_detectFlickerHz(as7343_instance_t * const p_inst) {
  // bool isEnabled = true;
  // bool isFdmeasReady = false;

  // disable everything; Flicker detect, smux, wait, spectral, power
    AS7343_disableAll(p_inst);
  // re-enable power
    AS7343_powerEnable(p_inst, true);

  // Write SMUX configuration from RAM to set SMUX chain registers (Write 0x10
  // to CFG6)
    as7343_setSMUXCommand(p_inst, AS7343_SMUX_CMD_WRITE);

  // Write new configuration to all the 20 registers for detecting Flicker
    as7343_FDConfig(p_inst);

  // Start SMUX command
    as7343_enableSMUX(p_inst);

  // Enable SP_EN bit
    AS7343_enableSpectralMeasurement(p_inst, true);

  // Enable flicker detection bit
    as7343_writeRegister(p_inst,AS7343_ENABLE, 0x41);
  R_BSP_SoftwareDelay(500, BSP_DELAY_UNITS_MILLISECONDS); // SF 2020-08-12 Does this really need to be so long?
  int8_t flicker_status = as7343_getFlickerDetectStatus(p_inst);
  as7343_enableFlickerDetection(p_inst, false);
  switch (flicker_status) {
  case 44:
    return 1;
  case 45:
    return 100;
  case 46:
    return 120;
  default:
    return 0;
  }
}

/**
 * @brief starts the process of getting readings from all channels without using
 * delays
 *
 * @return true: success false: failure (a bit arbitrary)
 */
fsp_err_t AS7343_startReading(as7343_instance_t * const p_inst) {
    _readingState = AS7343_WAITING_START; // Start the measurement please
    AS7343_checkReadingProgress(p_inst);               // Call the check function to start it
    return FSP_SUCCESS;
}

/**
 * @brief runs the process of getting readings from all channels without using
 * delays.  Should be called regularly (ie. in loop()) Need to call
 * startReading() to initialise the process Need to call getAllChannels() to
 * transfer the data into an external buffer
 *
 * @return true: reading is complete false: reading is incomplete (or failed)
 */
bool AS7343_checkReadingProgress(as7343_instance_t * const p_inst) {
    rm_comms_write_read_params_t write_read_params;
    uint32_t status = 0;
    uint8_t reg[2];

    if (_readingState == AS7343_WAITING_START) {
        as7343_setSMUXLowChannels(p_inst, true);        // Configure SMUX to read low channels
        AS7343_enableSpectralMeasurement(p_inst, true); // Start integration
        _readingState = AS7343_WAITING_LOW;
        return false;
    }
    AS7343_getIsDataReady(p_inst, &status);
    if (!status || _readingState == AS7343_WAITING_DONE)
        return false;

    if (_readingState ==
            AS7343_WAITING_LOW) // Check of getIsDataRead() is already done
    {

        reg[0] = AS7343_CH0_DATA_L;
        write_read_params.p_src      = reg;
        write_read_params.src_bytes  = 1;
        write_read_params.p_dest     = (uint8_t *)_channel_readings;
        write_read_params.dest_bytes = 12;
        as7343_read(p_inst, write_read_params);

        as7343_setSMUXLowChannels(p_inst, false);       // Configure SMUX to read high channels
        AS7343_enableSpectralMeasurement(p_inst, true); // Start integration
        _readingState = AS7343_WAITING_HIGH;
        return false;
    }

    if (_readingState ==
            AS7343_WAITING_HIGH) // Check of getIsDataRead() is already done
    {
        _readingState = AS7343_WAITING_DONE;

        reg[0] = AS7343_CH0_DATA_L;
        write_read_params.p_src      = reg;
        write_read_params.src_bytes  = 1;
        write_read_params.p_dest     = (uint8_t *)&_channel_readings[6];
        write_read_params.dest_bytes = 12;
        as7343_read(p_inst, write_read_params);

        return true;
    }

    return false;
}

/**
 * @brief transfer all the values from the private result buffer into one
 * nominated
 *
 * @param readings_buffer Pointer to a buffer of length 12 (THERE IS NO ERROR
 * CHECKING, YE BE WARNED!)
 *
 * @return true: success false: failure
 */
bool AS7343_getAllChannels(uint16_t *readings_buffer) {
    for (int i = 0; i < 12; i++)
        readings_buffer[i] = _channel_readings[i];
    return true;
}

/**
 * @brief Configure SMUX for sensors F1-4, Clear and NIR
 *
 */
void AS7343_setup_F1F4_Clear_NIR(as7343_instance_t * const p_inst) {
    // SMUX Config for F1,F2,F3,F4,NIR,Clear
    as7343_writeRegister(p_inst,0x00, 0x30); // F3 left set to ADC2
    as7343_writeRegister(p_inst,0x01, 0x01); // F1 left set to ADC0
    as7343_writeRegister(p_inst,0x02, 0x00); // Reserved or disabled
    as7343_writeRegister(p_inst,0x03, 0x00); // F8 left disabled
    as7343_writeRegister(p_inst,0x04, 0x00); // F6 left disabled
    as7343_writeRegister(p_inst,
            0x05,
            0x42); // F4 left connected to ADC3/f2 left connected to ADC1
    as7343_writeRegister(p_inst,0x06, 0x00); // F5 left disbled
    as7343_writeRegister(p_inst,0x07, 0x00); // F7 left disbled
    as7343_writeRegister(p_inst,0x08, 0x50); // CLEAR connected to ADC4
    as7343_writeRegister(p_inst,0x09, 0x00); // F5 right disabled
    as7343_writeRegister(p_inst,0x0A, 0x00); // F7 right disabled
    as7343_writeRegister(p_inst,0x0B, 0x00); // Reserved or disabled
    as7343_writeRegister(p_inst,0x0C, 0x20); // F2 right connected to ADC1
    as7343_writeRegister(p_inst,0x0D, 0x04); // F4 right connected to ADC3
    as7343_writeRegister(p_inst,0x0E, 0x00); // F6/F8 right disabled
    as7343_writeRegister(p_inst,0x0F, 0x30); // F3 right connected to AD2
    as7343_writeRegister(p_inst,0x10, 0x01); // F1 right connected to AD0
    as7343_writeRegister(p_inst,0x11, 0x50); // CLEAR right connected to AD4
    as7343_writeRegister(p_inst,0x12, 0x00); // Reserved or disabled
    as7343_writeRegister(p_inst,0x13, 0x06); // NIR connected to ADC5
}

/**
 * @brief Configure SMUX for sensors F5-8, Clear and NIR
 *
 */
void AS7343_setup_F5F8_Clear_NIR(as7343_instance_t * const p_inst) {
    // SMUX Config for F5,F6,F7,F8,NIR,Clear
    as7343_writeRegister(p_inst,0x00, 0x00); // F3 left disable
    as7343_writeRegister(p_inst,0x01, 0x00); // F1 left disable
    as7343_writeRegister(p_inst,0x02, 0x00); // reserved/disable
    as7343_writeRegister(p_inst,0x03, 0x40); // F8 left connected to ADC3
    as7343_writeRegister(p_inst,0x04, 0x02); // F6 left connected to ADC1
    as7343_writeRegister(p_inst,0x05, 0x00); // F4/ F2 disabled
    as7343_writeRegister(p_inst,0x06, 0x10); // F5 left connected to ADC0
    as7343_writeRegister(p_inst,0x07, 0x03); // F7 left connected to ADC2
    as7343_writeRegister(p_inst,0x08, 0x50); // CLEAR Connected to ADC4
    as7343_writeRegister(p_inst,0x09, 0x10); // F5 right connected to ADC0
    as7343_writeRegister(p_inst,0x0A, 0x03); // F7 right connected to ADC2
    as7343_writeRegister(p_inst,0x0B, 0x00); // Reserved or disabled
    as7343_writeRegister(p_inst,0x0C, 0x00); // F2 right disabled
    as7343_writeRegister(p_inst,0x0D, 0x00); // F4 right disabled
    as7343_writeRegister(p_inst,
            0x0E,
            0x24); // F8 right connected to ADC2/ F6 right connected to ADC1
    as7343_writeRegister(p_inst,0x0F, 0x00); // F3 right disabled
    as7343_writeRegister(p_inst,0x10, 0x00); // F1 right disabled
    as7343_writeRegister(p_inst,0x11, 0x50); // CLEAR right connected to AD4
    as7343_writeRegister(p_inst,0x12, 0x00); // Reserved or disabled
    as7343_writeRegister(p_inst,0x13, 0x06); // NIR connected to ADC5
}

fsp_err_t AS7343_powerEnable(as7343_instance_t * const p_inst, bool enable_power)
{
    uint8_t reg = AS7343_ENABLE;
    return as7343_write_bit(p_inst, &reg, 1, 0, enable_power);
}

/**
 * @brief Disable Spectral reading, flicker detection, and power
 *
 * */
void AS7343_disableAll(as7343_instance_t * const p_inst) {
    uint8_t reg = AS7343_ENABLE;
    as7343_write_bit(p_inst, &reg, 1, 0, 0);
}

/**
 * @brief Enables measurement of spectral data
 *
 * @param enable_measurement true: enabled false: disabled
 * @return true: success false: failure
 */
fsp_err_t AS7343_enableSpectralMeasurement(as7343_instance_t * const p_inst, bool enable_measurement)
{
    uint8_t reg = AS7343_ENABLE;
    return as7343_write_bit(p_inst, &reg, 1, 1, enable_measurement);
}

fsp_err_t AS7343_enableLED(as7343_instance_t * const p_inst, bool enable_led)
{
    uint8_t reg;
    uint32_t data = 0;
    fsp_err_t err;

    AS7343_setBank(p_inst, true);

    data = (uint32_t)enable_led;
    reg = AS7343_CONFIG;
    err = as7343_write_bit(p_inst, &reg, 1, 3, data);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    reg = AS7343_LED;
    err = as7343_write_bit(p_inst, &reg, 1, 7, data);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    AS7343_setBank(p_inst, false);

    return err;
}

/**
 * @brief Set the current limit for the LED
 *
 * @param led_current_ma the value to set in milliamps. With a minimum of 4. Any
 * amount under 4 will be rounded up to 4
 *
 * Range is 4mA to 258mA
 * @return true: success false: failure
 */
fsp_err_t AS7343_setLEDCurrent(as7343_instance_t * const p_inst, uint16_t led_current_ma) {
    uint8_t reg;
    uint32_t data = 0;
    fsp_err_t err;
    // check within permissible range
    if (led_current_ma > 258) {
        return false;
    }
    if (led_current_ma < 4) {
        led_current_ma = 4;
    }
    AS7343_setBank(p_inst, true); // //Access registers 0x20 to 0x7F

    reg = AS7343_LED;
    data = (uint32_t)((led_current_ma - 4) / 2);
    err = as7343_write_bit(p_inst, &reg, 7, 0, data);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    AS7343_setBank(p_inst, false); // Access registers 0x80 and above (default)
    return err;
}

/**
 * @brief
 *
 * @return true: success false: failure
 */
fsp_err_t AS7343_getIsDataReady(as7343_instance_t * const p_inst, uint32_t *p_status)
{

    uint8_t reg = AS7343_STATUS2;
    return as7343_read_bit(p_inst, &reg, 1, 6, p_status);
}

/**
 * @brief Sets the active register bank
 *
 * The AS7343 uses banks to organize the register making it nescessary to set
 * the correct bank to access a register.
 *

 * @param low **true**:
 * **false**: Set the current bank to allow access to registers with addresses
 of `0x80` and above
 * @return true: success false: failure
 */
fsp_err_t AS7343_setBank(as7343_instance_t * const p_inst, bool data)
{
    uint8_t reg = AS7343_CFG0;
    uint32_t value = 0;
    value = (uint32_t)data;
    return as7343_write_bit(p_inst, &reg, 1, 4, value);
}



fsp_err_t AS7343_Close (as7343_instance_t * const p_inst)
{
    rm_comms_instance_t *p_i2c = (rm_comms_instance_t *) p_inst->p_i2c;

    /* Close Communications Middleware */
    p_i2c->p_api->close(p_i2c->p_ctrl);

    return FSP_SUCCESS;
}


/*******************************************************************************************************************//**
 * @brief AS7331 callback function called in the I2C Communications Middleware callback function.
 **********************************************************************************************************************/
void as7343_callback (rm_comms_callback_args_t * p_args)
{
#if (BSP_CFG_RTOS > 0)
    FSP_PARAMETER_NOT_USED(p_args);
#else
    /* Set event */
    switch (p_args->event)
    {
        case RM_COMMS_EVENT_OPERATION_COMPLETE:
        {
            g_comm_i2c_event = RM_COMMS_EVENT_OPERATION_COMPLETE;

            g_comm_i2c_flag = true;

            break;
        }

        case RM_COMMS_EVENT_ERROR:
            g_comm_i2c_event = RM_COMMS_EVENT_ERROR;
            break;
        default:
        {
            break;
        }
    }
#endif
}

/***********************************************************************************************************************
 * Private Functions
 **********************************************************************************************************************/

static fsp_err_t as7343_enableSMUX(as7343_instance_t * const p_inst) {

    uint8_t reg;
    fsp_err_t err;
    uint32_t status = 0;

    reg = AS7343_ENABLE;
    err = as7343_write_bit(p_inst, &reg, 1, 4, true);

    int timeOut = 1000; // Arbitrary value, but if it takes 1000 milliseconds then
    // something is wrong
    int count = 0;
    as7343_read_bit(p_inst, &reg, 1, 4, &status);
    while (status && count < timeOut) {
        as7343_read_bit(p_inst, &reg, 1, 6, &status);
        R_BSP_SoftwareDelay(1, BSP_DELAY_UNITS_MILLISECONDS);
        count++;
    }
    if (count >= timeOut)
        return FSP_ERR_TIMEOUT;
    else
        return err;
}

static bool as7343_setSMUXCommand(as7343_instance_t * const p_inst, AS7343_smux_cmd_t command) {
    uint8_t reg = AS7343_CFG6;
    return as7343_write_bit(p_inst, &reg, 2, 3, command);
}

/**
 * @brief Write a uint8_t to the given register
 *
 * @param addr Register address
 * @param val The value to set the register to
 */
static void as7343_writeRegister(as7343_instance_t * const p_inst, uint8_t addr, uint8_t val) {

    uint8_t write_data[2];

    write_data[0] = addr;
    write_data[1]  = (uint8_t)val;
    as7343_write(p_inst, write_data, sizeof(write_data));
}

fsp_err_t as7343_enableFlickerDetection(as7343_instance_t * const p_inst, bool enable_fd) {

    uint8_t reg = AS7343_CFG6;
    return as7343_write_bit(p_inst, &reg, 2, 3, enable_fd);
}

/**
 * @brief Returns the flicker detection status
 *
 * @return int8_t
 */
int8_t as7343_getFlickerDetectStatus(as7343_instance_t * const p_inst) {
    rm_comms_write_read_params_t write_read_params;
    uint8_t reg[2];
    int8_t data = 0;

    reg[0] = AS7343_FD_STATUS;
    write_read_params.p_src      = reg;
    write_read_params.src_bytes  = 1;
    write_read_params.p_dest     = (uint8_t *)&data;
    write_read_params.dest_bytes = 1;
    as7343_read(p_inst, write_read_params);

    return data;
}

static void as7343_setSMUXLowChannels(as7343_instance_t * const p_inst, bool f1_f4) {
    AS7343_enableSpectralMeasurement(p_inst, false);
    as7343_setSMUXCommand(p_inst, AS7343_SMUX_CMD_WRITE);
    if (f1_f4) {
        AS7343_setup_F1F4_Clear_NIR(p_inst);
    } else {
        AS7343_setup_F5F8_Clear_NIR(p_inst);
    }
    as7343_enableSMUX(p_inst);
}

/**
 * @brief Configure SMUX for flicker detection
 *
 */
static void as7343_FDConfig(as7343_instance_t * const p_inst) {
    // SMUX Config for Flicker- register (0x13)left set to ADC6 for flicker
    // detection
    as7343_writeRegister(p_inst,0x00, 0x00); // disabled
    as7343_writeRegister(p_inst,0x01, 0x00); // disabled
    as7343_writeRegister(p_inst,0x02, 0x00); // reserved/disabled
    as7343_writeRegister(p_inst,0x03, 0x00); // disabled
    as7343_writeRegister(p_inst,0x04, 0x00); // disabled
    as7343_writeRegister(p_inst,0x05, 0x00); // disabled
    as7343_writeRegister(p_inst,0x06, 0x00); // disabled
    as7343_writeRegister(p_inst,0x07, 0x00); // disabled
    as7343_writeRegister(p_inst,0x08, 0x00); // disabled
    as7343_writeRegister(p_inst,0x09, 0x00); // disabled
    as7343_writeRegister(p_inst,0x0A, 0x00); // disabled
    as7343_writeRegister(p_inst,0x0B, 0x00); // Reserved or disabled
    as7343_writeRegister(p_inst,0x0C, 0x00); // disabled
    as7343_writeRegister(p_inst,0x0D, 0x00); // disabled
    as7343_writeRegister(p_inst,0x0E, 0x00); // disabled
    as7343_writeRegister(p_inst,0x0F, 0x00); // disabled
    as7343_writeRegister(p_inst,0x10, 0x00); // disabled
    as7343_writeRegister(p_inst,0x11, 0x00); // disabled
    as7343_writeRegister(p_inst,0x12, 0x00); // Reserved or disabled
    as7343_writeRegister(p_inst,0x13,
            0x60); // Flicker connected to ADC5 to left of 0x13
}

/*!  @brief Initializer for post i2c/spi init
 *   @param sensor_id Optional unique ID for the sensor set
 *   @returns True if chip identified and initialized
 */
bool as7343_init(as7343_instance_t * const p_inst)
{
    rm_comms_write_read_params_t write_read_params;
    fsp_err_t err;
    uint8_t reg;
    uint8_t chip_id;

    AS7343_setBank(p_inst, true);
    reg = AS7343_WHOAMI;
    write_read_params.p_src      = &reg;
    write_read_params.src_bytes  = 1;
    write_read_params.p_dest     = &chip_id;
    write_read_params.dest_bytes = 1;
    err = as7343_read(p_inst, write_read_params);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, false);
    AS7343_setBank(p_inst, false);
    if (chip_id != AS7343_CHIP_ID) {
        return false;
    }

    AS7343_powerEnable(p_inst, true);

    reg = AS7343_CFG20;
    if (as7343_write_bit(p_inst, &reg, 2, 5, 3) != FSP_SUCCESS)
    {
        return false;
    }
    return true;
}

/*******************************************************************************************************************//**
 * @brief Delay some microseconds.
 *
 * @retval FSP_SUCCESS              successfully configured.
 **********************************************************************************************************************/
static fsp_err_t as7343_delay_us (uint32_t const delay_us)
{
    /* Software delay */
    R_BSP_SoftwareDelay(delay_us, BSP_DELAY_UNITS_MICROSECONDS);

    return FSP_SUCCESS;
}

static fsp_err_t as7343_write_bit(as7343_instance_t * const p_inst, uint8_t *p_reg, uint8_t bits, uint8_t shift, uint32_t data)
{
    rm_comms_write_read_params_t write_read_params;
    fsp_err_t err;
    uint32_t read_data = 0;
    uint8_t write_data[2];

    write_read_params.p_src      = p_reg;
    write_read_params.src_bytes  = 1;
    write_read_params.p_dest     = (uint8_t *)&read_data;
    write_read_params.dest_bytes = 1;
    err = as7343_read(p_inst, write_read_params);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    // mask off the data before writing
    uint32_t mask = (uint32_t)((1 << (bits)) - 1);
    data &= mask;

    mask <<= shift;
    read_data &= (uint8_t)~mask;          // remove the current data at that spot
    read_data |= data << shift; // and add in the new data

    write_data[0] = *p_reg;
    write_data[1] = (uint8_t)read_data;
    return as7343_write(p_inst, write_data, sizeof(write_data));
}

static fsp_err_t as7343_read_bit(as7343_instance_t * const p_inst, uint8_t *p_reg, uint8_t bits, uint8_t shift, uint32_t *p_data)
{
    rm_comms_write_read_params_t write_read_params;
    fsp_err_t err;

    write_read_params.p_src      = p_reg;
    write_read_params.src_bytes  = 1;
    write_read_params.p_dest     = (uint8_t *)p_data;
    write_read_params.dest_bytes = 1;
    err = as7343_read(p_inst, write_read_params);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    *p_data >>= shift;
    *p_data =*p_data & (uint32_t)((1 << (bits)) - 1);

    return FSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief Read data from AS7331 device.
 *
 * @retval FSP_SUCCESS              Successfully started.
 * @retval FSP_ERR_TIMEOUT          communication is timeout.
 * @retval FSP_ERR_ABORTED          communication is aborted.
 **********************************************************************************************************************/
static fsp_err_t as7343_read (as7343_instance_t * const p_inst, rm_comms_write_read_params_t write_read_params)
{
    fsp_err_t err = FSP_SUCCESS;
    rm_comms_instance_t *p_i2c = (rm_comms_instance_t *) p_inst->p_i2c;
    /* WriteRead data */
    err = p_i2c->p_api->writeRead(p_i2c->p_ctrl, write_read_params);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

#if (BSP_CFG_RTOS == 0)
    uint16_t counter = 0;
    g_comm_i2c_flag = false;
    /* Wait callback */
    while (false == g_comm_i2c_flag)
    {
        as7343_delay_us(1000);
        counter++;
        FSP_ERROR_RETURN(AS7343_TIMEOUT >= counter, FSP_ERR_TIMEOUT);
    }
    /* Check callback event */
    FSP_ERROR_RETURN(RM_COMMS_EVENT_OPERATION_COMPLETE == g_comm_i2c_event, FSP_ERR_ABORTED);
#endif

    return FSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief Write data to AS7331 device.
 *
 * @retval FSP_SUCCESS              Successfully started.
 * @retval FSP_ERR_TIMEOUT          communication is timeout.
 * @retval FSP_ERR_ABORTED          communication is aborted.
 **********************************************************************************************************************/
static fsp_err_t as7343_write (as7343_instance_t * const p_inst, uint8_t * const p_src, uint8_t const bytes)
{
    fsp_err_t err = FSP_SUCCESS;
    rm_comms_instance_t *p_i2c = (rm_comms_instance_t *) p_inst->p_i2c;
    /* Write data */
    err = p_i2c->p_api->write(p_i2c->p_ctrl, p_src, (uint32_t) bytes);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);
#if (BSP_CFG_RTOS == 0)
    uint16_t counter = 0;
    g_comm_i2c_flag = false;
    /* Wait callback */
    while (false == g_comm_i2c_flag)
    {
        as7343_delay_us(1000);
        counter++;
        FSP_ERROR_RETURN(AS7343_TIMEOUT >= counter, FSP_ERR_TIMEOUT);
    }
    /* Check callback event */
    FSP_ERROR_RETURN(RM_COMMS_EVENT_OPERATION_COMPLETE == g_comm_i2c_event, FSP_ERR_ABORTED);
#endif

    return FSP_SUCCESS;
}

