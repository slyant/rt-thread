
#include <rthw.h>
#include <rtthread.h>

#include "app_uart.h"
#include "cmd_queue.h"
#include "hmi_driver.h"
#include "lcd_oper.h"



#define UART_RX_EVENT (1<<0)                  //串口接收事件标志

static struct rt_event event;                   //事件控制块
static rt_device_t uart_lcd_device = RT_NULL;   //串口设备句柄


unsigned char   cmd_buffer[CMD_MAX_SIZE];
unsigned char lcd_update_en = 0;



static uint8_t card_temp_num=0;

//rt_mq_t lcd_mq = RT_NULL;      //LCD消息队列控制块

uint8_t mcardwarning[] = {0xC5,0xE4,0xD6,0xC3,0xBF,0xA8,0xB2,0xBB,     //“配置卡不足，请配置”
					       0xD7,0xE3,0xA3,0xAC,0xC7,0xEB,0xC5,0xE4,0xD6,0xC3,0};
uint8_t mcardnum[] = {0xC5,0xE4,0xD6,0xC3,0xBF,0xA8,                   //显示“配置卡数量：”
						0xCA,0xFD,0xC1,0xBF,0xA3,0xBA,0};


static void setlcd(unsigned short screen_id)
{
	SetScreen(screen_id);
}
FINSH_FUNCTION_EXPORT(setlcd, screen id);


extern void SendNU8(unsigned char *pData,unsigned short nDataLen);


void SendChar(unsigned char c)                   //提供给串口屏使用
{
	uint8_t len=0;
	uint16_t timeout=0;
	do
	{
		len = rt_device_write(uart_lcd_device,0,&c,1);
	}
	while(len!=1&&timeout>500);
}

//static void lcd_delay_us(rt_uint32_t nus)
//{
//	extern void delay_us(rt_uint32_t nus);
//	delay_us(nus);
//}

static void lcd_delay_ms(rt_uint32_t nms)
{
	extern void delay_ms(rt_uint32_t nms);
	delay_ms(nms);
}

/* 回调函数 */
static rt_err_t uart_lcd_intput(rt_device_t dev, rt_size_t size)
{
    rt_event_send(&event, UART_RX_EVENT);       //接收事件 
    return RT_EOK;
}

uint8_t uart_rev_byte(void)
{
	rt_uint32_t e;
	uint8_t  ch=0;
	while(rt_device_read(uart_lcd_device,0,&ch,1)!=1)
	{
		rt_event_recv(&event,UART_RX_EVENT,RT_EVENT_FLAG_AND |
                      RT_EVENT_FLAG_CLEAR,RT_WAITING_FOREVER, &e);
	}
	return ch;
}

/***********************************************************************************************************/
extern uint8_t sys_key_a[6];
extern uint8_t sys_key_b[6];    //系统密钥对，由密钥函数生成，并存入加密文件系统，这里暂时定义便于调试
extern volatile uint8_t UI;
extern const uint8_t  key_card_str[];
extern const uint8_t  cof_card_str[];
/***********************************************************************************************************/
static int lcd_init(void)
{
	rt_err_t res;
	
    uart_lcd_device = rt_device_find("uart2"); //查找系统中的串口设备
	sark_msg_init();
	if (uart_lcd_device != RT_NULL)
    {   
        res = rt_device_set_rx_indicate(uart_lcd_device, uart_lcd_intput);

        if (res != RT_EOK)   //检查返回值
        {
            rt_kprintf("set uart2 rx indicate error.%d\n",res);
            return -RT_ERROR;
        }
		/* 打开设备，以可读写、中断方式 */
        res = rt_device_open(uart_lcd_device, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX );
		if (res != RT_EOK)
        {
            rt_kprintf("open uart2 device error.%d\n",res);
            return -RT_ERROR;
        }
		else rt_kprintf("open uart device init OK ! \n");
		 /* 初始化事件对象 */
        rt_event_init(&event, "event", RT_IPC_FLAG_FIFO); 
		
		lcd_delay_ms(10);
		
		/*清空串口接收缓冲区*/
		queue_reset();
		/*延时等待串口屏初始化完毕,必须等待300ms*/
		lcd_delay_ms(300);
		
		SetFcolor(0xFFE0);//前景色设置成黄色
		SetBcolor(0x52AA);

		if(sys_key_a[0]==RT_NULL && sys_key_b[0]==RT_NULL)    //没有做密钥设置
		{
			SetScreen(CARD_MANAG);    //进入密钥卡管理
			show_string(CARD_MANAG,258,62,1,9,(uint8_t*)key_card_str);
			UI = SYS_KEY_CRCF;
		}
		else if(card_temp_num<2)
		{
			SetScreen(CARD_MANAG);      //如果配置卡小于2则直接显示配置卡假面
			show_string(CARD_MANAG,258,62,1,9,(uint8_t*)cof_card_str);
			UI = MANA_CARD_SET;
			show_string(MANA_CARD_SET,20,440,0,6,mcardwarning);
			show_string(MANA_CARD_SET,20,410,0,6,mcardnum);
			show_string(MANA_CARD_SET,160,410,0,6,(uint8_t*)"0");
		}
		else SetScreen(MAIN_INDEX);
		
		return 0;
	}
	else
	{
        rt_kprintf("can't find uart2 device.\n");
        return -RT_ERROR;
	}
}
INIT_DEVICE_EXPORT(lcd_init);

static void uart_handle_entry(void* param)
{
	rt_uint32_t e,i;
	static uint16_t rxsize=0;
	qsize  size = 0;
	while(1)
	{
		while(rxsize==0)               //读串口数据。阻塞状态
		{
			rxsize = rt_device_read(uart_lcd_device,0,cmd_buffer,CMD_MAX_SIZE);
			rt_event_recv(&event, UART_RX_EVENT,RT_EVENT_FLAG_AND |
						  RT_EVENT_FLAG_CLEAR,RT_WAITING_FOREVER, &e);      
		}
/********************************************************************************************/
/****   该模块内代码拼接收到的指令，因上面while段代码在收到一个字节后跳出，并未完整接收  ****/
/********************************************************************************************/
		if(rxsize>1)
		{
			for(i=rxsize;i>0;i--)
			{
				cmd_buffer[i] = cmd_buffer[i-1];
			}
			cmd_buffer[0] = 0xEE;
			for(i=0;i<rxsize+1;i++)
			{
				queue_push(cmd_buffer[i]);
			}
		}
		rxsize = 0;
		size = queue_find_cmd(cmd_buffer,CMD_MAX_SIZE);                          //从缓冲区中获取一条指令 
		if(size>0)                                                               //接收到指令 ，及判断是否为开机提示
		{                   
			ProcessMessage((PCTRL_MSG)cmd_buffer, size);                         //指令处理  
		} 																	  
		rt_thread_delay(5);
	}
}

static int uart_handle(void)
{
	rt_thread_t uart_rev = rt_thread_create("uartrev",
										   uart_handle_entry,
										   RT_NULL,1024,8,10);
	if(uart_rev != RT_NULL)
		rt_thread_startup(uart_rev);
	
	return 0;
}
INIT_APP_EXPORT(uart_handle);








