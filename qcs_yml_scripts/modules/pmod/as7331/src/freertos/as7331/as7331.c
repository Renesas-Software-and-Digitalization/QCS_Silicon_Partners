/*
 * as7331.c
 *
 *  Created on: Nov 13, 2023
 *      Author:
 */

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "as7331.h"

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

#define AS7331_TIMEOUT                            (1000)
/*
 * https://www.mouser.com/catalog/specsheets/amsOsram_AS7331_DS001047_1-00.pdf
 */
// Configuration State registers
#define AS7331_OSR                      0x00
#define AS7331_AGEN                     0x02
#define AS7331_CREG1                    0x06
#define AS7331_CREG2                    0x07
#define AS7331_CREG3                    0x08
#define AS7331_BREAK                    0x09
#define AS7331_EDGES                    0x0A
#define AS7331_OPTREG                   0x0B

// Measurement State registers
#define AS7331_STATUS                   0x00
#define AS7331_TEMP                     0x01
#define AS7331_MRES1                    0x02
#define AS7331_MRES2                    0x03
#define AS7331_MRES3                    0x04
#define AS7331_OUTCONVL                 0x05
#define AS7331_OUTCONVH                 0x06

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
static fsp_err_t as7331_write (const rm_comms_instance_t *  p_i2c, uint8_t * const p_src, uint8_t const bytes);
static fsp_err_t as7331_read (const rm_comms_instance_t *  p_i2c, rm_comms_write_read_params_t write_read_params);
static fsp_err_t as7331_delay_us (uint32_t const delay_us);
void as7331_callback(rm_comms_callback_args_t *p_args);
/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/


/***********************************************************************************************************************
 * Global variables
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Functions
 **********************************************************************************************************************/
fsp_err_t AS7331_oneShot(const rm_comms_instance_t *  p_i2c)
{
    rm_comms_write_read_params_t write_read_params;
    fsp_err_t err;

    uint8_t reg;
    uint8_t read_data;
    uint8_t write_data[2];
    reg = AS7331_OSR;

    write_read_params.p_src      = &reg;
    write_read_params.src_bytes  = 1;
    write_read_params.p_dest     = &read_data;
    write_read_params.dest_bytes = 1;
    err = as7331_read(p_i2c, write_read_params);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    write_data[0] = reg;
    write_data[1] = (uint8_t)(read_data | 0x80);
    return as7331_write(p_i2c, write_data, sizeof(write_data));
}

fsp_err_t AS7331_getStatus(const rm_comms_instance_t *  p_i2c, uint16_t *p_status)
{
    rm_comms_write_read_params_t write_read_params;
    fsp_err_t err;
    uint8_t reg;
    uint8_t read_data[2];
    reg = AS7331_STATUS;

    write_read_params.p_src      = &reg;
    write_read_params.src_bytes  = 1;
    write_read_params.p_dest     = &read_data[0];
    write_read_params.dest_bytes = 2;
    err = as7331_read(p_i2c, write_read_params);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    *p_status = (uint16_t)(((uint16_t)read_data[0]) << 8 | read_data[1]);

    return FSP_SUCCESS;
}

fsp_err_t AS7331_readAllData(const rm_comms_instance_t *  p_i2c, uint16_t data[4])
{
    rm_comms_write_read_params_t write_read_params;
    fsp_err_t err;
    uint8_t reg;
    uint8_t read_data[8];

    reg = AS7331_TEMP;
    write_read_params.p_src      = &reg;
    write_read_params.src_bytes  = 1;
    write_read_params.p_dest     = &read_data[0];
    write_read_params.dest_bytes = 8;
    err = as7331_read(p_i2c, write_read_params);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    data[0] =  (uint16_t)(((uint16_t)read_data[1]) << 8 | read_data[0]);
    data[1]  =  (uint16_t)(((uint16_t)read_data[3]) << 8 | read_data[2]);
    data[2]  =  (uint16_t)(((uint16_t)read_data[5]) << 8 | read_data[4]);
    data[3]  =  (uint16_t)(((uint16_t)read_data[7]) << 8 | read_data[6]);

    return FSP_SUCCESS;
}
/*
Input light irradiance regarding the photodiode’s area within the conversion time interval
 */
fsp_err_t AS7331_readTempData(const rm_comms_instance_t * const p_i2c, uint16_t *p_data)
{
    rm_comms_write_read_params_t write_read_params;
    fsp_err_t err;
    uint8_t reg;
    uint8_t read_data[2];

    reg = AS7331_TEMP;
    write_read_params.p_src      = &reg;
    write_read_params.src_bytes  = 1;
    write_read_params.p_dest     = &read_data[0];
    write_read_params.dest_bytes = 2;
    err = as7331_read(p_i2c, write_read_params);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    *p_data = (uint16_t)(((uint16_t)read_data[0]) << 8 | read_data[1]);

    return FSP_SUCCESS;
}

