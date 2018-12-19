

#ifndef __APP_UART_H__
#define __APP_UART_H__

#include <rthw.h>
#include <rtthread.h>


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

