/*
 * File      : main.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2009, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-29     Slyant       
 */
#include <rtthread.h>
#include <board.h>

int main(void)
{
    /* user app entry */
	extern void app_sqlite_init(void);
	extern void sys_lcd_startup(void);
	extern void nrf_gateway_startup(void);
	app_sqlite_init();
	sys_lcd_startup();
	nrf_gateway_startup();
//	while(1)
//	{
//		rt_thread_mdelay(10);
//	}
    return 0;
}

