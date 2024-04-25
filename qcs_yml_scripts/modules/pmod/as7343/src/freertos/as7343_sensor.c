#include "common_utils.h"
#include "as7343_sensor.h"

as7343_instance_t g_as7343 = {
		.p_i2c = &g_comms_i2c_as7343
};

void g_as7343_quick_setup(void)
{

	g_comms_i2c_as7343.p_api->open(g_comms_i2c_as7343.p_ctrl, g_comms_i2c_as7343.p_cfg);
	AS7343_begin(&g_as7343);

	AS7343_setATIME(&g_as7343, 100);
	AS7343_setASTEP(&g_as7343, 999);
	AS7343_setGain(&g_as7343, AS7343_GAIN_256X);
	AS7343_enableLED(&g_as7343, true);
}

/* Quick getting for as7331. */
void g_as7343_sensor0_read(uint16_t *p_as7343_readings)
{
	AS7343_readAllChannels(&g_as7343, p_as7343_readings);
}
