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
 * File Name: r_ble_qc_svcs.h
 * Version : 1.0
 * Description : The header file for Quick Connect Service service.
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * History : DD.MM.YYYY Version Description
 *         : 31.12.2999 1.00 First Release
 ***********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @file
 * @defgroup qc_svcs Quick Connect Service Service
 * @{
 * @ingroup profile
 * @brief   
 **********************************************************************************************************************/
#include "profile_cmn/r_ble_servs_if.h"
#include "gatt_db.h"

#ifndef R_BLE_QC_SVCS_H
#define R_BLE_QC_SVCS_H

/*----------------------------------------------------------------------------------------------------------------------
    Request Characteristic
        Characteristic User Description descriptor : Characteristic User Description
----------------------------------------------------------------------------------------------------------------------*/

/***************************************************************************//**
 * @brief Characteristic User Description value structure.
*******************************************************************************/
typedef struct {
    uint8_t user_description[98]; /**< User Description */
    uint16_t length; /**< Length */
} st_ble_qc_svcs_qc_req_char_user_desc_t;

/***************************************************************************//**
 * @brief Request value structure.
*******************************************************************************/
typedef struct {
    uint8_t request[32]; /**< request */
} st_ble_qc_svcs_qc_req_t;

/***************************************************************************//**
 * @brief     Set Request char user desc descriptor value to the local GATT database.
 * @param[in] p_value  Descriptor value to set.
 * @return    @ref ble_status_t
*******************************************************************************/
ble_status_t R_BLE_QC_SVCS_SetQc_reqCharUserDesc(const st_ble_qc_svcs_qc_req_char_user_desc_t *p_value);

/***************************************************************************//**
 * @brief     Get Request char user desc descriptor value from the local GATT database.
 * @param[out] p_value  Output location for the acquired descriptor value.
 * @return    @ref ble_status_t
*******************************************************************************/
ble_status_t R_BLE_QC_SVCS_GetQc_reqCharUserDesc(st_ble_qc_svcs_qc_req_char_user_desc_t *p_value);

/*----------------------------------------------------------------------------------------------------------------------
    Response Characteristic
        Client Characteristic Configuration descriptor : Client Characteristic Configuration Descriptor
        Characteristic User Description descriptor : Characteristic User Description
----------------------------------------------------------------------------------------------------------------------*/


/***************************************************************************//**
 * @brief Characteristic User Description value structure.
*******************************************************************************/
typedef struct {
    uint8_t user_description[98]; /**< User Description */
    uint16_t length; /**< Length */
} st_ble_qc_svcs_qc_rsp_char_user_desc_t;

/***************************************************************************//**
 * @brief Response value structure.
*******************************************************************************/
typedef struct {
    uint8_t response[101]; /**< response */
} st_ble_qc_svcs_qc_rsp_t;

/***************************************************************************//**
 * @brief     Set Response characteristic value to the local GATT database.
 * @param[in] p_value  Characteristic value to set.
 * @return    @ref ble_status_t
*******************************************************************************/
ble_status_t R_BLE_QC_SVCS_SetQc_rsp(const st_ble_qc_svcs_qc_rsp_t *p_value);

/***************************************************************************//**
 * @brief     Get Response characteristic value from the local GATT database.
 * @param[out] p_value  Output location for the acquired descriptor value.
 * @return    @ref ble_status_t
*******************************************************************************/
ble_status_t R_BLE_QC_SVCS_GetQc_rsp(st_ble_qc_svcs_qc_rsp_t *p_value);

/***************************************************************************//**
 * @brief     Send notification of  Response characteristic value to the remote device.
 * @param[in] conn_hdl Connection handle.
 * @param[in] p_value  Characteristic value to send.
 * @return    @ref ble_status_t
*******************************************************************************/
ble_status_t R_BLE_QC_SVCS_NotifyQc_rsp(uint16_t conn_hdl, const st_ble_qc_svcs_qc_rsp_t *p_value);

/***************************************************************************//**
 * @brief     Set Response cli cnfg descriptor value to the local GATT database.
 * @param[in] conn_hdl Connection handle.
 * @param[in] p_value  Descriptor value to set.
 * @return    @ref ble_status_t
*******************************************************************************/
ble_status_t R_BLE_QC_SVCS_SetQc_rspCliCnfg(uint16_t conn_hdl, const uint16_t *p_value);

/***************************************************************************//**
 * @brief     Get Response cli cnfg descriptor value from the local GATT database.
 * @param[in] conn_hdl Connection handle.
 * @param[out] p_value  Output location for the acquired descriptor value.
 * @return    @ref ble_status_t
*******************************************************************************/
ble_status_t R_BLE_QC_SVCS_GetQc_rspCliCnfg(uint16_t conn_hdl, uint16_t *p_value);

/***************************************************************************//**
 * @brief     Set Response char user desc descriptor value to the local GATT database.
 * @param[in] p_value  Descriptor value to set.
 * @return    @ref ble_status_t
*******************************************************************************/
ble_status_t R_BLE_QC_SVCS_SetQc_rspCharUserDesc(const st_ble_qc_svcs_qc_rsp_char_user_desc_t *p_value);

/***************************************************************************//**
 * @brief     Get Response char user desc descriptor value from the local GATT database.
 * @param[out] p_value  Output location for the acquired descriptor value.
 * @return    @ref ble_status_t
*******************************************************************************/
ble_status_t R_BLE_QC_SVCS_GetQc_rspCharUserDesc(st_ble_qc_svcs_qc_rsp_char_user_desc_t *p_value);

/*----------------------------------------------------------------------------------------------------------------------
    Quick Connect Service Service
----------------------------------------------------------------------------------------------------------------------*/

/***************************************************************************//**
 * @brief Quick Connect Service characteristic Index.
*******************************************************************************/
typedef enum {
    BLE_QC_SVCS_QC_REQ_IDX,
    BLE_QC_SVCS_QC_REQ_CHAR_USER_DESC_IDX,
    BLE_QC_SVCS_QC_RSP_IDX,
    BLE_QC_SVCS_QC_RSP_CLI_CNFG_IDX,
    BLE_QC_SVCS_QC_RSP_CHAR_USER_DESC_IDX,
} e_ble_qc_svcs_char_idx_t;

/***************************************************************************//**
 * @brief Quick Connect Service event type.
*******************************************************************************/
typedef enum {
    /* Request */
    BLE_QC_SVCS_EVENT_QC_REQ_WRITE_REQ = BLE_SERVS_ATTR_EVENT(BLE_QC_SVCS_QC_REQ_IDX, BLE_SERVS_WRITE_REQ),
    BLE_QC_SVCS_EVENT_QC_REQ_WRITE_COMP = BLE_SERVS_ATTR_EVENT(BLE_QC_SVCS_QC_REQ_IDX, BLE_SERVS_WRITE_COMP),
    BLE_QC_SVCS_EVENT_QC_REQ_CHAR_USER_DESC_WRITE_REQ = BLE_SERVS_ATTR_EVENT(BLE_QC_SVCS_QC_REQ_CHAR_USER_DESC_IDX, BLE_SERVS_WRITE_REQ),
    BLE_QC_SVCS_EVENT_QC_REQ_CHAR_USER_DESC_WRITE_COMP = BLE_SERVS_ATTR_EVENT(BLE_QC_SVCS_QC_REQ_CHAR_USER_DESC_IDX, BLE_SERVS_WRITE_COMP),
    BLE_QC_SVCS_EVENT_QC_REQ_CHAR_USER_DESC_READ_REQ = BLE_SERVS_ATTR_EVENT(BLE_QC_SVCS_QC_REQ_CHAR_USER_DESC_IDX, BLE_SERVS_READ_REQ),
    /* Response */
    BLE_QC_SVCS_EVENT_QC_RSP_READ_REQ = BLE_SERVS_ATTR_EVENT(BLE_QC_SVCS_QC_RSP_IDX, BLE_SERVS_READ_REQ),
    BLE_QC_SVCS_EVENT_QC_RSP_CLI_CNFG_WRITE_REQ = BLE_SERVS_ATTR_EVENT(BLE_QC_SVCS_QC_RSP_CLI_CNFG_IDX, BLE_SERVS_WRITE_REQ),
    BLE_QC_SVCS_EVENT_QC_RSP_CLI_CNFG_WRITE_COMP = BLE_SERVS_ATTR_EVENT(BLE_QC_SVCS_QC_RSP_CLI_CNFG_IDX, BLE_SERVS_WRITE_COMP),
    BLE_QC_SVCS_EVENT_QC_RSP_CLI_CNFG_READ_REQ = BLE_SERVS_ATTR_EVENT(BLE_QC_SVCS_QC_RSP_CLI_CNFG_IDX, BLE_SERVS_READ_REQ),
    BLE_QC_SVCS_EVENT_QC_RSP_CHAR_USER_DESC_WRITE_REQ = BLE_SERVS_ATTR_EVENT(BLE_QC_SVCS_QC_RSP_CHAR_USER_DESC_IDX, BLE_SERVS_WRITE_REQ),
    BLE_QC_SVCS_EVENT_QC_RSP_CHAR_USER_DESC_WRITE_COMP = BLE_SERVS_ATTR_EVENT(BLE_QC_SVCS_QC_RSP_CHAR_USER_DESC_IDX, BLE_SERVS_WRITE_COMP),
    BLE_QC_SVCS_EVENT_QC_RSP_CHAR_USER_DESC_READ_REQ = BLE_SERVS_ATTR_EVENT(BLE_QC_SVCS_QC_RSP_CHAR_USER_DESC_IDX, BLE_SERVS_READ_REQ),
} e_ble_qc_svcs_event_t;

/***************************************************************************//**
 * @brief     Initialize Quick Connect Service service.
 * @param[in] cb Service callback.
 * @return    @ref ble_status_t
*******************************************************************************/
ble_status_t R_BLE_QC_SVCS_Init(ble_servs_app_cb_t cb);

#endif /* R_BLE_QC_SVCS_H */

/** @} */
