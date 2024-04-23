
#ifndef COMMON_UTILS_H_
#define COMMON_UTILS_H_

#include "hal_data.h"
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>


#define ON          (1)
#define OFF         (0)
#if (defined(BOARD_RA8M1_EK) || defined(BOARD_RA8D1_EK))
#define GREEN_LED   BSP_IO_PORT_04_PIN_14
#define RED_LED     BSP_IO_PORT_01_PIN_07
#define BLUE_LED    BSP_IO_PORT_06_PIN_00
#elif defined(BOARD_RA6M5_EK)
#define GREEN_LED   BSP_IO_PORT_00_PIN_07
#define RED_LED     BSP_IO_PORT_00_PIN_08
#define BLUE_LED    BSP_IO_PORT_00_PIN_06
#elif defined(BOARD_RA2E1_EK)
#define GREEN_LED   BSP_IO_PORT_09_PIN_14
#define RED_LED     BSP_IO_PORT_09_PIN_13
#define BLUE_LED    BSP_IO_PORT_09_PIN_15
#elif defined(BOARD_RA4M2_EK)
#define GREEN_LED   BSP_IO_PORT_04_PIN_04
#define RED_LED     BSP_IO_PORT_04_PIN_05
#define BLUE_LED    BSP_IO_PORT_04_PIN_15
#elif defined(BOARD_RA6M4_EK)
#define GREEN_LED   BSP_IO_PORT_04_PIN_04
#define RED_LED     BSP_IO_PORT_04_PIN_00
#define BLUE_LED    BSP_IO_PORT_04_PIN_15
#elif defined(BOARD_RA2L1_EK)
#define GREEN_LED   BSP_IO_PORT_05_PIN_04
#define RED_LED     BSP_IO_PORT_05_PIN_05
#define BLUE_LED    BSP_IO_PORT_05_PIN_03
#elif defined(BOARD_RA6M3_EK)
#define GREEN_LED   BSP_IO_PORT_04_PIN_00
#define RED_LED     BSP_IO_PORT_01_PIN_00
#define BLUE_LED    BSP_IO_PORT_04_PIN_03
#elif defined(BOARD_RA4M3_EK)
#define GREEN_LED   BSP_IO_PORT_04_PIN_04
#define RED_LED     BSP_IO_PORT_04_PIN_00
#define BLUE_LED    BSP_IO_PORT_04_PIN_15
#elif defined(BOARD_RA6E2_FPB)
#define GREEN_LED   BSP_IO_PORT_02_PIN_07
#define RED_LED     BSP_IO_PORT_02_PIN_07
#define BLUE_LED    BSP_IO_PORT_02_PIN_06
#elif defined(BOARD_RA0E1_FPB)
#define GREEN_LED   BSP_IO_PORT_00_PIN_08
#define RED_LED     BSP_IO_PORT_00_PIN_09
#define BLUE_LED    BSP_IO_PORT_00_PIN_09
#elif defined(BOARD_RA6E2_EK)
#define GREEN_LED   BSP_IO_PORT_04_PIN_00
#define RED_LED     BSP_IO_PORT_01_PIN_13
#define BLUE_LED    BSP_IO_PORT_02_PIN_07
#elif defined(BOARD_RA6E2_BGK)
#define GREEN_LED   BSP_IO_PORT_01_PIN_04
#define RED_LED     BSP_IO_PORT_01_PIN_04
#define BLUE_LED    BSP_IO_PORT_01_PIN_12
#elif defined(BOARD_RA4E2_EK)
#define GREEN_LED   BSP_IO_PORT_01_PIN_04
#define RED_LED     BSP_IO_PORT_01_PIN_12
#define BLUE_LED    BSP_IO_PORT_02_PIN_07
#else
#error Board yet to be supported
#endif

void LED_Toggle(bsp_io_port_pin_t pin, uint8_t state);
void SysTime_Init(uint32_t freq);
uint32_t SysTime_get(void);

void halt_func(void);

#define LVL_ERR      (1u)       /* error conditions   */

#define RESET_VALUE             (0x00)

#define APP_PRINT(fn_, ...)      printf((fn_), ##__VA_ARGS__);

#define APP_ERR_PRINT(fn_, ...)  if(LVL_ERR)\
		printf ("[ERR] In Function: %s(), %s",__FUNCTION__,(fn_),##__VA_ARGS__);

#define APP_ERR_TRAP(err)        if(err) {\
		printf"\r\nReturned Error Code: 0x%x  \r\n", err);\
        __asm("BKPT #0\n");} /* trap upon the error  */

#endif /* COMMON_UTILS_H_ */
