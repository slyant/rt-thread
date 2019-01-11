#ifndef __UART_LCD_DEVICE_H__
#define __UART_LCD_DEVICE_H__

#define LCD_UPDATE_EVENT	(1<<0)		//屏更新事件标志

struct lcd_device
{
	rt_sem_t rx_notice;
	rt_device_t uart_device;
	unsigned char *cmd_buffer;
	int cmd_bufsize;
};
typedef struct lcd_device *lcd_device_t;

#endif
