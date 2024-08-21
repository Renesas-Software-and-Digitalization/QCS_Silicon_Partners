#include <stdint.h>
#include "common_utils.h"
#include "sm.h"
// Uncomment the desired debug level
#include "log_disabled.h"
//#include "log_error.h"
//#include "log_warning.h"
//#include "log_info.h"
//#include "log_debug.h"


#if (BSP_CFG_RTOS) == 1
#include "tx_api.h"
#define QUEUE_TYPE TX_QUEUE
#elif (BSP_CFG_RTOS) == 2
// On FreeRTOS Sensor Manager uses a queue to pass data to the application
#include "FreeRTOS.h"
#include "queue.h"
#define QUEUE_TYPE QueueHandle_t
static StaticQueue_t sensor_queue_memory;
#endif

#if (BSP_CFG_RTOS) != 0
#define SENSOR_QUEUE_LENGTH     (20U * NUM_SENSORS)
#define SENSOR_SEND_WAIT_TIME   (10)    // Ticks
QUEUE_TYPE g_sensor_queue;
static uint8_t sensor_queue_storage[SENSOR_QUEUE_LENGTH * sizeof(sm_sensor_data)];
#endif

static uint8_t always_zero = 0;

// create automatic headers for each sensor driver
#define DEFINE_SENSOR_DRIVER(DRIVER) void DRIVER##_open(sm_handle * handle, uint8_t address, uint8_t channel);
#include "sm_define_sensors.inc"
#define DEFINE_SENSOR_DRIVER(DRIVER) void DRIVER##_close(sm_handle handle);
#include "sm_define_sensors.inc"
#define DEFINE_SENSOR_DRIVER(DRIVER) sm_sensor_status DRIVER##_read(sm_handle handle, int32_t * data);
#include "sm_define_sensors.inc"
#define DEFINE_SENSOR_DRIVER(DRIVER) void BSP_WEAK_REFERENCE DRIVER##_fsm(void);
#include "sm_define_sensors.inc"
#define DEFINE_SENSOR_DRIVER(DRIVER) void BSP_WEAK_REFERENCE DRIVER##_fsm(void) {}
#include "sm_define_sensors.inc"
#define DEFINE_SENSOR_DRIVER(DRIVER) uint8_t * BSP_WEAK_REFERENCE DRIVER##_get_flag(sm_handle handle);
#include "sm_define_sensors.inc"
#define DEFINE_SENSOR_DRIVER(DRIVER) uint8_t * BSP_WEAK_REFERENCE DRIVER##_get_flag(sm_handle handle)\
	{FSP_PARAMETER_NOT_USED(handle);return &always_zero;}
#include "sm_define_sensors.inc"
#define DEFINE_SENSOR_DRIVER(DRIVER) void BSP_WEAK_REFERENCE DRIVER##_reset(void);
#include "sm_define_sensors.inc"
#define DEFINE_SENSOR_DRIVER(DRIVER) void BSP_WEAK_REFERENCE DRIVER##_reset(void) {}
#include "sm_define_sensors.inc"
#define DEFINE_SENSOR_DRIVER(DRIVER) sm_result BSP_WEAK_REFERENCE DRIVER##_set_attr(	sm_handle handle,\
																						sm_sensor_attributes attr,\
																						uint32_t value);
#include "sm_define_sensors.inc"
#define DEFINE_SENSOR_DRIVER(DRIVER) sm_result BSP_WEAK_REFERENCE DRIVER##_set_attr(	sm_handle handle,\
																						sm_sensor_attributes attr,\
																						uint32_t value) {\
																							FSP_PARAMETER_NOT_USED(handle);\
																							FSP_PARAMETER_NOT_USED(attr);\
																							FSP_PARAMETER_NOT_USED(value);\
																							return SM_NOT_SUPPORTED;}
#include "sm_define_sensors.inc"
#define DEFINE_SENSOR_DRIVER(DRIVER) sm_result BSP_WEAK_REFERENCE DRIVER##_get_attr(	sm_handle handle,\
																						sm_sensor_attributes attr,\
																						uint32_t * value);
