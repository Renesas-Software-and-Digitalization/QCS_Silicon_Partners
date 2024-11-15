/*
* Copyright (c) 2020 - 2024 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier:  BSD-3-Clause
*/
#ifndef FECS44_SENSOR_H_
#define FECS44_SENSOR_H_

#include "common_utils.h"
#if (BSP_CFG_RTOS) > 0
#include "sensor_thread.h"
#endif
#include "fecs44/rm_fecs44_api.h"
#include "fecs44/rm_fecs44.h"

void fecs44_sensor_open(sm_handle * handle, uint8_t address, uint8_t channel);
void fecs44_sensor_close(sm_handle handle);
sm_sensor_status fecs44_sensor_read(sm_handle handle, int32_t * data);
uint8_t * fecs44_sensor_get_flag(sm_handle handle);
sm_result fecs44_sensor_set_attr(sm_handle handle, sm_sensor_attributes attr, uint32_t value);
sm_result fecs44_sensor_get_attr(sm_handle handle, sm_sensor_attributes attr, uint32_t * value);

#endif /* FECS44_SENSOR_H_ */

