/*
* Copyright (c) 2020 - 2024 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier:  BSD-3-Clause
*/
#include <stdio.h>
#include "common_utils.h"
#include "RTOS.h"
// Uncomment the desired debug level
#include "log_disabled.h"
//#include "log_error.h"
//#include "log_warning.h"
//#include "log_info.h"
//#include "log_debug.h"

#if (BSP_CFG_RTOS) == 0
void SysTick_Handler(void);
static uint32_t systick_counter;
void SysTick_Handler(void) {
    systick_counter++;
}
#endif

void utils_print_fractional(int32_t data, edecimals dec) {
	int32_t decimals = dec;
    int32_t fractional = data % decimals;
    if (0 > data) printf("-");	// negative value
    // First print the integer part followed by the decimal separator .
    printf("%ld.", data/decimals);
    // Print leading zeros
    decimals /= 10;
    while ((fractional < decimals) && (10 <= decimals)) {
        printf("0");
        decimals /= 10;
    }
    printf("%ld", fractional);
}

fsp_err_t utils_systime_init(uint32_t freq) {
    fsp_err_t status = FSP_SUCCESS;
#if (BSP_CFG_RTOS) == 0
    if (SysTick->CTRL & SysTick_CTRL_ENABLE_Msk) {
        // Systick already enabled, return error
        status = FSP_ERR_ALREADY_OPEN;
        log_warning("SysTick already init");
    } else {
        // Configure systick reload
        SysTick_Config(SystemCoreClock/(freq));
        systick_counter = 0;
    }
#else
    FSP_PARAMETER_NOT_USED(freq);
#endif
    return status;
}

uint32_t utils_systime_get(void) {
#if (BSP_CFG_RTOS) == 0
    return systick_counter;
#elif (BSP_CFG_RTOS) == 1
    return tx_time_get();
#elif (BSP_CFG_RTOS) == 2
    return xTaskGetTickCount();
#endif
}

void utils_set_LED(bsp_io_port_pin_t pin, uint8_t state) {
    /* Enable access to the PFS registers. If using r_ioport module then register protection is automatically
     * handled. This code uses BSP IO functions to show how it is used.
     */
    R_BSP_PinAccessEnable();
    if (state == ON) {
        /* Write to this pin */
        R_BSP_PinWrite((bsp_io_port_pin_t) pin, BSP_IO_LEVEL_HIGH);
    } else {
        /* Write to this pin */
        R_BSP_PinWrite((bsp_io_port_pin_t) pin, BSP_IO_LEVEL_LOW);
    }
    /* Protect PFS registers */
    R_BSP_PinAccessDisable();
}

void utils_halt_func(void) {
    utils_set_LED(RED_LED, ON);

    log_error("Fatal Error! System Reseting in 1 second");
    utils_delay_ms(1000);

    NVIC_SystemReset();
}

void utils_delay_us(uint32_t delay) {
    R_BSP_SoftwareDelay(delay, BSP_DELAY_UNITS_MICROSECONDS);
}

void utils_delay_ms(uint32_t delay) {
#if (BSP_CFG_RTOS) == 0
    R_BSP_SoftwareDelay(delay, BSP_DELAY_UNITS_MILLISECONDS);
#elif (BSP_CFG_RTOS) == 1
    tx_thread_sleep(delay);
#elif (BSP_CFG_RTOS) == 2
    vTaskDelay(delay);
#else
    FSP_PARAMETER_NOT_USED(delay);
#endif
}