#include "sm_define_sensors.inc"
#define DEFINE_SENSOR_DRIVER(DRIVER) sm_result BSP_WEAK_REFERENCE DRIVER##_get_attr(	sm_handle handle,\
																						sm_sensor_attributes attr,\
																						uint32_t * value) {\
																							FSP_PARAMETER_NOT_USED(handle);\
																							FSP_PARAMETER_NOT_USED(attr);\
																							FSP_PARAMETER_NOT_USED(value);\
																							return SM_NOT_SUPPORTED;}
#include "sm_define_sensors.inc"

typedef struct {
  void (*open)(sm_handle * handle, uint8_t address, uint8_t channel);
  void (*close)(sm_handle handle);
  sm_sensor_status (*read)(sm_handle handle, int32_t * data);
  void (*fsm)(void);
  uint8_t *(*get_flag)(sm_handle handle);
  void (*reset)(void);
  sm_result (*set_attr)(sm_handle handle, sm_sensor_attributes attr, uint32_t value);
  sm_result (*get_attr)(sm_handle handle, sm_sensor_attributes attr, uint32_t * value);
} sm_interface;

// Create individual structures for each sensor
#define DEFINE_SENSOR_DRIVER(DRIVER) sm_interface DRIVER##_api={\
		.open=&DRIVER##_open,.close=&DRIVER##_close,\
		.read=&DRIVER##_read,.fsm=&DRIVER##_fsm,\
		.get_flag=&DRIVER##_get_flag, .reset=&DRIVER##_reset,\
		.set_attr=&DRIVER##_set_attr, .get_attr=&DRIVER##_get_attr};
#include "sm_define_sensors.inc"

// Create an array with pointers to all sensor structures
static const sm_interface *driver[]={
    #define DEFINE_SENSOR_DRIVER(DRIVER) &DRIVER##_api,
    #include "sm_define_sensors.inc"
};

#define NUM_DRIVERS (sizeof(driver)/sizeof(driver[0]))

typedef enum {
    SM_CLOSE,
    SM_OPEN,
    SM_SW_TRIGGER,
    SM_TRIGGERED,
    SM_SAMPLING,
    SM_WAITING
} sensor_state;

typedef struct {
    sensor_state state;
    sm_handle handle;
    sm_sensor_status status;
    uint32_t last_sample_time;
    uint32_t interval;
    int32_t data;
    sm_callback callback;
    uint8_t * flag;
} instance_property;

typedef struct {
    sm_type type;
    sm_driver driver;
    uint8_t channel;
    uint8_t address;
    int32_t multipler;
    int32_t divider;
    int32_t offset;
} instance_const_property;

static uint16_t handle_indexer;

static instance_property sensor_properties[NUM_SENSORS] = {
    #define DEFINE_SENSOR_INSTANCE(TYPE, ADDR, CHAN, DRV, MULT, DIV, OFFS, INTERVAL_MS) {.state=SM_OPEN, .interval=INTERVAL_MS, .handle.value=0, .status = SM_SENSOR_ERROR, .callback=NULL, .flag=NULL},
    #include "sm_define_sensors.inc"
};

static const instance_const_property sensor_const_properties[NUM_SENSORS] = {
    #define DEFINE_SENSOR_INSTANCE(TYPE, ADDR, CHAN, DRV, MULT, DIV, OFFS, INTERVAL_MS) {.type=TYPE, .driver=DRIVER_##DRV, .address=ADDR, .channel=CHAN, .multipler=MULT, .divider=DIV, .offset = OFFS},
    #include "sm_define_sensors.inc"
};

static const char *driver_path[] = {
    #define TOSTR(arg) #arg
    #define DEFINE_SENSOR_TYPE(TYPE, UNIT, PATH) TOSTR(PATH),
    #include "sm_define_sensors.inc"
    #undef TOSTR
};

static const char *driver_unit[] = {
    #define TOSTR(arg) #arg
    #define DEFINE_SENSOR_TYPE(TYPE, UNIT, PATH) TOSTR(UNIT),
    #include "sm_define_sensors.inc"
    #undef TOSTR
};

static const char *driver_id[] = {
    #define TOSTR(arg) #arg
    #define DEFINE_SENSOR_DRIVER(NAME) TOSTR(NAME),
    #include "sm_define_sensors.inc"
    #undef TOSTR
};

