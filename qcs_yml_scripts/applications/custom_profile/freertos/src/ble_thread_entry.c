#include "ble_thread.h"
/* BLE Thread entry function */
/* pvParameters contains TaskHandle_t */
extern void app_main(void);

void ble_thread_entry(void *pvParameters)
{
    FSP_PARAMETER_NOT_USED (pvParameters);

    /* TODO: add your own code here */
    app_main();

    while (1)
    {
        vTaskDelay (1);
    }
}
