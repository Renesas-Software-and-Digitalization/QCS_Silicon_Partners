#include "common_utils.h"
#include "as7331_sensor.h"

// Specify sensor parameters //
as7331_mode_t   mmode = AS7331_CONT_MODE;  // choices are modes are CONT, CMD, SYNS, SYND
as7331_cclk_t    cclk  = AS7331_1024;      // choices are 1.024, 2.048, 4.096, or 8.192 MHz
uint8_t sb    = 0x01;             // standby enabled 0x01 (to save power), standby disabled 0x00
uint8_t breakTime = 40;           // sample timeMs == 8 us x breaktimeMs (0 - 255, or 0 - 2040 us range), CONT or SYNX modes

uint8_t gain = 8; // ADCGain = 2^(11-gain), by 2s, 1 - 2048 range,  0 < gain = 11 max, default 10
uint8_t timeMs = 9; // 2^time in ms, so 0x07 is 2^6 = 64 ms, 0 < time = 15 max, default  6

void g_as7331_quick_setup(void)
{

    g_comms_i2c_as7331.p_api->open(g_comms_i2c_as7331.p_ctrl, g_comms_i2c_as7331.p_cfg);
    AS7331_power_up(&g_comms_i2c_as7331);
    AS7331_reset(&g_comms_i2c_as7331);
    R_BSP_SoftwareDelay(100, BSP_DELAY_UNITS_MICROSECONDS);
    uint8_t AS7331_ID = AS7331_get_chip_id(&g_comms_i2c_as7331);
    //APP_PRINT("ID: %d\n", AS7331_ID);

    // check if AS7331 has acknowledged
    if(AS7331_ID == 0x21)  {
        AS7331_setConfigurationMode(&g_comms_i2c_as7331);
        AS7331_init(&g_comms_i2c_as7331,mmode, cclk, sb, breakTime, gain, timeMs);
        R_BSP_SoftwareDelay(100, BSP_DELAY_UNITS_MICROSECONDS);
        AS7331_setMeasurementMode(&g_comms_i2c_as7331);
    }
}

/* Quick getting humidity and temperature for g_hs300x_sensor0. */
void g_as7331_sensor0_read(float *p_temp, float UV_data[3])
{
    fsp_err_t err = FSP_SUCCESS;
    uint16_t allData[4] = {0, 0, 0, 0};
    uint16_t status;
    // sensitivities at 1.024 MHz clock
    float lsbA = 304.69f / ((float)(1 << (11 - gain))) / ((float)(1 << timeMs)/1024.0f) / 1000.0f;  // uW/cm^2
    float lsbB = 398.44f / ((float)(1 << (11 - gain))) / ((float)(1 << timeMs)/1024.0f) / 1000.0f;
    float lsbC = 191.41f / ((float)(1 << (11 - gain))) / ((float)(1 << timeMs)/1024.0f) / 1000.0f;
    err = AS7331_getStatus(&g_comms_i2c_as7331, &status);
    if(err != FSP_SUCCESS)
    {
        __BKPT(0);
    }

    if (status & 0x0008) {
        err = AS7331_readAllData(&g_comms_i2c_as7331, allData);
        if(err != FSP_SUCCESS)
        {
            __BKPT(0);
        }
        *p_temp = allData[0] * 0.05f - 66.9f;
        UV_data[0] = (allData[1] * lsbA * 100 );
        UV_data[1] = (allData[2] * lsbB * 100 );
        UV_data[2] = (allData[3] * lsbC * 100 );
    }
}
