/*
* Copyright (c) 2020 - 2024 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier:  BSD-3-Clause
*/
#ifndef __PUBLIC_HANDLE_H
#define __PUBLIC_HANDLE_H

#include <stdint.h>

typedef union {
  uint32_t value;
  __attribute__((packed)) struct {
    uint8_t channel;
    uint8_t address;
    uint16_t internal;
  };
} sm_handle;

#define INVALID_HANDLE_INIT {.value=0}

enum {
  SM_CH0 = 0, SM_CH1, SM_CH2, SM_CH3,
  SM_CH4, SM_CH5, SM_CH6, SM_CH7,
  SM_CH8, SM_CH9, SM_CH10, SM_CH11,
  SM_CH12, SM_CH13, SM_CH14, SM_CH15,
};

typedef enum {
  SM_SENSOR_INVALID_DATA,
  SM_SENSOR_DATA_VALID,
  SM_SENSOR_STALE_DATA,
  SM_SENSOR_ERROR
} sm_sensor_status;

typedef enum {
  SM_SAMPLE_INTERVAL,		// milliseconds between each sample reading
  SM_ACQUISITION_INTERVAL,	// milliseconds between each sample publishing
} sm_sensor_attributes;

typedef enum {
  SM_OK,
  SM_ERROR,
  SM_NOT_SUPPORTED
} sm_result;

#endif
