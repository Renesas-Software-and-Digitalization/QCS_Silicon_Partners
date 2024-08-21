/***********************************************************************************************************************
 * File Name    : common_utils.h
 * Description  : Contains macros, data structures and functions used  common to the EP
 ***********************************************************************************************************************/
/***********************************************************************************************************************
 * DISCLAIMER
 * This software is supplied by Renesas Electronics Corporation and is only intended for use with Renesas products. No
 * other uses are authorized. This software is owned by Renesas Electronics Corporation and is protected under all
 * applicable laws, including copyright laws.
 * THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
 * THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED. TO THE MAXIMUM
 * EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES
 * SHALL BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO THIS
 * SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 * Renesas reserves the right, without notice, to make changes to this software and to discontinue the availability of
 * this software. By using this software, you agree to the additional terms and conditions found by accessing the
 * following link:
 * http://www.renesas.com/disclaimer
 *
 * Copyright (C) 2020 Renesas Electronics Corporation. All rights reserved.
 ***********************************************************************************************************************/

#ifndef __COMMON_UTILS_H_
#define __COMMON_UTILS_H_

/* generic headers */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "hal_data.h"
/* SEGGER RTT and error related headers */
#include "SEGGER_RTT.h"

#define ON          (1)
#define OFF         (0)

#define RED_LED  BSP_IO_PORT_04_PIN_00 // LED3 on the board

#define GREEN_LED  BSP_IO_PORT_04_PIN_04 // LED2 on the board

#define BLUE_LED  BSP_IO_PORT_04_PIN_15 // LED1 on the board

#define SW1_SWITCH  BSP_IO_PORT_00_PIN_05 // Switch S1 on the board

#define SW2_SWITCH  BSP_IO_PORT_00_PIN_06 // Switch S2 on the board


#define BIT_SHIFT_8             (8u)
#define SIZE_64                 (64u)
#define RESET_VALUE             (0x00)
#define EP_VERSION              ("1.0")
#define SEGGER_INDEX            (0)

#define APP_READ(read_data)     SEGGER_RTT_Read (SEGGER_INDEX, read_data, sizeof(read_data));
#define APP_CHECK_DATA          SEGGER_RTT_HasKey()
#define APP_TRAP()              __asm("BKPT #0\n"); /* trap upon the error  */

typedef enum {
	ONE_DECIMAL 	= 10,
	TWO_DECIMALS 	= 100,
	THREE_DECIMALS 	= 1000
} edecimals;

/************************************************************************
  Utility functions
************************************************************************/

/*******************************************************************************************************************//**
 * @brief       Print (stdout) a fractional number with one, two or three decimals
 * @param[in]   data - signed 32-bit value to print
 * @param[in]	decimals - ONE_DECIMAL, TWO_DECIMALS or THREE_DECIMALS
 * @retval      FSP_SUCCESS or an error
 ***********************************************************************************************************************/
void utils_print_fractional(int32_t data, edecimals dec);
/*******************************************************************************************************************//**
 * @brief       Initialize systick timing using the supplied frequency
 * @param[in]   freq - define the time unit, use 1 for us, 1000 for ms and so on
 * @retval      FSP_SUCCESS or an error
 ***********************************************************************************************************************/
fsp_err_t utils_systime_init(uint32_t freq);
/*******************************************************************************************************************//**
 * @brief       Get the current time
 * @param[in]   none
 * @retval      time count in the time unit specified by utils_systime_init
 ***********************************************************************************************************************/
uint32_t utils_systime_get(void);
/*******************************************************************************************************************//**
 * @brief       Set LED to the specific state
 * @param[in]   pin - the pin where the LED is connected
 * @param[in]   state - the desired state (ON/OFF)
 * @retval      none
 ***********************************************************************************************************************/
void utils_set_LED(bsp_io_port_pin_t pin, uint8_t state);
/*******************************************************************************************************************//**
 * @brief       Halt the system
 * @param[in]   none
 * @retval      none
 ***********************************************************************************************************************/
void utils_halt_func(void);

#endif /* __COMMON_UTILS_H_ */
