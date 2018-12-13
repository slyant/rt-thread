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
 * 2015-07-29     Arda.Fu      first implementation
 */
#include <rtthread.h>

/*
USART1:EM20扫码
USART2:空/Debug
USART3:4G
UART4:LCD
USART6:Debug/RS485
UART7:2G

I2C3:PCF8563时钟

SPI1：SpiFlash
SPI2:ICCard

*/
#define POWER_4G_PIN		(34)
#define RESET_4G_PIN		(57)
#include <rtdevice.h>

int main(void)
{	
	  rt_pin_mode(POWER_4G_PIN, PIN_MODE_OUTPUT);
		rt_pin_write(POWER_4G_PIN, 0);
	
		rt_pin_mode(RESET_4G_PIN, PIN_MODE_OUTPUT);
		rt_pin_write(RESET_4G_PIN, 0);
	
    rt_kprintf("hello rt-thread\n");
	
    return 0;	
}

