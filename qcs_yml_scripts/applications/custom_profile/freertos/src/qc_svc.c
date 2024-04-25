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
* Copyright (C) 2019 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "qc_svc.h"

#define QC_SVC_REQUEST_LEN_MIN        3
#define QC_SVC_REQUEST_TYPE_WRITE     0x01
#define QC_SVC_REQUEST_TYPE_READ      0x02
#define QC_SVC_RESPONSE_FRAG_LEN_MAX  256  // TODO should this be tied to the MTU?

typedef struct
{
  uint32_t   payload_len;
  uint32_t   offset;
  uint16_t   remaining;
  uint8_t  * payload_data;
  bool       in_progress;
  uint16_t   id;
  uint8_t    type;
  uint8_t    status;
} qc_svc_rsp_msg_t;

static qc_svc_error_t qc_svc_send_rsp_msg_fragment(qc_svc_rsp_msg_t * rsp);

static qc_svc_request_handlers_t const * p_req_handlers = NULL;
static qc_svc_response_transmit_cb_t     transmit_cb;
static qc_svc_rsp_msg_t                  qc_svc_rsp_msg;
static uint16_t                          transport_mtu  = 0;

qc_svc_error_t qc_svc_send_read_response(qc_svc_error_t status, uint16_t id, uint16_t len, uint8_t * data)
{
    qc_svc_error_t err;

    qc_svc_rsp_msg.type         = QC_SVC_REQUEST_TYPE_READ;
    qc_svc_rsp_msg.status       = status;
    qc_svc_rsp_msg.id           = id;
    qc_svc_rsp_msg.payload_len  = len;
    qc_svc_rsp_msg.payload_data = data;
    qc_svc_rsp_msg.remaining    = len;
    qc_svc_rsp_msg.offset       = 0;

    err = qc_svc_send_rsp_msg_fragment(&qc_svc_rsp_msg);

    return err;
}

qc_svc_error_t qc_svc_handle_response_write_cfm(void)
{
    if (qc_svc_rsp_msg.in_progress == true)
    {
        if (qc_svc_rsp_msg.remaining > 0)
        {
            qc_svc_send_rsp_msg_fragment(&qc_svc_rsp_msg);
        }
        else
        {
            qc_svc_rsp_msg.in_progress = false;
        }
    }

    return QC_SVC_SUCCESS;
}

qc_svc_error_t qc_svc_handle_request(uint8_t const * const p_request, uint32_t len)
{
    uint8_t                  i;
    uint8_t                  type;
    uint16_t                 id;
    qc_svc_error_t           err         = QC_SVC_ERR_INVALID_REQEST;
    qc_svc_request_handler_t req_handler = NULL;

    type = p_request[0];
    id  =  (uint16_t)p_request[1];
    id |=  (uint16_t)((uint16_t)p_request[2] << 8);

    if (QC_SVC_REQUEST_LEN_MIN <= len)
    {
        switch (type)
        {
            case QC_SVC_REQUEST_TYPE_WRITE:
            case QC_SVC_REQUEST_TYPE_READ:
            {
                if (NULL != p_req_handlers)
                {
                    for (i = 0; p_req_handlers[i].id != 0xFFFF; i++)
                    {
                        if (p_req_handlers[i].id == id)
                        {
                            switch (type)
                            {
                                case QC_SVC_REQUEST_TYPE_READ:
                                    req_handler = p_req_handlers[i].rd_handler;
                                    break;
                                case QC_SVC_REQUEST_TYPE_WRITE:
                                    req_handler = p_req_handlers[i].wr_handler;
                                    break;
                                default:
                                    break;
                            }
                            if (NULL != req_handler)
                            {
                                req_handler(id, &p_request[3]);
                            }
                            else
                            {
                                err = QC_SVC_ERR_UNKNOWN_ID;
                            }
                            break;
                        }
                    }
                }
                else
                {
                    err = QC_SVC_ERR_HANLDERS_NOT_REGISTERED;
                }
            }
            break;

            default:
            {
                /* Unknown request type */
                err = QC_SVC_ERR_UNKNOWN_REQUEST_TYPE;
            }
            break;
        }
    }

    return err;
}

qc_svc_error_t qc_svc_register_handlers(qc_svc_request_handlers_t const * const p_handlers)
{
    qc_svc_error_t err = QC_SVC_ERR_ALREADY_REGISTERED;

    if (NULL == p_req_handlers)
    {
        p_req_handlers = p_handlers;
        err = QC_SVC_SUCCESS;
    }

    return err;
}

qc_svc_error_t qc_svc_register_transmit_cb(qc_svc_response_transmit_cb_t cb)
{
    qc_svc_error_t err = QC_SVC_ERR_ALREADY_REGISTERED;

    if (NULL == transmit_cb)
    {
        transmit_cb = cb;
        err = QC_SVC_SUCCESS;
    }

    return err;
}

qc_svc_error_t qc_svc_set_transport_mtu(uint16_t mtu)
{
    transport_mtu = mtu;

    return QC_SVC_SUCCESS;
}

static qc_svc_error_t qc_svc_send_rsp_msg_fragment(qc_svc_rsp_msg_t * p_rsp)
{
    /* Determine size of header */
    uint16_t  hdr_len = 0;
    uint16_t frag_len = 0;
    uint16_t ix       = 0;
    uint8_t  buffer[QC_SVC_RESPONSE_FRAG_LEN_MAX];

    if (p_rsp->payload_len > 0)
    {
        hdr_len = sizeof(p_rsp->type) + sizeof(p_rsp->status) +
                  sizeof(p_rsp->id)   + sizeof(p_rsp->payload_len);
    }
    else
    {
        hdr_len = sizeof(p_rsp->type) + sizeof(p_rsp->status) +
                  sizeof(p_rsp->id);
    }

    /* Determine size of fragment to be transmitted */
    if ((hdr_len + p_rsp->remaining) <= transport_mtu)
    {
        frag_len = hdr_len + p_rsp->remaining;
    }
    else
    {
        frag_len = transport_mtu;
    }

    /* Add packet fragment header */
    buffer[ix++] = p_rsp->type;
    buffer[ix++] = p_rsp->status;
    memcpy(&buffer[ix], (uint8_t *)&p_rsp->id, sizeof(p_rsp->id));
    ix += sizeof(p_rsp->id);

    memcpy(&buffer[ix], (uint8_t *)&p_rsp->payload_len, sizeof(p_rsp->payload_len));
    ix += sizeof(p_rsp->payload_len);

    /* Add payload data, if present */
    if (p_rsp->payload_len > 0)
    {
        if (p_rsp->payload_data != NULL)
        {
            memcpy(&buffer[ix], (uint8_t *)&p_rsp->payload_data[p_rsp->offset], (frag_len - hdr_len));
        }
        p_rsp->offset += (frag_len - hdr_len);
        ix += ((uint16_t)(frag_len - hdr_len));
    }
    p_rsp->in_progress = true;
    p_rsp->remaining  -= ((uint16_t)(frag_len - hdr_len));

    if (transmit_cb != 0)
    {
        transmit_cb(buffer, ix);
    }

    return QC_SVC_SUCCESS;
}
