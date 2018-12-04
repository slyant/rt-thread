

#include <rtthread.h>
#include <drv_usart.h>
#include <string.h>
#include <stdlib.h>

#include "rfic_oper.h"
#include "pcf8563.h"
#include "hmi_driver.h"

#define  GPS_RX_EVENT  (1<<0)

static struct rt_event gps_evt;                   //事件控制块
static rt_device_t uart_gps_dev = RT_NULL;        //串口设备句柄

char gps_buf[128]={0};


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
	uart_gps_dev = rt_device_find("uart1"); //查找系统中的串口设备
	if(uart_gps_dev != RT_NULL)
	{
		res = rt_device_set_rx_indicate(uart_gps_dev, uart_gps_rev);
		if (res != RT_EOK)   //检查返回值  
		{
			rt_kprintf("set uart1 rx indicate error.%d\n",res);
			return -RT_ERROR;
		}
		/* 打开设备，以可读写、中断方式 */
        res = rt_device_open(uart_gps_dev, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX );
		 /* 初始化事件对象 */
        rt_event_init(&gps_evt, "gps-evt", RT_IPC_FLAG_FIFO);
		if(res == RT_EOK)
		{
			rt_kprintf("gps init done ! \n");
			return -RT_EOK;
		}
	}
	return -RT_ERROR; 
}
INIT_DEVICE_EXPORT(gps_init);

extern Calendar_Def sys_datetime;
static void gps_data_parse(uint8_t len)
{
	static uint16_t cfg_time=300,upday=0;
	
	if(cfg_time==0) cfg_time=300;    //每5分钟检查一次时间
	else return;
	
	uint8_t i,t,ds[3]={0},sec;
	
	rtc_get_time(&sys_datetime);     //获取系统时钟
	sec = sys_datetime.sec;
	
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
		t = ds[0];
		sys_datetime.hour =   (gps_buf[t]-'0')*10;   t++;
		sys_datetime.hour +=  (gps_buf[t]-'0')+8;    t++;
		sys_datetime.min  =   (gps_buf[t]-'0')*10;   t++;
		sys_datetime.min  +=  (gps_buf[t]-'0');      t++;
		sys_datetime.sec  =   (gps_buf[t]-'0')*10;   t++;
		sys_datetime.sec  +=  (gps_buf[t]-'0');
		
		t = ds[2];
		sys_datetime.mday   =  (gps_buf[t]-'0')*10;   t++;
		sys_datetime.mday  +=  (gps_buf[t]-'0');      t++;
		sys_datetime.month  =  (gps_buf[t]-'0')*10;   t++;
		sys_datetime.month +=  (gps_buf[t]-'0');      t++;
		sys_datetime.year   =  (gps_buf[t]-'0')*10;   t++;
		sys_datetime.year  +=  (gps_buf[t]-'0')+2000;
		
		if(upday==0) upday = sys_datetime.mday;
		else if(upday != sys_datetime.mday || abs(sys_datetime.sec-sec)>2)     //如果日期不对，或者与系统时钟的时间相差大于2s
		{                                                                      //更新系统时钟以及串口屏时钟
			rt_kprintf("------------   updata system time ! ----------------\n");
			rtc_set_time(&sys_datetime);
			SetRtc(sys_datetime.year,sys_datetime.month,sys_datetime.mday,sys_datetime.wday,sys_datetime.hour,sys_datetime.min,sys_datetime.sec);
		}
		cfg_time--;
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
	rt_thread_t gps_dat = rt_thread_create("gps-tag",uart_handle_entry,
											RT_NULL,1024,12,50);
	if(gps_dat!=RT_NULL)
		rt_thread_startup(gps_dat);
	return 0;
}
INIT_APP_EXPORT(gps_handle);

