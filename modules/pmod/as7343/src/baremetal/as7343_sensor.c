#include "common_utils.h"
#include "as7343_sensor.h"

as7343_instance_t g_as7343 = {
		.p_i2c = &g_comms_i2c_as7343
};


enum{
	SENSOR_IDLE = 0,
	SENSOR_START_MEASURING,
	SENSOR_MEASURE_READY,
};

uint8_t app_state = SENSOR_IDLE;

void g_as7343_quick_setup(void)
{

	g_comms_i2c_as7343.p_api->open(g_comms_i2c_as7343.p_ctrl, g_comms_i2c_as7343.p_cfg);
	AS7343_begin(&g_as7343);

	AS7343_setATIME(&g_as7343, 100);
	AS7343_setASTEP(&g_as7343, 999);
	AS7343_setGain(&g_as7343, AS7343_GAIN_256X);
	AS7343_enableLED(&g_as7343, true);
	AS7343_setLEDCurrent(&g_as7343, 15);
}

/* Quick getting for as7331. */
bool g_as7343_sensor0_read(uint16_t *p_as7343_readings)
{
	bool ret = false;
	if (app_state == SENSOR_IDLE)
	{
		AS7343_startReading(&g_as7343);
		app_state = SENSOR_START_MEASURING;
	}
	else if (app_state == SENSOR_START_MEASURING)
	{
		if (AS7343_checkReadingProgress(&g_as7343) == true)
		{
			app_state = SENSOR_MEASURE_READY;
		}
	}
	else if (app_state == SENSOR_MEASURE_READY)
	{
        AS7343_getAllChannels(p_as7343_readings);
		app_state = SENSOR_IDLE;
		ret = true;
	}
	return ret;
}
