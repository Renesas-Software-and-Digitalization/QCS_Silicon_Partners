#ifndef AS7331_SENSOR_H_
#define AS7331_SENSOR_H_

#include "common_utils.h"
#include "sensor_thread.h"
#include "as7331/as7331.h"

void g_as7331_quick_setup(void);
void g_as7331_sensor0_read(float *p_temp, float UV_data[3]);

#endif /* AS7331_SENSOR_H_ */
