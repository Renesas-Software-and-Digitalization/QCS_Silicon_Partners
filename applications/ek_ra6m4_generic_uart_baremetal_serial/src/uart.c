/***********************************************************************************************************************
 * File Name    : uart.c
 * Description  : Contains UART functions definition.
 **********************************************************************************************************************/
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

#include <stdio.h>
#include "common_utils.h"
#include "uart.h"
#if (BSP_CFG_RTOS) != 0
#include "app_thread.h"
#endif
// Uncomment the desired debug level
#include "log_disabled.h"
//#include "log_error.h"
//#include "log_warning.h"
//#include "log_info.h"
//#include "log_debug.h"

/*******************************************************************************************************************//**
 * @addtogroup r_sci_uart_ep
 * @{
 **********************************************************************************************************************/

/*
 * Private function declarations
 */
unsigned int _write(int fd, char * p_src, unsigned len);
unsigned int _close(int fd);
int _read(int const fd, void * const buffer, unsigned const buffer_size);
long _lseek(int fd, long offset, int origin);
int _fstat(int fd, void *buffer);
int _isatty( int fd );

/*
 * Private global variables
 */
/* Temporary buffer to save data from receive buffer for further processing */
static uint8_t g_temp_buffer[DATA_LENGTH] = {RESET_VALUE};

/* Counter to update g_temp_buffer index */
static volatile uint8_t g_counter_var = RESET_VALUE;

/* Flag to check whether data is received or not */
static volatile uint8_t g_data_received_flag = false;

/* Flag for user callback */
static volatile uint8_t g_uart_event = RESET_VALUE;


fsp_err_t uart_initialize(void)
{
    fsp_err_t err = FSP_SUCCESS;

    /* Initialize UART channel with baud rate 115200 */
    err = g_uart0.p_api->open(&g_uart0_ctrl, &g_uart0_cfg);
    if (FSP_SUCCESS != err)
    {
        log_error("UART open failed");
    }
    return err;
}

void uart_deinitialize(void)
{
    fsp_err_t status = FSP_SUCCESS;

    /* Close module */
    status =  g_uart0.p_api->close(&g_uart0_ctrl);
    if (FSP_SUCCESS != status)
    {
        log_error ("UART close failed");
    }
}

/*******************************************************************************************************************//**
 * @brief       Redirect printf to UART/RTT according to file handle
 * @param[in]   file - the file descriptor (1 for stdout or 2 for stderr)
 * @param[in]   p_src - a char pointer pointing to the string buffer to write
 * @param[in]   len - the length of the string
 * @retval      number of characters written
 ***********************************************************************************************************************/
unsigned int _write(int fd, char * p_src, unsigned len)
{
    fsp_err_t status;

    if (1 == fd) {
        uint32_t local_timeout = (DATA_LENGTH * UINT16_MAX);

        status = g_uart0.p_api->write(&g_uart0_ctrl, (uint8_t *) p_src, len);

        /* Reset callback capture variable */
        g_uart_event = RESET_VALUE;

        /* Check for event transfer complete */
        while ((UART_EVENT_TX_COMPLETE != g_uart_event) && (--local_timeout))
        {
            /* Check if any error event occurred */
            if (UART_ERROR_EVENTS == g_uart_event)
            {
                log_warning("UART Error Event Received");
                log_debug("Events %d", g_uart_event);
                status = FSP_ERR_WRITE_FAILED;
            }
        }
        if(RESET_VALUE == local_timeout)
        {
            status = FSP_ERR_TIMEOUT;
        }
        if (FSP_SUCCESS == status) {
            return len;
        } else {
            return 0;
        }
    } else {
        // We always assume that if it is not stdout, it is stderr
        //return SEGGER_RTT_Write (0, p_src, len);
        return 0;
    }
}

unsigned int _close(int fd) {
    FSP_PARAMETER_NOT_USED(fd);
    return 0;
}

int _read(int const fd, void * const buffer, unsigned const buffer_size) {
    FSP_PARAMETER_NOT_USED(fd);
    FSP_PARAMETER_NOT_USED(buffer);
    FSP_PARAMETER_NOT_USED(buffer_size);
    return 0;
}

long _lseek(int fd, long offset, int origin) {
    FSP_PARAMETER_NOT_USED(fd);
    FSP_PARAMETER_NOT_USED(offset);
    FSP_PARAMETER_NOT_USED(origin);
    return 0;
}

int _fstat(int fd, void *buffer) {
    FSP_PARAMETER_NOT_USED(fd);
    FSP_PARAMETER_NOT_USED(buffer);
    return 0;
}

int _isatty( int fd ) {
    if (1 == fd) return 1; else return 0;
}

/*****************************************************************************************************************
 *  @brief      UART user callback
 *  @param[in]  p_args
 *  @retval     None
 ****************************************************************************************************************/
void user_uart_callback(uart_callback_args_t *p_args)
{
    /* Logged the event in global variable */
    g_uart_event = (uint8_t)p_args->event;

    /* Reset g_temp_buffer index if it exceeds than buffer size */
    if(DATA_LENGTH == g_counter_var)
    {
        g_counter_var = RESET_VALUE;
    }

    if(UART_EVENT_RX_CHAR == p_args->event)
    {
        switch (p_args->data)
        {
            /* If Enter is pressed by user, set flag to process the data */
            case CARRIAGE_ASCII:
            {
                g_counter_var = RESET_VALUE;
                g_data_received_flag  = true;
                break;
            }
            /* Read all data provided by user until enter key is pressed */
            default:
            {
                g_temp_buffer[g_counter_var++] = (uint8_t ) p_args->data;
                break;
            }
        }
    }
}


/*******************************************************************************************************************//**
 * @} (end addtogroup r_sci_uart_ep)
 **********************************************************************************************************************/
