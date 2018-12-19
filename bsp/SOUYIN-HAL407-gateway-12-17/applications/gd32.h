

#ifndef GD32_H_
#define GD32_H_

#include <rtthread.h>
#include "lcd_oper.h"

#ifdef NRF24L01_USING_GATEWAY
	void sark_param_store(sark_msg_t arg);
	void sark_param_read(uint8_t which,uint8_t* buf);
	void sark_param_clear(uint8_t which);
#else
	void w25_store_param(rt_uint8_t addr,rt_uint32_t pwd);
	void w25_read_param(rt_uint8_t *addr,rt_uint32_t *pwd);
	void w25_clear_store_data(void);
#endif


#endif
