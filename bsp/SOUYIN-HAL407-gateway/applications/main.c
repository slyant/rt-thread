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
	//extern void app_sqlite_init(void);
	//extern void nrf_gateway_startup(void);
	//extern int lcd_device_startup(void);
	//extern int unit_test(void);
	extern int sqlite3_initialize(void);
	extern int sqlite3_os_init(void);
	
	//app_sqlite_init();
	//nrf_gateway_startup();
	//lcd_device_startup();
	int r0 = sqlite3_initialize();
	//int r1 = sqlite3_os_init();
	rt_kprintf("r0:%d\n", r0);
	//rt_kprintf("r1:%d\n", r1);
	//unit_test();
//	while(1)
//	{
//		rt_thread_mdelay(10);
//	}
    return 0;
}