void sm_init(void) {
    log_info("Init SM");
#if (BSP_CFG_RTOS) == 0
    // Only baremetal version needs to initialize system time
    fsp_err_t status = FSP_SUCCESS;
    status = utils_systime_init(1000);
    if (FSP_SUCCESS != status) {
        // Error initializing milliseconds timer
        log_error("Systime failure");
        log_debug("Error %d", status);
        APP_TRAP();
    }
#elif (BSP_CFG_RTOS) == 1
    // On AzureRTOS we need to initialize the message queue
    UINT result;
    result = tx_queue_create(&g_sensor_queue, "Sensor", sizeof(sm_sensor_data), &sensor_queue_storage[0], sizeof(sensor_queue_storage));
    if (TX_SUCCESS != result) {
        // Error creating the queue
        log_error("Queue creation failed");
        log_debug("Error %d", result);
        APP_TRAP();
    }    
#elif (BSP_CFG_RTOS) == 2
    // On FreeRTOS we need to initialize the message queue
    g_sensor_queue = xQueueCreateStatic(SENSOR_QUEUE_LENGTH, sizeof(sm_sensor_data), &sensor_queue_storage[0], &sensor_queue_memory);
    if (NULL == g_sensor_queue) {
        // Error creating the queue
        log_error("Queue creation failed");
        APP_TRAP();        
    }
#endif
    // First reset all sensors
    for (int i = 0; NUM_SENSORS > i; i++) {
        sm_interface *this_driver = (sm_interface *)driver[sensor_const_properties[i].driver-1];
        this_driver->reset();
    }
    // Now open each sensor
    for (int i = 0; NUM_SENSORS > i; i++) {
        sm_interface *this_driver = (sm_interface *)driver[sensor_const_properties[i].driver-1];
        sensor_properties[i].state = SM_OPEN;
        sensor_properties[i].handle.value = 0;
        this_driver->open(&sensor_properties[i].handle, sensor_const_properties[i].address, sensor_const_properties[i].channel);
        sensor_properties[i].handle.internal = sensor_const_properties[i].driver;
        // Now get the pointer to the driver's flag
        sensor_properties[i].flag = this_driver->get_flag(sensor_properties[i].handle);
    }
    log_info("Working with %d sensors",NUM_SENSORS);
    handle_indexer = 0;
}

