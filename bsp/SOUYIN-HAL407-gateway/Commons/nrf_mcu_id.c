#include <rtthread.h>
#include "st_mcuid.h"

void nrf_get_mcuid(rt_uint32_t *id)
{
	get_stm32_mcuid(id, STM32F4_MCU);
}
