/*
* Copyright (c) 2020 - 2024 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier:  BSD-3-Clause
*/
#ifndef __SM_H
#define __SM_H
#include <sm_handle.h>
#include <stdint.h>

#define IS_HANDLE_VALID(X) (X.value != 0)

typedef enum {
    #define DEFINE_SENSOR_TYPE(TYPE, UNIT, PATH) TYPE,
    #include "sm_define_sensors.inc"
    TOTAL_SENSOR_TYPES,
    SENSOR_ANY_TYPE
} sm_type;

typedef enum {
    SM_DRIVER_FIRST,
    #define DEFINE_SENSOR_DRIVER(NAME) DRIVER_##NAME,
    #include "sm_define_sensors.inc"
    SM_DRIVER_LAST
} sm_driver;

// The following anonymous enum is a simple way to get the total number of sensors (NUM_SENSORS) in the system!
enum {
  #define DEFINE_SENSOR_INSTANCE(type, address, channel, driver, multiplier, divider, offset, interval_ms) ______x##type##address##channel##driver,
  #include "sm_define_sensors.inc"
  NUM_SENSORS
};

typedef struct {
  int32_t multiplier;
  int32_t divider;
  int32_t offset;
} sm_scaling;

typedef void (* sm_callback)(sm_handle,  uint8_t * , uint16_t);

typedef struct {
  sm_handle handle;
  int32_t data;
} sm_sensor_data;

/************************************************************************
  Sensor Manager - Application interface
************************************************************************/

/*******************************************************************************************************************//**
 * @brief       Initialize SM
 * @param[in]   none
 * @retval      none
 ***********************************************************************************************************************/
void sm_init(void);
/*******************************************************************************************************************//**
 * @brief       Run SM state-machine (non-blocking, must be called periodically in order to keep timing)
 * @param[in]   none
 * @retval      none
 ***********************************************************************************************************************/
void sm_run(void);
/*******************************************************************************************************************//**
 * @brief       Return the total number of sensors in the system
 * @param[in]   none
 * @retval      number of sensors
 ***********************************************************************************************************************/
uint16_t sm_get_total_sensor_count(void);
/*******************************************************************************************************************//**
 * @brief       Get a handle for a sensor of a specific type. Each call returns the next handle for that sensor type
 *              until no more handles are found and an INVALID_HANDLE is returned.
 *              A call to this function with a different type is only allowed following a sm_init call or once an
 *              INVALID_HANDLE is returned.
 * @param[in]   sensor type
 * @param[out]  pointer to a handle
 * @param[in]   pointer to an auxiliary variable for storing internal search index
 * @retval      0 if a valid handle is returned, -1 otherwise
 ***********************************************************************************************************************/
int32_t sm_get_sensor_handle(sm_type type, sm_handle * handle, uint16_t * index);
/*******************************************************************************************************************//**
 * @brief       Get the sensor driver id
 * @param[in]   handle of the desired sensor
 * @retval      pointer to a constant string with the sensor id
 ***********************************************************************************************************************/
const char * sm_get_sensor_id(sm_handle handle);
/*******************************************************************************************************************//**
 * @brief       Get scaling factors for a sensor
 * @param[in]   handle of the desired sensor
 * @param[out]  pointer to a variable to store scaling data (sm_scaling type)
 * @retval      none
 ***********************************************************************************************************************/
void sm_get_sensor_scaling(sm_handle handle, sm_scaling *scaling);
/*******************************************************************************************************************//**
 * @brief       Read the latest data acquired by the sensor specified by the handle
 * @param[in]   handle of the sensor to read
 * @param[in]   signed 32-bit sensor reading
 * @retval      sm_sensor_status
 ***********************************************************************************************************************/
 sm_sensor_status sm_read_sensor(sm_handle handle, int32_t * data);
 /*******************************************************************************************************************//**
 * @brief       Get the type of the sensor specified by the handle
 * @param[in]   handle of the sensor
 * @retval      sensor type (sm_type) or SENSOR_ANY_TYPE if handle is invalid
 ***********************************************************************************************************************/
sm_type sm_get_sensor_type_by_handle(sm_handle handle);
/*******************************************************************************************************************//**
 * @brief       Register a callback to be used by the sensor specified by the handle
 * @param[in]   handle of the desired sensor
 * @param[in]   function pointer to the desired callback (using sm_callback type)
 * @retval      none
 ***********************************************************************************************************************/
void sm_register_callback_by_handle(sm_handle handle, sm_callback);
/*******************************************************************************************************************//**
 * @brief       Register a callback to be used by all sensors of the specified type
 * @param[in]   sensor type to be assigned to the callback
 * @param[in]   function pointer to the desired callback (using sm_callback type)
 * @retval      number of instances using the callback
 ***********************************************************************************************************************/
uint16_t sm_register_callback_by_type(sm_type type, sm_callback callback);
/*******************************************************************************************************************//**
 * @brief       Register a single callback to be used by all sensors in the system
 * @param[in]   function pointer to the desired callback (using sm_callback type)
 * @retval      number of instances using the callback
 ***********************************************************************************************************************/
uint16_t sm_register_callback_any_type(sm_callback callback);
/*******************************************************************************************************************//**
 * @brief       Get sensor unit using a sensor type
 * @param[in]   sensor type
 * @retval      pointer to a constant string with the sensor unit
 ***********************************************************************************************************************/
const char * sm_get_sensor_unit_by_type(sm_type type);
/*******************************************************************************************************************//**
 * @brief       Get the sensor unit using a sensor handle
 * @param[in]   handle of the desired sensor
 * @retval      pointer to a constant string with the sensor unit
 ***********************************************************************************************************************/
const char * sm_get_sensor_unit_by_handle(sm_handle handle);
/*******************************************************************************************************************//**
 * @brief       Get sensor path using a sensor type
 * @param[in]   sensor type
 * @retval      pointer to a constant string with the sensor path (topic), it does not include sensor id
 ***********************************************************************************************************************/
const char * sm_get_sensor_path_by_type(sm_type type);
/*******************************************************************************************************************//**
 * @brief       Get the sensor path using a sensor handle
 * @param[in]   handle of the desired sensor
 * @retval      pointer to a constant string with the sensor path (topic), it does not include sensor id
 ***********************************************************************************************************************/
const char * sm_get_sensor_path_by_handle(sm_handle handle);
/*******************************************************************************************************************//**
 * @brief       Set a sensor attribute
 * @param[in]   handle of the desired sensor
 * @param[in]   attribute that will be set
 * @param[in]   value to be assigned to the attribute
 * @retval      one of sm_result
 ***********************************************************************************************************************/
sm_result sm_set_sensor_attribute(sm_handle handle, sm_sensor_attributes attr, uint32_t value);
/*******************************************************************************************************************//**
 * @brief       Get a sensor attribute
 * @param[in]   handle of the desired sensor
 * @param[in]   attribute that will be set
 * @param[in]   pointer to uint32_t that will store the value of the attribute
 * @retval      one of sm_result
 ***********************************************************************************************************************/
sm_result sm_get_sensor_attribute(sm_handle handle, sm_sensor_attributes attr, uint32_t * value);

#endif