void sm_run(void) {
    uint32_t minimum_interval = UINT32_MAX;
    uint32_t num_waiting = 0;
    uint32_t num_wait_trigger = 0;
    for (int i = 0; NUM_SENSORS > i; i++) {
        // Get the driver for this sensor
        sm_interface *this_driver = (sm_interface *)driver[sensor_const_properties[i].driver-1];
        if (0 != *sensor_properties[i].flag) {
            // Sensor is flagged, that means there is data to read
            sensor_properties[i].state = SM_SAMPLING;
        }
        switch (sensor_properties[i].state) {
            case SM_CLOSE:
                if (0 != sensor_properties[i].handle.value) {
                    this_driver->close(sensor_properties[i].handle);
                    sensor_properties[i].handle.value = 0;
                }
                break;
            case SM_OPEN:
                if (0 < sensor_properties[i].interval) {
                    sensor_properties[i].state = SM_TRIGGERED;
                } else sensor_properties[i].state = SM_SW_TRIGGER;
                break;
          case SM_SW_TRIGGER:
                // do nothing, wait for a trigger from the application or sensor
                num_wait_trigger++;
                break;
          case SM_TRIGGERED:
                if (0 != sensor_properties[i].handle.value) {
                    sensor_properties[i].state = SM_SAMPLING;
                } else {
                    sensor_properties[i].state = SM_CLOSE;
                }
                break;
          case SM_SAMPLING:
                *sensor_properties[i].flag = 0;
                sensor_properties[i].status = this_driver->read(sensor_properties[i].handle, &sensor_properties[i].data);
                if (SM_SENSOR_DATA_VALID == sensor_properties[i].status) {
                    log_debug("Sensor index %d received %d",i,sensor_properties[i].data);
                } else if (SM_SENSOR_INVALID_DATA == sensor_properties[i].status){
                    log_error("Sensor index %d received invalid data %d",i,sensor_properties[i].data);
                } else if (SM_SENSOR_STALE_DATA == sensor_properties[i].status) {
                    log_debug("Sensor index %d stale data %d",i,sensor_properties[i].data);
                } else {
                    log_error("Sensor index %d error",i);
                }
                if (SM_SENSOR_DATA_VALID == sensor_properties[i].status) {
#if (BSP_CFG_RTOS) == 0                
                    // On baremetal, if we have a callback registered for this sensor, it is time to call it!
                    if (NULL != sensor_properties[i].callback) {
                        sensor_properties[i].callback(sensor_properties[i].handle, (uint8_t *)&sensor_properties[i].data, sizeof(sensor_properties[i].data));
                    }
#elif (BSP_CFG_RTOS) == 1
                    // On AzureRTOS we send the data to the queue
                    sm_sensor_data data;
                    data.handle = sensor_properties[i].handle;
                    data.data = sensor_properties[i].data;
                    UINT result = tx_queue_send(&g_sensor_queue, (void *)&data, (ULONG) SENSOR_SEND_WAIT_TIME);
                    if(TX_SUCCESS != result) {
                        // Failed to send sensor data
                        log_error("Queue send fail");
                        log_debug("Error %d",result);
                        log_debug("Handle %d",data.handle.value);
                    }
#elif (BSP_CFG_RTOS) == 2
                    // On FreeRTOS we send the data to the queue
                    sm_sensor_data data;
                    data.handle = sensor_properties[i].handle;
                    data.data = sensor_properties[i].data;
                    BaseType_t result = xQueueSend(g_sensor_queue, (void *)&data, ( TickType_t ) SENSOR_SEND_WAIT_TIME);
                    if(pdPASS != result) {
                        // Failed to send sensor data
                        log_error("Queue send fail");
                        log_debug("Error %d",result);
                        log_debug("Handle %d",data.handle.value);                    
                    }
#endif
                }
                sensor_properties[i].last_sample_time = utils_systime_get();
                if (0 < sensor_properties[i].interval) {
                    sensor_properties[i].state = SM_WAITING;
                } else {
                    sensor_properties[i].state = SM_SW_TRIGGER;
                }
                break;
          case SM_WAITING:
                if (sensor_properties[i].interval < minimum_interval) minimum_interval = sensor_properties[i].interval;
                if (utils_systime_get() - sensor_properties[i].last_sample_time > sensor_properties[i].interval) {
                    sensor_properties[i].state = SM_SAMPLING;
                } else num_waiting++;
                break;
        }
        this_driver->fsm();
    }
    // We went through all sensors, now set sleep time to the minimum interval
    if (NUM_SENSORS == (num_waiting + num_wait_trigger)) {
#if (BSP_CFG_RTOS) == 0
    // Do nothing in baremetal
#elif (BSP_CFG_RTOS) == 1
        tx_thread_sleep(1);
#elif (BSP_CFG_RTOS) == 2
        vTaskDelay (1);
#endif
    }
}

static int16_t sm_get_sensor_index(sm_handle handle) {
    for (int16_t i = 0; i < NUM_SENSORS; i++) {
        if (handle.value == sensor_properties[i].handle.value) return i;
    }
    log_debug("Handle not found");
    return -1;
}

sm_sensor_status sm_read_sensor(sm_handle handle, int32_t * data) {
    int16_t sensor_index = sm_get_sensor_index(handle);
    sm_sensor_status result = SM_SENSOR_ERROR;
    if (0 <= sensor_index) {
        *data = sensor_properties[sensor_index].data;
        result =  sensor_properties[sensor_index].status;
        // if we read the sensor again before it is updated, we will sill state status
        sensor_properties[sensor_index].status = SM_SENSOR_STALE_DATA;
    }
    return result;
}

uint16_t sm_get_total_sensor_count(void) {
    return NUM_SENSORS;
}

int32_t sm_get_sensor_handle(sm_type type, sm_handle * handle) {
    while (handle_indexer < NUM_SENSORS) {
        if (SENSOR_ANY_TYPE == type  || sensor_const_properties[handle_indexer].type == type) {
            handle->value = sensor_properties[handle_indexer++].handle.value;
            return 0;
        } else handle_indexer++;
    }
    handle_indexer = 0;  // We have gone through all sensors, reset the indexer
    handle->value = 0;
    return -1;
}

