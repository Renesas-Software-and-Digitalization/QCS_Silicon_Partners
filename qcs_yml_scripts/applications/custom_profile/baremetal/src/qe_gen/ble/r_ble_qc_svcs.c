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
/***********************************************************************************************************************
 * File Name: r_ble_qc_svcs.c
 * Version : 1.0
 * Description : The source file for Quick Connect Service service.
 **********************************************************************************************************************/

#include "r_ble_qc_svcs.h"
#include "profile_cmn/r_ble_servs_if.h"
#include "gatt_db.h"

static st_ble_servs_info_t gs_servs_info;

/*----------------------------------------------------------------------------------------------------------------------
    Request Characteristic User Description descriptor : Characteristic User Description
----------------------------------------------------------------------------------------------------------------------*/

static ble_status_t decode_st_ble_qc_svcs_qc_req_char_user_desc_t(st_ble_qc_svcs_qc_req_char_user_desc_t *p_app_value, const st_ble_gatt_value_t *p_gatt_value)
{
    /* Start user code for Request Characteristic User Description descriptor value decode function. Do not edit comment generated here */
    FSP_PARAMETER_NOT_USED(p_app_value);
    FSP_PARAMETER_NOT_USED(p_gatt_value);
    /* End user code. Do not edit comment generated here */
    return BLE_SUCCESS;
}

static ble_status_t encode_st_ble_qc_svcs_qc_req_char_user_desc_t(const st_ble_qc_svcs_qc_req_char_user_desc_t *p_app_value, st_ble_gatt_value_t *p_gatt_value)
{
    /* Start user code for Request Characteristic User Description descriptor value encode function. Do not edit comment generated here */
    FSP_PARAMETER_NOT_USED(p_app_value);
    FSP_PARAMETER_NOT_USED(p_gatt_value);
    /* End user code. Do not edit comment generated here */
    return BLE_SUCCESS;
}

static const st_ble_servs_desc_info_t gs_qc_req_char_user_desc = {
    .attr_hdl = BLE_QC_SVCS_QC_REQ_CHAR_USER_DESC_DESC_HDL,
    .app_size = sizeof(st_ble_qc_svcs_qc_req_char_user_desc_t),
    .desc_idx = BLE_QC_SVCS_QC_REQ_CHAR_USER_DESC_IDX,
    .db_size  = BLE_QC_SVCS_QC_REQ_CHAR_USER_DESC_LEN,
    .decode   = (ble_servs_attr_decode_t)decode_st_ble_qc_svcs_qc_req_char_user_desc_t,
    .encode   = (ble_servs_attr_encode_t)encode_st_ble_qc_svcs_qc_req_char_user_desc_t,
};

ble_status_t R_BLE_QC_SVCS_SetQc_reqCharUserDesc(const st_ble_qc_svcs_qc_req_char_user_desc_t *p_value)
{
    return R_BLE_SERVS_SetDesc(&gs_qc_req_char_user_desc, BLE_GAP_INVALID_CONN_HDL, (const void *)p_value);
}

ble_status_t R_BLE_QC_SVCS_GetQc_reqCharUserDesc(st_ble_qc_svcs_qc_req_char_user_desc_t *p_value)
{
    return R_BLE_SERVS_GetDesc(&gs_qc_req_char_user_desc, BLE_GAP_INVALID_CONN_HDL, (void *)p_value);
}

/*----------------------------------------------------------------------------------------------------------------------
    Request characteristic
----------------------------------------------------------------------------------------------------------------------*/

static ble_status_t decode_st_ble_qc_svcs_qc_req_t(st_ble_qc_svcs_qc_req_t *p_app_value, const st_ble_gatt_value_t *p_gatt_value)
{
    /* Start user code for Request characteristic value decode function. Do not edit comment generated here */
    FSP_PARAMETER_NOT_USED(p_app_value);
    FSP_PARAMETER_NOT_USED(p_gatt_value);
    /* End user code. Do not edit comment generated here */
    return BLE_SUCCESS;
}

static ble_status_t encode_st_ble_qc_svcs_qc_req_t(const st_ble_qc_svcs_qc_req_t *p_app_value, st_ble_gatt_value_t *p_gatt_value)
{
    /* Start user code for Request characteristic value encode function. Do not edit comment generated here */
    FSP_PARAMETER_NOT_USED(p_app_value);
    FSP_PARAMETER_NOT_USED(p_gatt_value);
    /* End user code. Do not edit comment generated here */
    return BLE_SUCCESS;
}

/* Request characteristic descriptor definition */
static const st_ble_servs_desc_info_t *gspp_qc_req_descs[] = { 
    &gs_qc_req_char_user_desc,
};

