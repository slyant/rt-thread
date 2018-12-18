#include <rthw.h>
#include <rtthread.h>
#include "app_lcd.h"
#include "hmi_driver.h"
#include "lcd_oper.h"

struct lcd_device_s lcd_device;

const char mcardwarning[] = {0xC5,0xE4,0xD6,0xC3,0xBF,0xA8,0xB2,0xBB,     //“配置卡不足，请配置”
					       0xD7,0xE3,0xA3,0xAC,0xC7,0xEB,0xC5,0xE4,0xD6,0xC3,0};
const char mcardnum[] = {0xC5,0xE4,0xD6,0xC3,0xBF,0xA8,                   //显示“配置卡数量：”
						0xCA,0xFD,0xC1,0xBF,0xA3,0xBA,0};
static void lcd_delay_ms(rt_uint32_t nms)
{
	extern void delay_ms(rt_uint32_t nms);
	delay_ms(nms);
}

/* 回调函数 */
static rt_err_t uart_lcd_rx_ind(rt_device_t dev, rt_size_t size)
{
    if(lcd_device.uart_device == dev && size > 0)
    {
		rt_sem_release(lcd_device.rx_notice);
    }
    return RT_EOK;
}

void send_char(rt_uint8_t ch)
{
	rt_device_write(lcd_device.uart_device, 0, &ch, 1);
}

static rt_err_t get_char(rt_uint8_t *ch)
{
    rt_err_t result = RT_EOK;

    while (rt_device_read(lcd_device.uart_device, 0, ch, 1) == 0)
    {
        result = rt_sem_take(lcd_device.rx_notice, RT_WAITING_FOREVER);
        if (result != RT_EOK)
        {
            return result;
        }
    }

    return RT_EOK;
}

static void lcd_uart_rx_handle_entry(void* param)
{
	rt_bool_t is_full = RT_FALSE;
	rt_uint8_t ch;
	queue_reset();
	while(1)
	{		
		get_char(&ch);
		queue_push(ch);
		qsize size = queue_find_cmd(lcd_device.cmd_buffer,lcd_device.cmd_bufsize); 		//从缓冲区中获取一条指令 
		if(size>0)
		{                   
			ProcessMessage((PCTRL_MSG)lcd_device.cmd_buffer, size);		//指令处理  
		}
	}
}

static int lcd_device_init(void)
{
	rt_err_t res;	
    rt_device_t uart_lcd_device = rt_device_find(UART_LCD_UART_NAME);
	if (uart_lcd_device != RT_NULL)
    {   
		lcd_device.uart_device = uart_lcd_device;
		
		lcd_device.lcd_event = rt_event_create("lcd_evt", RT_IPC_FLAG_FIFO);
		lcd_device.lcd_lock = rt_mutex_create("lcd_lock", RT_IPC_FLAG_FIFO);
		lcd_device.rx_notice = rt_sem_create("lcd_rx", 0, RT_IPC_FLAG_FIFO);
		lcd_device.cmd_bufsize = CMD_MAX_SIZE;
		lcd_device.cmd_buffer = rt_calloc(1, lcd_device.cmd_bufsize);
		
		RT_ASSERT(lcd_device.cmd_buffer!=RT_NULL);
		
        res = rt_device_set_rx_indicate(lcd_device.uart_device, uart_lcd_rx_ind);

        if (res != RT_EOK)   //检查返回值
        {
            rt_kprintf("set %s rx indicate error(%d)\n", UART_LCD_UART_NAME, res);
            return -RT_ERROR;
        }
		/* 打开设备，以可读写、中断方式 */
        res = rt_device_open(uart_lcd_device, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX );
		if (res != RT_EOK)
        {
            rt_kprintf("open %s device error.%d\n", UART_LCD_UART_NAME, res);
            return -RT_ERROR;
        }
		else rt_kprintf("open %s device init OK ! \n", UART_LCD_UART_NAME);
		
		lcd_delay_ms(10);
		
		/*清空串口接收缓冲区*/
		queue_reset();
		/*延时等待串口屏初始化完毕,必须等待300ms*/
		lcd_delay_ms(300);
		
		return RT_EOK;
	}
	else
	{
        rt_kprintf("can't find %s device.\n", UART_LCD_UART_NAME);
        return -RT_ERROR;
	}
}
INIT_DEVICE_EXPORT(lcd_device_init);

static int lcd_uart_rx_handle(void)
{
	rt_thread_t lcd_rev = rt_thread_create("lcd_rev",
										   lcd_uart_rx_handle_entry,
										   RT_NULL,1024,8,10);
	if(lcd_rev != RT_NULL)
		rt_thread_startup(lcd_rev);
	
	return 0;
}
INIT_APP_EXPORT(lcd_uart_rx_handle);
