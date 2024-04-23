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
* https://www.renesas.com/disclaimer
*
* Copyright (C) 2021 Renesas Electronics Corporation.
***********************************************************************************************************************/
/***********************************************************************************************************************
* File Name    : qe_ble_profile.c
* Description  : This file includes definitions.
***********************************************************************************************************************/
/***********************************************************************************************************************
* History      : MM/DD/YYYY Version Description
*              : 08/31/2021 1.00    First Release
***********************************************************************************************************************/

#include "qe_ble_profile.h"

const attribute_t qe_ble_profile[QE_ATTRIBUTE_HANDLE_PROFILE_END] =
{

	// Profile Declaration
	[0] =
	{
		.handle = 0,
		.encapsulated_attributes = 23,
		.permissions = 0x00, 
		.uuid_length = 0x00,
		.value_length = 0x00,
		.notify_write = 0,
		.notify_read = 0
	},
	// Service Declaration: GAP Service
	[1] =
	{
		.handle = 1,
		.encapsulated_attributes = 11,
		.permissions = 0x01,
		.uuid_length = QE_BLE_PROFILE_UUID_SIZE_ADOPTED,
		.value_length = 0x02,
		.uuid = (__attribute__((aligned(2))) uint8_t []){0x00,0x28},
		.value = (__attribute__((aligned(2))) uint8_t []){0x00,0x18},
		.notify_write = 1,
		.notify_read = 1
	},
	// Characteristic Declaration: Device Name
	[2] =
	{
		.handle = 2,
		.encapsulated_attributes = 0,
		.permissions = 0x01,
		.uuid_length = QE_BLE_PROFILE_UUID_SIZE_ADOPTED,
		.value_length = 0x05,
		.uuid = (__attribute__((aligned(2))) uint8_t [] ){0x03,0x28},
		.value = (__attribute__((aligned(2))) uint8_t []){0x0A,0x03,0x00,0x00,0x2A},
		.notify_write = 1,
		.notify_read = 1
	},
	// Characteristic Value: Device Name
	[3] =
	{
		.handle = 3,
		.encapsulated_attributes = 0,
		.permissions = 0x03,
		.uuid_length = QE_BLE_PROFILE_UUID_SIZE_ADOPTED,
		.value_length = 0x80,
		.uuid = (__attribute__((aligned(2))) uint8_t []){0x00,0x2A},
		.value = (__attribute__((aligned(2))) uint8_t []){0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
		.notify_write = 1,
		.notify_read = 1
	},
	// Characteristic Declaration: Appearance
	[4] =
	{
		.handle = 4,
		.encapsulated_attributes = 0,
		.permissions = 0x01,
		.uuid_length = QE_BLE_PROFILE_UUID_SIZE_ADOPTED,
		.value_length = 0x05,
		.uuid = (__attribute__((aligned(2))) uint8_t []){0x03,0x28},
		.value = (__attribute__((aligned(2))) uint8_t []){0x02,0x05,0x00,0x01,0x2A},
		.notify_write = 1,
		.notify_read = 1
	},
	// Characteristic Value: Appearance
	[5] =
	{
		.handle = 5,
		.encapsulated_attributes = 0,
		.permissions = 0x01,
		.uuid_length = QE_BLE_PROFILE_UUID_SIZE_ADOPTED,
		.value_length = 0x02,
		.uuid = (__attribute__((aligned(2))) uint8_t []){0x01,0x2A},
		.value = (__attribute__((aligned(2))) uint8_t []){0x00,0x00},
		.notify_write = 1,
		.notify_read = 1
	},
	// Characteristic Declaration: Peripheral Preferred Connection Parameters
	[6] =
	{
		.handle = 6,
		.encapsulated_attributes = 0,
		.permissions = 0x01,
		.uuid_length = QE_BLE_PROFILE_UUID_SIZE_ADOPTED,
		.value_length = 0x05,
		.uuid = (__attribute__((aligned(2))) uint8_t []){0x03,0x28},
		.value = (__attribute__((aligned(2))) uint8_t []){0x02,0x07,0x00,0x04,0x2A},
		.notify_write = 1,
		.notify_read = 1
	},
	// Characteristic Value: Peripheral Preferred Connection Parameters
	[7] =
	{
		.handle = 7,
		.encapsulated_attributes = 0,
		.permissions = 0x01,
		.uuid_length = QE_BLE_PROFILE_UUID_SIZE_ADOPTED,
		.value_length = 0x08,
		.uuid = (__attribute__((aligned(2))) uint8_t []){0x04,0x2A},
		.value = (__attribute__((aligned(2))) uint8_t []){0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
		.notify_write = 1,
		.notify_read = 1
	},
	// Characteristic Declaration: Central Address Resolution
	[8] =
	{
		.handle = 8,
		.encapsulated_attributes = 0,
		.permissions = 0x01,
		.uuid_length = QE_BLE_PROFILE_UUID_SIZE_ADOPTED,
		.value_length = 0x05,
		.uuid = (__attribute__((aligned(2))) uint8_t []){0x03,0x28},
		.value = (__attribute__((aligned(2))) uint8_t []){0x02,0x09,0x00,0xA6,0x2A},
		.notify_write = 1,
		.notify_read = 1
	},
	// Characteristic Value: Central Address Resolution
	[9] =
	{
		.handle = 9,
		.encapsulated_attributes = 0,
		.permissions = 0x01,
		.uuid_length = QE_BLE_PROFILE_UUID_SIZE_ADOPTED,
		.value_length = 0x01,
		.uuid = (__attribute__((aligned(2))) uint8_t []){0xA6,0x2A},
		.value = (__attribute__((aligned(2))) uint8_t []){0x00},
		.notify_write = 1,
		.notify_read = 1
	},
	// Characteristic Declaration: Resolvable Private Address Only
	[10] =
	{
		.handle = 10,
		.encapsulated_attributes = 0,
		.permissions = 0x01,
		.uuid_length = QE_BLE_PROFILE_UUID_SIZE_ADOPTED,
		.value_length = 0x05,
		.uuid = (__attribute__((aligned(2))) uint8_t []){0x03,0x28},
		.value = (__attribute__((aligned(2))) uint8_t []){0x02,0x0B,0x00,0xC9,0x2A},
		.notify_write = 1,
		.notify_read = 1
	},
	// Characteristic Value: Resolvable Private Address Only
	[11] =
	{
		.handle = 11,
		.encapsulated_attributes = 0,
		.permissions = 0x01,
		.uuid_length = QE_BLE_PROFILE_UUID_SIZE_ADOPTED,
		.value_length = 0x01,
		.uuid = (__attribute__((aligned(2))) uint8_t []){0xC9,0x2A},
		.value = (__attribute__((aligned(2))) uint8_t []){0x00},
		.notify_write = 1,
		.notify_read = 1
	},
	// Service Declaration: GATT Service
	[12] =
	{
		.handle = 12,
		.encapsulated_attributes = 4,
		.permissions = 0x01,
		.uuid_length = QE_BLE_PROFILE_UUID_SIZE_ADOPTED,
		.value_length = 0x02,
		.uuid = (__attribute__((aligned(2))) uint8_t []){0x00,0x28},
		.value = (__attribute__((aligned(2))) uint8_t []){0x01,0x18},
		.notify_write = 1,
		.notify_read = 1
	},
	// Characteristic Declaration: Service Changed
	[13] =
	{
		.handle = 13,
		.encapsulated_attributes = 0,
		.permissions = 0x01,
		.uuid_length = QE_BLE_PROFILE_UUID_SIZE_ADOPTED,
		.value_length = 0x05,
		.uuid = (__attribute__((aligned(2))) uint8_t []){0x03,0x28},
		.value = (__attribute__((aligned(2))) uint8_t []){0x20,0x0E,0x00,0x05,0x2A},
		.notify_write = 1,
		.notify_read = 1
	},
	// Characteristic Value: Service Changed
	[14] =
	{
		.handle = 14,
		.encapsulated_attributes = 0,
		.permissions = 0x00,
		.uuid_length = QE_BLE_PROFILE_UUID_SIZE_ADOPTED,
		.value_length = 0x04,
		.uuid = (__attribute__((aligned(2))) uint8_t []){0x05,0x2A},
		.value = (__attribute__((aligned(2))) uint8_t []){0x00,0x00,0x00,0x00},
		.notify_write = 1,
		.notify_read = 1
	},
	// Descriptor: Client Characteristic Configuration
	[15] =
	{
		.handle = 15,
		.encapsulated_attributes = 0,
		.permissions = 0x03,
		.uuid_length = QE_BLE_PROFILE_UUID_SIZE_ADOPTED,
		.value_length = 0x02,
		.uuid = (__attribute__((aligned(2))) uint8_t []){0x02,0x29},
		.value = (__attribute__((aligned(2))) uint8_t []){0x00,0x00},
		.notify_write = 1,
		.notify_read = 1
	},
	// Service Declaration: Quick Connect Service
	[16] =
	{
		.handle = 16,
		.encapsulated_attributes = 8,
		.permissions = 0x01,
		.uuid_length = QE_BLE_PROFILE_UUID_SIZE_ADOPTED,
		.value_length = 0x10,
		.uuid = (__attribute__((aligned(2))) uint8_t []){0x00,0x28},
		.value = (__attribute__((aligned(2))) uint8_t []){0x59,0x5A,0x08,0xE4,0x86,0x2A,0x9E,0x8F,0xE9,0x11,0xBC,0x7C,0x98,0x43,0x42,0x18},
		.notify_write = 1,
		.notify_read = 1
	},
	// Characteristic Declaration: Request
	[17] =
	{
		.handle = 17,
		.encapsulated_attributes = 0,
		.permissions = 0x01,
		.uuid_length = QE_BLE_PROFILE_UUID_SIZE_ADOPTED,
		.value_length = 0x13,
		.uuid = (__attribute__((aligned(2))) uint8_t []){0x03,0x28},
		.value = (__attribute__((aligned(2))) uint8_t []){0x08,0x12,0x00,0x20,0xEE,0x8D,0x0C,0xE1,0xF0,0x4A,0x0C,0xB3,0x25,0xDC,0x53,0x6A,0x68,0x86,0x2D},
		.notify_write = 1,
		.notify_read = 1
	},
	// Characteristic Value: Request
	[18] =
	{
		.handle = 18,
		.encapsulated_attributes = 0,
		.permissions = 0x02,
		.uuid_length = QE_BLE_PROFILE_UUID_SIZE_CUSTOM,
		.value_length = 0x20,
		.uuid = (__attribute__((aligned(2))) uint8_t []){0x20,0xEE,0x8D,0x0C,0xE1,0xF0,0x4A,0x0C,0xB3,0x25,0xDC,0x53,0x6A,0x68,0x86,0x2D},
		.value = (__attribute__((aligned(2))) uint8_t []){0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
		.notify_write = 1,
		.notify_read = 1
	},
	// Descriptor: Characteristic User Description
	[19] =
	{
		.handle = 19,
		.encapsulated_attributes = 0,
		.permissions = 0x03,
		.uuid_length = QE_BLE_PROFILE_UUID_SIZE_ADOPTED,
		.value_length = 0x64,
		.uuid = (__attribute__((aligned(2))) uint8_t []){0x01,0x29},
		.value = (__attribute__((aligned(2))) uint8_t []){0x52,0x65,0x71,0x75,0x65,0x73,0x74,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
		.notify_write = 1,
		.notify_read = 1
	},
	// Characteristic Declaration: Response
	[20] =
	{
		.handle = 20,
		.encapsulated_attributes = 0,
		.permissions = 0x01,
		.uuid_length = QE_BLE_PROFILE_UUID_SIZE_ADOPTED,
		.value_length = 0x13,
		.uuid = (__attribute__((aligned(2))) uint8_t []){0x03,0x28},
		.value = (__attribute__((aligned(2))) uint8_t []){0x12,0x15,0x00,0x22,0xEE,0x8D,0x0C,0xE1,0xF0,0x4A,0x0C,0xB3,0x25,0xDC,0x53,0x6A,0x68,0x86,0x2D},
		.notify_write = 1,
		.notify_read = 1
	},
	// Characteristic Value: Response
	[21] =
	{
		.handle = 21,
		.encapsulated_attributes = 0,
		.permissions = 0x01,
		.uuid_length = QE_BLE_PROFILE_UUID_SIZE_CUSTOM,
		.value_length = 0x65,
		.uuid = (__attribute__((aligned(2))) uint8_t []){0x22,0xEE,0x8D,0x0C,0xE1,0xF0,0x4A,0x0C,0xB3,0x25,0xDC,0x53,0x6A,0x68,0x86,0x2D},
		.value = (__attribute__((aligned(2))) uint8_t []){0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
		.notify_write = 1,
		.notify_read = 1
	},
	// Descriptor: Client Characteristic Configuration
	[22] =
	{
		.handle = 22,
		.encapsulated_attributes = 0,
		.permissions = 0x03,
		.uuid_length = QE_BLE_PROFILE_UUID_SIZE_ADOPTED,
		.value_length = 0x02,
		.uuid = (__attribute__((aligned(2))) uint8_t []){0x02,0x29},
		.value = (__attribute__((aligned(2))) uint8_t []){0x00,0x00},
		.notify_write = 1,
		.notify_read = 1
	},
	// Descriptor: Characteristic User Description
	[23] =
	{
		.handle = 23,
		.encapsulated_attributes = 0,
		.permissions = 0x03,
		.uuid_length = QE_BLE_PROFILE_UUID_SIZE_ADOPTED,
		.value_length = 0x64,
		.uuid = (__attribute__((aligned(2))) uint8_t []){0x01,0x29},
		.value = (__attribute__((aligned(2))) uint8_t []){0x52,0x65,0x73,0x70,0x6F,0x6E,0x73,0x65,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
		.notify_write = 1,
		.notify_read = 1
	}

};
