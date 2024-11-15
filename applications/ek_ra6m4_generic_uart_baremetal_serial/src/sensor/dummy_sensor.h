/*
* Copyright (c) 2020 - 2024 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier:  BSD-3-Clause
*
* This file demonstrates a sensor implementation over I2C, it performs a direct read and the sensor
* is expected to reply with data without the need of additional transactions (standard I2C transaction).
* There are two channels: temperature (channel 0) and humidity (channel 1).
* If a sensor needs multiple transactions to perform a read (ie.: one transaction to start the read and another
* for retrieving data) then we recommend using a state machine to handle it. For details please refer to
* hs3001_sensor_fsm() in hs3001_sensor.c.
*/
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