fsp_err_t AS7331_readUVAData(const rm_comms_instance_t * const p_i2c, uint16_t *p_data)
{
    rm_comms_write_read_params_t write_read_params;
    fsp_err_t err;
    uint8_t reg;
    uint8_t read_data[2];

    reg = AS7331_MRES1;
    write_read_params.p_src      = &reg;
    write_read_params.src_bytes  = 1;
    write_read_params.p_dest     = &read_data[0];
    write_read_params.dest_bytes = 2;
    err = as7331_read(p_i2c, write_read_params);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    *p_data = (uint16_t)(((uint16_t)read_data[0]) << 8 | read_data[1]);

    return FSP_SUCCESS;
}

fsp_err_t AS7331_readUVBData(const rm_comms_instance_t * const p_i2c, uint16_t *p_data)
{
    rm_comms_write_read_params_t write_read_params;
    fsp_err_t err;
    uint8_t reg;
    uint8_t read_data[2];

    reg = AS7331_MRES2;
    write_read_params.p_src      = &reg;
    write_read_params.src_bytes  = 1;
    write_read_params.p_dest     = &read_data[0];
    write_read_params.dest_bytes = 2;
    err = as7331_read(p_i2c, write_read_params);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    *p_data = (uint16_t)(((uint16_t)read_data[0]) << 8 | read_data[1]);

    return FSP_SUCCESS;
}

fsp_err_t AS7331_readUVCData(const rm_comms_instance_t * const p_i2c, uint16_t *p_data)
{
    rm_comms_write_read_params_t write_read_params;
    fsp_err_t err;
    uint8_t reg;
    uint8_t read_data[2];

    reg = AS7331_MRES3;
    write_read_params.p_src      = &reg;
    write_read_params.src_bytes  = 1;
    write_read_params.p_dest     = &read_data[0];
    write_read_params.dest_bytes = 2;
    err = as7331_read(p_i2c, write_read_params);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    *p_data = (uint16_t)(((uint16_t)read_data[0]) << 8 | read_data[1]);

    return FSP_SUCCESS;
}

fsp_err_t AS7331_power_down(const rm_comms_instance_t * const p_i2c)
{
    rm_comms_write_read_params_t write_read_params;
    fsp_err_t err;
    uint8_t reg;
    uint8_t read_data;
    uint8_t write_data[2];
    reg = AS7331_OSR;

    write_read_params.p_src      = &reg;
    write_read_params.src_bytes  = 1;
    write_read_params.p_dest     = &read_data;
    write_read_params.dest_bytes = 1;
    err = as7331_read(p_i2c, write_read_params);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    write_data[0] = reg;
    write_data[1] = (uint8_t)(read_data & ~(0x40));
    return as7331_write(p_i2c, write_data, sizeof(write_data));
}

fsp_err_t AS7331_power_up(const rm_comms_instance_t *  p_i2c)
{
    rm_comms_write_read_params_t write_read_params;
    fsp_err_t err;
    uint8_t reg;
    uint8_t read_data;
    uint8_t write_data[2];
    reg = AS7331_OSR;

    write_read_params.p_src      = &reg;
    write_read_params.src_bytes  = 1;
    write_read_params.p_dest     = &read_data;
    write_read_params.dest_bytes = 1;
    err = as7331_read(p_i2c, write_read_params);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    write_data[0] = reg;
    write_data[1] = (uint8_t)(read_data | 0x40);
    return as7331_write(p_i2c, write_data, sizeof(write_data));
}

fsp_err_t AS7331_reset(const rm_comms_instance_t *  p_i2c)
{
    rm_comms_write_read_params_t write_read_params;
    fsp_err_t err;
    uint8_t reg;
    uint8_t read_data;
    uint8_t write_data[2];
    reg = AS7331_OSR;

    write_read_params.p_src      = &reg;
    write_read_params.src_bytes  = 1;
    write_read_params.p_dest     = &read_data;
    write_read_params.dest_bytes = 1;
    err = as7331_read(p_i2c, write_read_params);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    write_data[0] = reg;
    write_data[1] = (uint8_t)(read_data | 0x08);
    return as7331_write(p_i2c, write_data, sizeof(write_data));
}

uint8_t AS7331_get_chip_id(const rm_comms_instance_t *  p_i2c)
{
    rm_comms_write_read_params_t write_read_params;
    fsp_err_t err;
    uint8_t reg;
    uint8_t temp;
    reg = AS7331_AGEN;

    write_read_params.p_src      = &reg;
    write_read_params.src_bytes  = 1;
    write_read_params.p_dest     = &temp;
    write_read_params.dest_bytes = 1;
    err = as7331_read(p_i2c, write_read_params);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    return temp;
}