/* Request characteristic definition */
static const st_ble_servs_char_info_t gs_qc_req_char = {
    .start_hdl    = BLE_QC_SVCS_QC_REQ_DECL_HDL,
    .end_hdl      = BLE_QC_SVCS_QC_REQ_CHAR_USER_DESC_DESC_HDL,
    .char_idx     = BLE_QC_SVCS_QC_REQ_IDX,
    .app_size     = sizeof(st_ble_qc_svcs_qc_req_t),
    .db_size      = BLE_QC_SVCS_QC_REQ_LEN,
    .decode       = (ble_servs_attr_decode_t)decode_st_ble_qc_svcs_qc_req_t,
    .encode       = (ble_servs_attr_encode_t)encode_st_ble_qc_svcs_qc_req_t,
    .pp_descs     = gspp_qc_req_descs,
    .num_of_descs = ARRAY_SIZE(gspp_qc_req_descs),
};

/*----------------------------------------------------------------------------------------------------------------------
    Response Client Characteristic Configuration descriptor : Client Characteristic Configuration Descriptor
----------------------------------------------------------------------------------------------------------------------*/

static const st_ble_servs_desc_info_t gs_qc_rsp_cli_cnfg = {
    .attr_hdl = BLE_QC_SVCS_QC_RSP_CLI_CNFG_DESC_HDL,
    .app_size = sizeof(uint16_t),
    .desc_idx = BLE_QC_SVCS_QC_RSP_CLI_CNFG_IDX,
    .db_size  = BLE_QC_SVCS_QC_RSP_CLI_CNFG_LEN,
    .decode   = (ble_servs_attr_decode_t)decode_uint16_t,
    .encode   = (ble_servs_attr_encode_t)encode_uint16_t,
};

ble_status_t R_BLE_QC_SVCS_SetQc_rspCliCnfg(uint16_t conn_hdl, const uint16_t *p_value)
{
    return R_BLE_SERVS_SetDesc(&gs_qc_rsp_cli_cnfg, conn_hdl, (const void *)p_value);
}

ble_status_t R_BLE_QC_SVCS_GetQc_rspCliCnfg(uint16_t conn_hdl, uint16_t *p_value)
{
    return R_BLE_SERVS_GetDesc(&gs_qc_rsp_cli_cnfg, conn_hdl, (void *)p_value);
}

/*----------------------------------------------------------------------------------------------------------------------
    Response Characteristic User Description descriptor : Characteristic User Description
----------------------------------------------------------------------------------------------------------------------*/

static ble_status_t decode_st_ble_qc_svcs_qc_rsp_char_user_desc_t(st_ble_qc_svcs_qc_rsp_char_user_desc_t *p_app_value, const st_ble_gatt_value_t *p_gatt_value)
{
    /* Start user code for Response Characteristic User Description descriptor value decode function. Do not edit comment generated here */
    FSP_PARAMETER_NOT_USED(p_app_value);
    FSP_PARAMETER_NOT_USED(p_gatt_value);
    /* End user code. Do not edit comment generated here */
    return BLE_SUCCESS;
}

static ble_status_t encode_st_ble_qc_svcs_qc_rsp_char_user_desc_t(const st_ble_qc_svcs_qc_rsp_char_user_desc_t *p_app_value, st_ble_gatt_value_t *p_gatt_value)
{
    /* Start user code for Response Characteristic User Description descriptor value encode function. Do not edit comment generated here */
    FSP_PARAMETER_NOT_USED(p_app_value);
    FSP_PARAMETER_NOT_USED(p_gatt_value);
    /* End user code. Do not edit comment generated here */
    return BLE_SUCCESS;
}

static const st_ble_servs_desc_info_t gs_qc_rsp_char_user_desc = {
    .attr_hdl = BLE_QC_SVCS_QC_RSP_CHAR_USER_DESC_DESC_HDL,
    .app_size = sizeof(st_ble_qc_svcs_qc_rsp_char_user_desc_t),
    .desc_idx = BLE_QC_SVCS_QC_RSP_CHAR_USER_DESC_IDX,
    .db_size  = BLE_QC_SVCS_QC_RSP_CHAR_USER_DESC_LEN,
    .decode   = (ble_servs_attr_decode_t)decode_st_ble_qc_svcs_qc_rsp_char_user_desc_t,
    .encode   = (ble_servs_attr_encode_t)encode_st_ble_qc_svcs_qc_rsp_char_user_desc_t,
};

ble_status_t R_BLE_QC_SVCS_SetQc_rspCharUserDesc(const st_ble_qc_svcs_qc_rsp_char_user_desc_t *p_value)
{
    return R_BLE_SERVS_SetDesc(&gs_qc_rsp_char_user_desc, BLE_GAP_INVALID_CONN_HDL, (const void *)p_value);
}

