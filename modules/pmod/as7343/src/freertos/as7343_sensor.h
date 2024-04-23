#ifndef AS7343_SENSOR_H_
#define AS7343_SENSOR_H_

#include "common_utils.h"
#include "sensor_thread.h"
#include "as7343/as7343.h"

void g_as7343_quick_setup(void);
void g_as7343_sensor0_read(uint16_t *p_as7343_readings);

#endif /* AS7331_SENSOR_H_ */
