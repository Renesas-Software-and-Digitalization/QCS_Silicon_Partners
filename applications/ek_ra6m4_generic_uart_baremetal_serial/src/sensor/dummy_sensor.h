#ifndef __DUMMY_SENSOR_H
#define __DUMMY_SENSOR_H
#include <stdint.h>
#include "sm_handle.h"

void dummy_sensor_open(sm_handle * handle, uint8_t address, uint8_t channel);
void dummy_sensor_close(sm_handle handle);
sm_sensor_status dummy_sensor_read(sm_handle handle, int32_t * data);
sm_result dummy_sensor_set_attr(sm_handle handle, sm_sensor_attributes attr, uint32_t value);
sm_result dummy_sensor_get_attr(sm_handle handle, sm_sensor_attributes attr, uint32_t * value);

#endif
