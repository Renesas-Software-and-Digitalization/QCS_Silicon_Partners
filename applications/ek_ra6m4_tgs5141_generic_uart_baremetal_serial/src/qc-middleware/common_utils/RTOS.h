/*
* Copyright (c) 2020 - 2024 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier:  BSD-3-Clause
*/
/*
    This header is used for selecting the right RTOS
*/
#if (BSP_CFG_RTOS) == 1
#include "tx_api.h"
#elif (BSP_CFG_RTOS) == 2
#include "FreeRTOS.h"
#include "task.h"
#endif