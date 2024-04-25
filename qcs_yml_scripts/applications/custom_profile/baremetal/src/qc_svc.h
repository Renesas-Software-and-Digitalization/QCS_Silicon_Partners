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
* Copyright (C) 2019-2020 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/

#ifndef QC_SVC_H
#define QC_SVC_H

#include <stdint.h>

typedef enum e_qc_svc_err
{
    QC_SVC_SUCCESS                     = 0,
    QC_SVC_ERR_INVALID_REQEST          = 1,
    QC_SVC_ERR_UNKNOWN_REQUEST_TYPE    = 2,
    QC_SVC_ERR_ALREADY_REGISTERED      = 3,
    QC_SVC_ERR_HANLDERS_NOT_REGISTERED = 4,
    QC_SVC_ERR_UNKNOWN_ID              = 5,
} qc_svc_error_t;

typedef void (* qc_svc_request_handler_t)(uint16_t id, uint8_t const * const data);
typedef void (* qc_svc_response_transmit_cb_t)(uint8_t const * const data, uint16_t len);

typedef struct
{
    uint16_t                 id;
    qc_svc_request_handler_t rd_handler;
    qc_svc_request_handler_t wr_handler;
} qc_svc_request_handlers_t;

qc_svc_error_t qc_svc_set_transport_mtu(uint16_t mtu);
qc_svc_error_t qc_svc_register_handlers(qc_svc_request_handlers_t const * const p_handlers);
qc_svc_error_t qc_svc_register_transmit_cb(qc_svc_response_transmit_cb_t cb);
qc_svc_error_t qc_svc_handle_request(uint8_t const * const p_request, uint32_t len);
qc_svc_error_t qc_svc_send_read_response(qc_svc_error_t status, uint16_t id, uint16_t len, uint8_t * data);
qc_svc_error_t qc_svc_handle_response_write_cfm(void);

#endif
