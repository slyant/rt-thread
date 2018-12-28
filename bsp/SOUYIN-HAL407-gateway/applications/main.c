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
	rt_kprintf("hello rt-thread!\n");
	extern void app_startup(void);
	app_startup();

    return 0;
}

