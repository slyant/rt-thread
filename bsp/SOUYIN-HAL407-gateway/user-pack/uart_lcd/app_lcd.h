#ifndef __APP_LCD_H__
#define __APP_LCD_H__

#include <rthw.h>
#include <rtthread.h>
#include "cmd_queue.h"
#include "hmi_driver.h"

#define LCD_RX_EVENT		(1<<0)		//屏接收事件标志
#define LCD_UPDATE_EVENT	(1<<1)		//屏更新事件标志

typedef void (*send_char_handle)(rt_uint8_t);

struct lcd_device_s
{
	rt_event_t lcd_event;
	rt_sem_t rx_notice;
	rt_mutex_t lcd_lock;
	rt_device_t uart_device;
	unsigned char *cmd_buffer;
	int cmd_bufsize;
};
typedef struct lcd_device_s *lcd_device_t;




#define usart_rx_maxcount	5

typedef struct{
	unsigned short 			lcd_buf_index;
	unsigned char*			lcd_buffer;
	unsigned short			lcd_buf_size;
	unsigned char			lcd_rx_check_ms;
}lcd_rx_args_t;

typedef struct{
	unsigned char*			lcd_rx_byte_point;    
	unsigned char			lcd_rx_start_tag;	
	unsigned char			lcd_rx_check_counter;
	unsigned char			lcd_rx_end_tag;	
	lcd_rx_args_t			lcd_rx_args;
}lcd_rx_handle_t;

#endif

