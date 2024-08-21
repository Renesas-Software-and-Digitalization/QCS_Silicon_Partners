/*
 * i2c.c
 *
 *  Created on: May 6, 2024
 *      Author: a5073464
 */
#include <stdio.h>
#include "common_utils.h"
#include "i2c.h"
// Uncomment the desired debug level
#include "log_disabled.h"
//#include "log_error.h"
//#include "log_warning.h"
//#include "log_info.h"
//#include "log_debug.h"

static bool init_done = false;

fsp_err_t i2c_initialize(void) {
    fsp_err_t status = FSP_ERR_ALREADY_OPEN;
    if (false == init_done) {
        i2c_master_instance_t * p_driver_instance = (i2c_master_instance_t *) g_comms_i2c_bus0_extended_cfg.p_driver_instance;
        /* Open I2C driver, this must be done before calling any COMMS API */
        status = p_driver_instance->p_api->open(p_driver_instance->p_ctrl, p_driver_instance->p_cfg);
        if (FSP_SUCCESS == status) {
            init_done = true;
        } else {
            log_error("I2C open error %d", status)
        }
    }
    return status;
}

void i2c_deinitialize(void) {
    fsp_err_t status = FSP_ERR_NOT_OPEN;
    if (true == init_done) {
        i2c_master_instance_t * p_driver_instance = (i2c_master_instance_t *) g_comms_i2c_bus0_extended_cfg.p_driver_instance;
        /* Close I2C driver */
        status = p_driver_instance->p_api->close(p_driver_instance->p_ctrl);
        if (FSP_SUCCESS == status) {
            init_done = false;
        } else {
            log_error("I2C open error %d", status)
        }       
    }
}
