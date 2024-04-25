#ifndef AS7331_SENSOR_H_
#define AS7331_SENSOR_H_

#include "common_utils.h"
#include "icm42688/inv_icm426xx_api.h"
#include "icm42688/inv_icm426xx.h"

void g_icm42688_quick_setup(void);
void g_icm42688_sensor0_read(float *p_temp, float acc_data[3], float gyro_data[3]);

#endif /* AS7331_SENSOR_H_ */
