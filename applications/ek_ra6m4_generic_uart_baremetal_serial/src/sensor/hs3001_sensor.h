#ifndef __HS3001_SENSOR_H
#define __HS3001_SENSOR_H
#include <stdint.h>
#include "sm_handle.h"

void hs3001_sensor_open(sm_handle * handle, uint8_t address, uint8_t channel);
void hs3001_sensor_close(sm_handle handle);
sm_sensor_status hs3001_sensor_read(sm_handle handle, int32_t * data);
void hs3001_sensor_fsm(void);
uint8_t * hs3001_sensor_get_flag(sm_handle handle);
sm_result hs3001_sensor_set_attr(sm_handle handle, sm_sensor_attributes attr, uint32_t value);
sm_result hs3001_sensor_get_attr(sm_handle handle, sm_sensor_attributes attr, uint32_t * value);

#endif