ble_status_t R_BLE_QC_SVCS_GetQc_rspCharUserDesc(st_ble_qc_svcs_qc_rsp_char_user_desc_t *p_value)
{
    return R_BLE_SERVS_GetDesc(&gs_qc_rsp_char_user_desc, BLE_GAP_INVALID_CONN_HDL, (void *)p_value);
}

/*----------------------------------------------------------------------------------------------------------------------
    Response characteristic
----------------------------------------------------------------------------------------------------------------------*/

static ble_status_t decode_st_ble_qc_svcs_qc_rsp_t(st_ble_qc_svcs_qc_rsp_t *p_app_value, const st_ble_gatt_value_t *p_gatt_value)
{
    /* Start user code for Response characteristic value decode function. Do not edit comment generated here */
    FSP_PARAMETER_NOT_USED(p_app_value);
    FSP_PARAMETER_NOT_USED(p_gatt_value);
    /* End user code. Do not edit comment generated here */
    return BLE_SUCCESS;
}

static ble_status_t encode_st_ble_qc_svcs_qc_rsp_t(const st_ble_qc_svcs_qc_rsp_t *p_app_value, st_ble_gatt_value_t *p_gatt_value)
{
    /* Start user code for Response characteristic value encode function. Do not edit comment generated here */
    FSP_PARAMETER_NOT_USED(p_app_value);
    FSP_PARAMETER_NOT_USED(p_gatt_value);
    /* End user code. Do not edit comment generated here */
    return BLE_SUCCESS;
}

/* Response characteristic descriptor definition */
static const st_ble_servs_desc_info_t *gspp_qc_rsp_descs[] = { 
    &gs_qc_rsp_cli_cnfg, 
    &gs_qc_rsp_char_user_desc,
};

/* Response characteristic definition */
static const st_ble_servs_char_info_t gs_qc_rsp_char = {
    .start_hdl    = BLE_QC_SVCS_QC_RSP_DECL_HDL,
    .end_hdl      = BLE_QC_SVCS_QC_RSP_CHAR_USER_DESC_DESC_HDL,
    .char_idx     = BLE_QC_SVCS_QC_RSP_IDX,
    .app_size     = sizeof(st_ble_qc_svcs_qc_rsp_t),
    .db_size      = BLE_QC_SVCS_QC_RSP_LEN,
    .decode       = (ble_servs_attr_decode_t)decode_st_ble_qc_svcs_qc_rsp_t,
    .encode       = (ble_servs_attr_encode_t)encode_st_ble_qc_svcs_qc_rsp_t,
    .pp_descs     = gspp_qc_rsp_descs,
    .num_of_descs = ARRAY_SIZE(gspp_qc_rsp_descs),
};

ble_status_t R_BLE_QC_SVCS_SetQc_rsp(const st_ble_qc_svcs_qc_rsp_t *p_value)
{
    return R_BLE_SERVS_SetChar(&gs_qc_rsp_char, BLE_GAP_INVALID_CONN_HDL, (const void *)p_value);
}

ble_status_t R_BLE_QC_SVCS_GetQc_rsp(st_ble_qc_svcs_qc_rsp_t *p_value)
{
    return R_BLE_SERVS_GetChar(&gs_qc_rsp_char, BLE_GAP_INVALID_CONN_HDL, (void *)p_value);
}

ble_status_t R_BLE_QC_SVCS_NotifyQc_rsp(uint16_t conn_hdl, const st_ble_qc_svcs_qc_rsp_t *p_value)
{
    return R_BLE_SERVS_SendHdlVal(&gs_qc_rsp_char, conn_hdl, (const void *)p_value, true);
}

/*----------------------------------------------------------------------------------------------------------------------
    Quick Connect Service server
----------------------------------------------------------------------------------------------------------------------*/

/* Quick Connect Service characteristics definition */
static const st_ble_servs_char_info_t *gspp_chars[] = {
    &gs_qc_req_char,
    &gs_qc_rsp_char,
};

/* Quick Connect Service service definition */
static st_ble_servs_info_t gs_servs_info = {
    .pp_chars     = gspp_chars,
    .num_of_chars = ARRAY_SIZE(gspp_chars),
};

ble_status_t R_BLE_QC_SVCS_Init(ble_servs_app_cb_t cb)
{
    if (NULL == cb)
    {
        return BLE_ERR_INVALID_PTR;
    }

    gs_servs_info.cb = cb;

    return R_BLE_SERVS_RegisterServer(&gs_servs_info);
}
