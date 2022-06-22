#ifndef __STM32_HARDWARE_H__
#define __STM32_HARDWARE_H__

#ifdef __cplusplus
 extern "C" {
#endif

#include "stdint.h"
#if 0
	struct DEV_INFO devinfo __attribute__((aligned(4)));

	typedef enum
	{ 
	  FLASH_BUSY = 1,
	  FLASH_ERROR_PG,
	  FLASH_ERROR_WRP,
	  FLASH_COMPLETE,
	  FLASH_TIMEOUT
	}FLASH_Status;

	typedef enum {DISABLE = 0, ENABLE = !DISABLE} FunctionalState;

#endif
uint8_t   result_frame[100];

#ifdef __cplusplus
}
#endif

#endif
