/*
* Copyright (c) 2020 - 2024 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier:  BSD-3-Clause
*/
#ifndef TGS6810_SENSOR_H_
#define TGS6810_SENSOR_H_

#include "common_utils.h"
#if (BSP_CFG_RTOS) > 0
#include "sensor_thread.h"
#endif
#include "tgs6810/rm_tgs6810_api.h"
#include "tgs6810/rm_tgs6810.h"

void tgs6810_sensor_open(sm_handle * handle, uint8_t address, uint8_t channel);
void tgs6810_sensor_close(sm_handle handle);
sm_sensor_status tgs6810_sensor_read(sm_handle handle, int32_t * data);
uint8_t * tgs6810_sensor_get_flag(sm_handle handle);
sm_result tgs6810_sensor_set_attr(sm_handle handle, sm_sensor_attributes attr, uint32_t value);
sm_result tgs6810_sensor_get_attr(sm_handle handle, sm_sensor_attributes attr, uint32_t * value);

#endif /* TGS6810_SENSOR_H_ */

