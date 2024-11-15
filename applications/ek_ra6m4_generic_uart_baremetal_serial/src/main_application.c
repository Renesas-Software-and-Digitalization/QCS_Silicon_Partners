#include <stdio.h>
#include "common_utils.h"
#include "sm.h"
#include "uart.h"
#include "main_application.h"
// Uncomment the desired debug level
#include "log_disabled.h"
//#include "log_error.h"
//#include "log_warning.h"
//#include "log_info.h"
//#include "log_debug.h"

void publish_sensor(sm_handle handle, uint8_t * buffer, uint16_t size);

void publish_sensor(sm_handle handle, uint8_t * buffer, uint16_t size) {
    if (sizeof(int32_t) == size) {
        sm_scaling scaling = {.divider=1, .multiplier=1,.offset=0};
        sm_get_sensor_scaling(handle, &scaling);
        int32_t data = *(int32_t *)buffer;
        data += scaling.offset;
        data = (data * scaling.multiplier * 100) / scaling.divider;
        printf("Publishing: /%s/%s: ", sm_get_sensor_path_by_handle(handle),sm_get_sensor_id(handle));
        // First print the integer part followed by the decimal separator .
        utils_print_fractional(data, TWO_DECIMALS);
        // print the unit
        printf("%s\r\n",sm_get_sensor_unit_by_handle(handle));
    }
}

fsp_err_t main_application(void) {
    fsp_err_t status = FSP_SUCCESS;
    static uint8_t state;

    while (true) {
        switch (state) {
            case 0:
                status = uart_initialize();
                if (FSP_SUCCESS != status) {
                    log_error("UART init failed");
                    APP_TRAP();
                }
                sm_init();
                printf("Application - prints out sensor data\r\n");
                printf("We have %d sensors\r\n", sm_get_total_sensor_count());
                if (sm_register_callback_any_type(publish_sensor) == 0) {
                    // Error, no sensor registered
                    log_error("No sensor found");
                    uart_deinitialize();
                    return FSP_ERR_NOT_FOUND;
                }
                state = 1;
                break;
            case 1:
               sm_run();
               break;
        }
    }
    return status;
}