fsp_err_t AS7331_setConfigurationMode(const rm_comms_instance_t *  p_i2c)
{
    rm_comms_write_read_params_t write_read_params;
    fsp_err_t err;
    uint8_t reg;
    uint8_t read_data;
    uint8_t write_data[2];
    reg = AS7331_OSR;

    write_read_params.p_src      = &reg;
    write_read_params.src_bytes  = 1;
    write_read_params.p_dest     = &read_data;
    write_read_params.dest_bytes = 1;
    err = as7331_read(p_i2c, write_read_params);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    write_data[0] = reg;
    write_data[1] =  (uint8_t)(read_data | 0x02);
    return as7331_write(p_i2c, write_data, sizeof(write_data));
}

fsp_err_t AS7331_setMeasurementMode(const rm_comms_instance_t *  p_i2c)
{
    rm_comms_write_read_params_t write_read_params;
    fsp_err_t err;
    uint8_t reg;
    uint8_t read_data;
    uint8_t write_data[2];
    reg = AS7331_OSR;

    write_read_params.p_src      = &reg;
    write_read_params.src_bytes  = 1;
    write_read_params.p_dest     = &read_data;
    write_read_params.dest_bytes = 1;
    err = as7331_read(p_i2c, write_read_params);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    write_data[0] = reg;
    write_data[1] =  (uint8_t)(read_data | 0x83);
    return as7331_write(p_i2c, write_data, sizeof(write_data));
}

fsp_err_t  AS7331_init(const rm_comms_instance_t *  p_i2c, uint8_t mmode, uint8_t cclk, uint8_t sb, uint8_t breakTime, uint8_t gain, uint8_t time)
{
    //   set measurement mode (bits 6,7), standby on/off (bit 4)
    //   and internal clk (bits 0,1); bit 3 determines ready interrupt configuration, 0 means push pull
    //   1 means open drain
    fsp_err_t err = FSP_SUCCESS;
    uint8_t write_data[2];

    write_data[0] = AS7331_CREG1;
    write_data[1]  = ((gain << 4) |  (time));
    err = as7331_write(p_i2c, write_data, sizeof(write_data));
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    write_data[0] = AS7331_CREG3;
    write_data[1]  = ((mmode << 6) | (sb << 4) |  (sb << 4) |  (cclk));
    err = as7331_write(p_i2c, write_data, sizeof(write_data));
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    write_data[0] = AS7331_BREAK;
    write_data[1]  = breakTime;
    err = as7331_write(p_i2c, write_data, sizeof(write_data));
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    return err;
}

/*******************************************************************************************************************//**
 * @brief AS7331 callback function called in the I2C Communications Middleware callback function.
 **********************************************************************************************************************/
void as7331_callback (rm_comms_callback_args_t * p_args)
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

/*******************************************************************************************************************//**
 * @brief Delay some microseconds.
 *
 * @retval FSP_SUCCESS              successfully configured.
 **********************************************************************************************************************/
static fsp_err_t as7331_delay_us (uint32_t const delay_us)
{
    /* Software delay */
    R_BSP_SoftwareDelay(delay_us, BSP_DELAY_UNITS_MICROSECONDS);

    return FSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief Read data from AS7331 device.
 *
 * @retval FSP_SUCCESS              Successfully started.
 * @retval FSP_ERR_TIMEOUT          communication is timeout.
 * @retval FSP_ERR_ABORTED          communication is aborted.
 **********************************************************************************************************************/
static fsp_err_t as7331_read (const rm_comms_instance_t *  p_i2c, rm_comms_write_read_params_t write_read_params)
{
    fsp_err_t err = FSP_SUCCESS;

    /* WriteRead data */
    err = p_i2c->p_api->writeRead(p_i2c->p_ctrl, write_read_params);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

#if (BSP_CFG_RTOS == 0)
    uint16_t counter = 0;
    g_comm_i2c_flag = false;
    /* Wait callback */
    while (false == g_comm_i2c_flag)
    {
        as7331_delay_us(1000);
        counter++;
        FSP_ERROR_RETURN(AS7331_TIMEOUT >= counter, FSP_ERR_TIMEOUT);
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
static fsp_err_t as7331_write (const rm_comms_instance_t *  p_i2c, uint8_t * const p_src, uint8_t const bytes)
{
    fsp_err_t err = FSP_SUCCESS;

    /* Write data */
    err = p_i2c->p_api->write(p_i2c->p_ctrl, p_src, (uint32_t) bytes);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);
#if (BSP_CFG_RTOS == 0)
    uint16_t counter = 0;
    g_comm_i2c_flag = false;
    /* Wait callback */
    while (false == g_comm_i2c_flag)
    {
        as7331_delay_us(1000);
        counter++;
        FSP_ERROR_RETURN(AS7331_TIMEOUT >= counter, FSP_ERR_TIMEOUT);
    }
    /* Check callback event */
    FSP_ERROR_RETURN(RM_COMMS_EVENT_OPERATION_COMPLETE == g_comm_i2c_event, FSP_ERR_ABORTED);
#endif

    return FSP_SUCCESS;
}

