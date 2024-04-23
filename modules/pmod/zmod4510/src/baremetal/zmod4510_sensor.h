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
#ifndef ZMOD4510_SENSOR_H_
#define ZMOD4510_SENSOR_H_

#include "common_utils.h"

#ifndef ZMOD4510_CFG_DEVICE_TYPE
#define ZMOD4510_CFG_DEVICE_TYPE    (1U)
#endif

/* Quick setup for g_zmod4510_sensor0.
 * - g_comms_i2c_bus0 must be setup before calling this function
 *     (See Developer Assistance -> g_zmod4510_sensor0 -> g_comms_i2c_device0 -> g_comms_i2c_bus0 -> Quick Setup).
 */
void g_zmod4510_sensor0_quick_setup(void);

/* Quick getting air quality values for g_zmod4510_sensor0.
 * - g_zmod4510_sensor0 must be setup before calling this function.
 */
void g_zmod4510_sensor0_quick_getting_air_quality_data(rm_zmod4xxx_oaq_2nd_data_t * p_data);


#endif /* ZMOD4510_SENSOR_H_ */

