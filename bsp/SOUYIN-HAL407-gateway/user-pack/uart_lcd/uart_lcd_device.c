#include <rtthread.h>
#include <cmd_queue.h>
#include <uart_lcd_device.h>
#include <uart_lcd_process.h>

#define UART_LCD_DEVICE_DEBUG

#define DBG_ENABLE
#define DBG_COLOR
#define DBG_SECTION_NAME		"uart.lcd.device"
#ifdef UART_LCD_DEVICE_DEBUG
#define DBG_LEVEL				DBG_LOG
#else
#define DBG_LEVEL				DBG_INFO
#endif
#include <rtdbg.h>

struct lcd_device_s lcd_device;

void send_char(rt_uint8_t ch)	//for hmi_driver.c
{
	rt_device_write(lcd_device.uart_device, 0, &ch, 1);
}

static rt_err_t uart_lcd_rx_ind(rt_device_t dev, rt_size_t size)
{
    if(lcd_device.uart_device == dev && size > 0)
    {
		rt_sem_release(lcd_device.rx_notice);
    }
    return RT_EOK;
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
static void lcd_updata_entry(void* param)
{
	lcd_load_default_screen();
	while(1)
	{    
		rt_uint32_t e;
		if(rt_event_recv(lcd_device.lcd_event,LCD_UPDATE_EVENT,RT_EVENT_FLAG_CLEAR,RT_WAITING_FOREVER, &e) == RT_EOK)
		{
			if(e & LCD_UPDATE_EVENT)
			{
				lcd_update_ui();
			}
		}
		rt_thread_mdelay(100);//100ms允许更新屏幕
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
		RT_ASSERT(lcd_device.lcd_event!=RT_NULL);
				
		lcd_device.rx_notice = rt_sem_create("lcd_rx", 0, RT_IPC_FLAG_FIFO);
		RT_ASSERT(lcd_device.rx_notice!=RT_NULL);
		
		lcd_device.cmd_bufsize = CMD_MAX_SIZE;
		lcd_device.cmd_buffer = rt_calloc(1, lcd_device.cmd_bufsize);		
		RT_ASSERT(lcd_device.cmd_buffer!=RT_NULL);
		
        res = rt_device_set_rx_indicate(lcd_device.uart_device, uart_lcd_rx_ind);

        if (res != RT_EOK)   //检查返回值
        {
            LOG_E("set %s rx indicate error(%d)\n", UART_LCD_UART_NAME, res);
            return -RT_ERROR;
        }
		/* 打开设备，以可读写、中断方式 */
        res = rt_device_open(uart_lcd_device, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX );
		if (res != RT_EOK)
        {
            LOG_E("open %s device error.%d\n", UART_LCD_UART_NAME, res);
            return -RT_ERROR;
        }
		/*清空串口接收缓冲区*/
		queue_reset();
		/*延时等待串口屏初始化完毕,必须等待300ms*/
		rt_thread_mdelay(300);
		lcd_device_reg(&lcd_device);
		LOG_I("lcd device on %s init OK! \n", UART_LCD_UART_NAME);
		return RT_EOK;
	}
	else
	{
        LOG_E("can't find %s device.\n", UART_LCD_UART_NAME);
        return -RT_ERROR;
	}
}

int lcd_device_startup(void)
{
	if(lcd_device_init()==RT_EOK)
	{	
		rt_thread_t lcd_rev = rt_thread_create("lcd_rev",
											   lcd_uart_rx_handle_entry,
											   RT_NULL,1024,8,10);
		if(lcd_rev != RT_NULL)
			rt_thread_startup(lcd_rev);
		
		rt_thread_t lcd_upd = rt_thread_create("lcd_upd",
											   lcd_updata_entry,
											   RT_NULL,512,7,10);
		if(lcd_upd != RT_NULL)
			rt_thread_startup(lcd_upd);	
		
		return 0;
	}
	else
	{
		return -RT_ERROR;
	}
}
