#ifndef __DUMMY_DRIVER_H
#define __DUMMY_DRIVER_H
#include "hal_data.h"
#include <stdint.h>

// These are some pretend registers...
#define DUMMY_PWR_CTRL          (0x01)
#define DUMMY_TEMP_LO           (0x02)
#define DUMMY_TEMP_HI   	    (0x03)
#define DUMMY_HUMI_LO           (0x04)
#define DUMMY_HUMI_HI           (0x05)
#define DUMMY_POWER_ON          (0x02)

fsp_err_t dummy_readReg(uint8_t reg, uint8_t *p_data);
fsp_err_t dummy_writeReg(uint8_t reg, uint8_t data);
fsp_err_t dummy_read(rm_comms_write_read_params_t write_read_params);
fsp_err_t dummy_write(uint8_t * const p_src, uint8_t const bytes);

#endif
