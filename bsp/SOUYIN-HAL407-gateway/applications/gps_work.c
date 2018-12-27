#include <rtthread.h>
#include <drv_usart.h>
#include <string.h>
#include <stdlib.h>
#include "drv_pcf8563.h"
#include "hmi_driver.h"
#include <app_config.h>

#define  GPS_RX_EVENT  (1<<0)

static struct rt_event gps_evt;                   //事件控制块
static rt_device_t uart_gps_dev = RT_NULL;        //串口设备句柄

static char gps_buf[128]={0};


/* 回调函数 */
static rt_err_t uart_gps_rev(rt_device_t dev, rt_size_t size)
{
    rt_event_send(&gps_evt, GPS_RX_EVENT);       //接收事件 
    return RT_EOK;
}

static uint8_t gps_getchar(void)
{
	rt_uint32_t es;
	uint8_t revd;
	while(rt_device_read(uart_gps_dev,0,&revd,1)!=1)
	{
		rt_event_recv(&gps_evt,GPS_RX_EVENT,RT_EVENT_FLAG_AND |
                      RT_EVENT_FLAG_CLEAR,RT_WAITING_FOREVER, &es);
	}
	return revd;
}

static int gps_init(void)
{
	rt_err_t res;
	uart_gps_dev = rt_device_find(UART_GPS_UART_NAME); //查找系统中的串口设备
	if(uart_gps_dev != RT_NULL)
	{
		uart_name_set_baud_rate(UART_GPS_UART_NAME, UART_GPS_BAUD_RATE);
		res = rt_device_set_rx_indicate(uart_gps_dev, uart_gps_rev);
		if (res != RT_EOK)   //检查返回值  
		{
			rt_kprintf("set %s rx indicate error.%d\n", UART_GPS_UART_NAME, res);
			return -RT_ERROR;
		}
		/* 打开设备，以可读写、中断方式 */
        res = rt_device_open(uart_gps_dev, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX );
		 /* 初始化事件对象 */
        rt_event_init(&gps_evt, "gps_rx", RT_IPC_FLAG_FIFO);
		if(res == RT_EOK)
		{
			rt_kprintf("gps init on %s done ! \n", UART_GPS_UART_NAME);
			return -RT_EOK;
		}
	}
	return -RT_ERROR; 
}
INIT_DEVICE_EXPORT(gps_init);

static void gps_data_parse(rt_uint8_t len)
{	
	static rt_uint16_t cfg_time = 0;
	rt_uint8_t i,t,ds[3]={0};	
	t=0;
	for(i=0;i<len;i++)
	{
		if(gps_buf[i]==',')  
		{ 
			if(t==0) ds[0] = i+1;
			else if(t==1) ds[1] = i+1;
			else if(t==8) ds[2] = i+1;
			t++;
		}
	}
	if(gps_buf[ds[1]] == 'A')
	{
		if(cfg_time == 0)
		{
			cfg_time = 300;    //每5分钟检查一次时间
		}
		else
		{
			cfg_time--;	
			return;
		}	
		t = ds[0];
		sys_status.sys_datetime.hour =   (gps_buf[t]-'0')*10;   t++;
		sys_status.sys_datetime.hour +=  (gps_buf[t]-'0')+8;    t++;
		sys_status.sys_datetime.min  =   (gps_buf[t]-'0')*10;   t++;
		sys_status.sys_datetime.min  +=  (gps_buf[t]-'0');      t++;
		sys_status.sys_datetime.sec  =   (gps_buf[t]-'0')*10;   t++;
		sys_status.sys_datetime.sec  +=  (gps_buf[t]-'0');
		
		t = ds[2];
		sys_status.sys_datetime.mday   =  (gps_buf[t]-'0')*10;   t++;
		sys_status.sys_datetime.mday  +=  (gps_buf[t]-'0');      t++;
		sys_status.sys_datetime.month  =  (gps_buf[t]-'0')*10;   t++;
		sys_status.sys_datetime.month +=  (gps_buf[t]-'0');      t++;
		sys_status.sys_datetime.year   =  (gps_buf[t]-'0')*10;   t++;
		sys_status.sys_datetime.year  +=  (gps_buf[t]-'0')+2000;

		sys_status.sys_datetime.wday = ymd_to_wday(sys_status.sys_datetime.year, sys_status.sys_datetime.month, sys_status.sys_datetime.mday);
        //更新系统时钟以及串口屏时钟
		rt_kprintf("------------   updata system time ! ----------------\n");
		rtc_set_time(&(sys_status.sys_datetime));
		SetRtc(sys_status.sys_datetime.year, sys_status.sys_datetime.month, sys_status.sys_datetime.mday, sys_status.sys_datetime.wday,
		sys_status.sys_datetime.hour, sys_status.sys_datetime.min, sys_status.sys_datetime.sec);		
	}
}

static void uart_handle_entry(void* param)
{
	uint8_t gps_rx;
	static uint8_t gps_index;
	
	gps_index=0; 
	while(1)
	{
		gps_rx = gps_getchar();
		if(gps_rx == '$') { gps_buf[0] = gps_rx; gps_index = 1; }
		else if(gps_index)
		{
			gps_buf[gps_index] = gps_rx;
			if(gps_rx == 0xA && gps_buf[gps_index-1] == 0xD)
			{
				gps_buf[gps_index+1]=0; 
				gps_data_parse(gps_index); 
				gps_index=0; 
			}
			else gps_index++; 
		}
	}
}

static int gps_handle(void)
{
	rt_thread_t gps_thread = rt_thread_create("tgps",uart_handle_entry,
											RT_NULL,512,12,50);
	if(gps_thread!=RT_NULL)
		rt_thread_startup(gps_thread);
	return 0;
}
INIT_APP_EXPORT(gps_handle);
