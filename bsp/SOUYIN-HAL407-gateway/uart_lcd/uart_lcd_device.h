#ifndef __UART_LCD_DEVICE_H__
#define __UART_LCD_DEVICE_H__

#define LCD_UPDATE_EVENT	(1<<0)		//屏更新事件标志

struct lcd_device_s
{
	rt_event_t lcd_event;
	rt_sem_t rx_notice;
	rt_device_t uart_device;
	unsigned char *cmd_buffer;
	int cmd_bufsize;
	rt_uint8_t screen_id;
	rt_uint8_t ui_tag;
};
typedef struct lcd_device_s *lcd_device_t;

int lcd_device_startup(void);

#endif

