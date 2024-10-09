/*
* Copyright (c) 2020 - 2024 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier:  BSD-3-Clause
*/
#ifndef TGS5141_SENSOR_H_
#define TGS5141_SENSOR_H_

#include "common_utils.h"
#if (BSP_CFG_RTOS) > 0
#include "sensor_thread.h"
#endif
#include "tgs5141/rm_tgs5141_api.h"
#include "tgs5141/rm_tgs5141.h"

void tgs5141_sensor_open(sm_handle * handle, uint8_t address, uint8_t channel);
void tgs5141_sensor_close(sm_handle handle);
sm_sensor_status tgs5141_sensor_read(sm_handle handle, int32_t * data);
uint8_t * tgs5141_sensor_get_flag(sm_handle handle);
sm_result tgs5141_sensor_set_attr(sm_handle handle, sm_sensor_attributes attr, uint32_t value);
sm_result tgs5141_sensor_get_attr(sm_handle handle, sm_sensor_attributes attr, uint32_t * value);

#endif /* TGS5141_SENSOR_H_ */

