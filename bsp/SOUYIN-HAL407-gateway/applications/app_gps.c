#include <rtthread.h>
#include <drv_usart.h>
#include <app_gps.h>

#define BUF_LEN		128

static rt_sem_t gps_rx_sem = RT_NULL;
static rt_device_t uart_gps_dev = RT_NULL;        //串口设备句柄
static char gps_buf[BUF_LEN]={0};
static gps_update_hook_t gps_update_hook = RT_NULL;

void gps_update_set_hook(gps_update_hook_t hook)
{
	gps_update_hook = hook;
}

/* 回调函数 */
static rt_err_t uart_gps_rev(rt_device_t dev, rt_size_t size)
{
    if(dev == uart_gps_dev && size > 0)
    {
        rt_sem_release(gps_rx_sem);
    }
    return RT_EOK;
}

static rt_err_t gps_getchar(rt_uint8_t *ch)
{
	rt_err_t result = RT_EOK;
	while(rt_device_read(uart_gps_dev, 0, ch, 1) == 0)
	{
		result = rt_sem_take(gps_rx_sem, RT_WAITING_FOREVER);
        if(result != RT_EOK)
            break;
	}
	return result;
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
        /* 创建串口接收中断信号量 */
        gps_rx_sem = rt_sem_create("gps_sem", 0, RT_IPC_FLAG_FIFO);
        RT_ASSERT(gps_rx_sem != RT_NULL);
		/* 打开设备，以可读写、中断方式 */
        res = rt_device_open(uart_gps_dev, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX );
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
        struct calendar cal;
		t = ds[0];
		cal.hour =   (gps_buf[t]-'0')*10;   t++;
		cal.hour +=  (gps_buf[t]-'0');      t++;
		cal.min  =   (gps_buf[t]-'0')*10;   t++;
		cal.min  +=  (gps_buf[t]-'0');      t++;
		cal.sec  =   (gps_buf[t]-'0')*10;   t++;
		cal.sec  +=  (gps_buf[t]-'0');
		
		t = ds[2];
		cal.mday   =  (gps_buf[t]-'0')*10;   t++;
		cal.mday  +=  (gps_buf[t]-'0');      t++;
		cal.month  =  (gps_buf[t]-'0')*10;   t++;
		cal.month +=  (gps_buf[t]-'0');      t++;
		cal.year   =  (gps_buf[t]-'0')*10;   t++;
		cal.year  +=  (gps_buf[t]-'0')+2000;
		
        rt_uint32_t seccount = rtc_make_time(&cal);
        seccount += (8 * 60 * 60);  //+8时区
        rtc_local_time(seccount, &cal);
        
		if(gps_update_hook)
			gps_update_hook(&cal);
	}
}

static void uart_handle_entry(void* param)
{
	rt_uint8_t gps_rx;
	static rt_uint8_t gps_index = 0;
	while(1)
	{
		if(gps_getchar(&gps_rx) != RT_EOK)
            continue;
		if(gps_rx == '$') 
		{ 
			gps_buf[0] = gps_rx; 
			gps_index = 1;
		}
		else if(gps_index > 0 && gps_index < BUF_LEN - 1)
		{
			gps_buf[gps_index] = gps_rx;
			if(gps_rx == 0xA && gps_buf[gps_index-1] == 0xD)
			{
				gps_buf[gps_index + 1] = 0; 
				gps_data_parse(gps_index); 
				gps_index = 0; 
			}
			else gps_index++; 
		}
		else
		{
			gps_index = 0;
		}
	}
}

static int gps_handle(void)
{
	rt_thread_t gps_thread = rt_thread_create("tgps",uart_handle_entry,
											RT_NULL,1024,16,50);
	if(gps_thread!=RT_NULL)
		rt_thread_startup(gps_thread);
	return 0;
}
INIT_APP_EXPORT(gps_handle);