sm_type sm_get_sensor_type_by_handle(sm_handle handle) {
    int16_t sensor_index = sm_get_sensor_index(handle);
    if (0 <= sensor_index) {
        return sensor_const_properties[sensor_index].type;
    } else {
        return SENSOR_ANY_TYPE;
    }     
}

const char * sm_get_sensor_id(sm_handle handle) {
    if (!IS_HANDLE_VALID(handle)) return NULL;
    int16_t sensor_index = sm_get_sensor_index(handle);
    if (0 <= sensor_index) {
        return driver_id[sensor_const_properties[sensor_index].driver-1];
    } else return NULL;
}

void sm_get_sensor_scaling(sm_handle handle, sm_scaling *scaling) {
    int16_t sensor_index = sm_get_sensor_index(handle);
    if (0 <= sensor_index) {
        scaling->multiplier = sensor_const_properties[sensor_index].multipler;
        scaling->divider = sensor_const_properties[sensor_index].divider;
        scaling->offset = sensor_const_properties[sensor_index].offset;
    } else {
        log_error("Invalid handle");
    }
}

void sm_register_callback_by_handle(sm_handle handle, sm_callback callback) {
    int16_t sensor_index = sm_get_sensor_index(handle);
    if (0 <= sensor_index) {
        log_info("Registered callback for %d\n", handle.value);
        sensor_properties[sensor_index].callback = callback;
    }
}

uint16_t sm_register_callback_by_type(sm_type type, sm_callback callback) {
    sm_handle handle;
    uint16_t num_registrations = 0;
    while (1) {
        if (0 != sm_get_sensor_handle(type, &handle)) break;
        sm_register_callback_by_handle(handle, callback);
        num_registrations++;
    }
    return num_registrations;
}

uint16_t sm_register_callback_any_type(sm_callback callback) {
    sm_handle handle;
    uint16_t num_registrations = 0;
    while (1) {
        if (0 != sm_get_sensor_handle(SENSOR_ANY_TYPE, &handle)) break;
        sm_register_callback_by_handle(handle, callback);
        num_registrations++;
    }
    return num_registrations;
}

const char * sm_get_sensor_unit_by_type(sm_type type) {
    if (SENSOR_ANY_TYPE > type) {
        return driver_unit[type];
    } else return NULL;
}

const char * sm_get_sensor_unit_by_handle(sm_handle handle) {
    int16_t sensor_index = sm_get_sensor_index(handle);
    if (0 <= sensor_index) {
        return driver_unit[sensor_const_properties[sensor_index].type];
    } else return NULL;
}

const char * sm_get_sensor_path_by_type(sm_type type) {
    if (SENSOR_ANY_TYPE > type) {
        return driver_path[type];
    } else return NULL;
}

const char * sm_get_sensor_path_by_handle(sm_handle handle) {
    int16_t sensor_index = sm_get_sensor_index(handle);
    if (0 <= sensor_index) {
        return driver_path[sensor_const_properties[sensor_index].type];
    } else return NULL;
}

sm_result sm_set_sensor_attribute(sm_handle handle, sm_sensor_attributes attr, uint32_t value) {
    int16_t sensor_index = sm_get_sensor_index(handle);
    sm_result result = SM_ERROR;
    if (0 <= sensor_index) {
        sm_interface *this_driver = (sm_interface *)driver[sensor_const_properties[sensor_index].driver-1];
        result = this_driver->set_attr(handle, attr, value);
    	if (SM_ACQUISITION_INTERVAL == attr) {
            // Update sensor manager publishing interval
            sensor_properties[sensor_index].interval = value;
            result = SM_OK;
        }        
    }
    return result;
}

sm_result sm_get_sensor_attribute(sm_handle handle, sm_sensor_attributes attr, uint32_t * value) {
    int16_t sensor_index = sm_get_sensor_index(handle);
    sm_result result = SM_ERROR;
    if (0 <= sensor_index) {
        sm_interface *this_driver = (sm_interface *)driver[sensor_const_properties[sensor_index].driver-1];
        result = this_driver->get_attr(handle, attr, value);
    	if (SM_ACQUISITION_INTERVAL == attr) {
            // Update sensor manager publishing interval
            *value = sensor_properties[sensor_index].interval;
            result = SM_OK;
        }        
    }
    return result;
}
