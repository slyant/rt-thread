#include <rtthread.h>
#include <board.h>
#include <rtdevice.h>

#define UART_DEVICE_NAME	"uart1"

#define PRIORITY 10        
#define STACK_SIZE 20480
#define TIMESLICE 10 

static rt_timer_t rxdelay_timer;

#define RX_DATA_MAX_LEN	256
static rt_uint8_t uart_rx_data[RX_DATA_MAX_LEN];
static rt_uint16_t data_index = 0;

/* 串口接收事件标志 */
#define UART_RX_EVENT (1 << 0)
/* 事件控制块 */
static struct rt_event event;
/* 设备句柄 */
static rt_device_t uart_device = RT_NULL;

/* 回调函数 */
static rt_err_t uart_intput(rt_device_t dev, rt_size_t size)
{
    /* 发送事件 */
    rt_event_send(&event, UART_RX_EVENT);
    return RT_EOK;
}

static void uart_putchar(const rt_uint8_t c)
{
    rt_size_t len = 0;
    rt_uint32_t timeout = 0;
    do
    {
        len = rt_device_write(uart_device, 0, &c, 1);
        timeout++;
    }
    while (len != 1 && timeout < 500);
}

static void uart_putstring(char *string)
{
	char* c = string;
	while(*c!=0x00)
	{
		uart_putchar(*c++);
	}
}

static rt_uint8_t uart_getchar(void)
{
    rt_uint32_t e;
    rt_uint8_t ch;
    /* 读取1字节数据 */
    while (rt_device_read(uart_device, 0, &ch, 1) != 1)
	{
         /* 接收事件 */
        rt_event_recv(&event, UART_RX_EVENT,RT_EVENT_FLAG_AND | 
                      RT_EVENT_FLAG_CLEAR,RT_WAITING_FOREVER, &e);
	}
    return ch;
}

static rt_err_t uart_open(const char *name, rt_uint32_t baud_rate, rt_uint32_t bufsz)
{
    rt_err_t res;
    /* 查找系统中的串口设备 */
    uart_device = rt_device_find(name);   
    /* 查找到设备后将其打开 */
    if (uart_device != RT_NULL)
    {   
        res = rt_device_set_rx_indicate(uart_device, uart_intput);
        /* 检查返回值 */
        if (res != RT_EOK)
        {
            rt_kprintf("set %s rx indicate error.%d\n", name, res);
            return -RT_ERROR;
        }
        /* 打开设备，以可读写、中断方式 */
        res = rt_device_open(uart_device, RT_DEVICE_OFLAG_RDWR | 
                             RT_DEVICE_FLAG_INT_RX );       
        /* 检查返回值 */
        if (res != RT_EOK)
        {
            rt_kprintf("open %s device error.%d\n", name, res);
            return -RT_ERROR;
        }		
		
		/* 修改串口配置参数 */
		struct serial_configure config = 
		{
			baud_rate,   	  /* bits/s */
			DATA_BITS_8,      /* 8 databits */
			STOP_BITS_1,      /* 1 stopbit */
			PARITY_NONE,      /* No parity  */ 
			BIT_ORDER_LSB,    /* LSB first sent */
			NRZ_NORMAL,       /* Normal mode */
			bufsz,            /* Buffer size */
			0   
		};
		res= rt_device_control(uart_device, RT_DEVICE_CTRL_CONFIG, (void*)&config);

		/* 检查返回值 */
        if (res != RT_EOK)
        {
            rt_kprintf("control %s device error.%d\n", name, res);
            return -RT_ERROR;
        }
    }
    else
    {
        rt_kprintf("can't find %s device.\n", name);
        return -RT_ERROR;
    }
    /* 初始化事件对象 */
    rt_event_init(&event, "qr_event", RT_IPC_FLAG_FIFO); 
    return RT_EOK;
}

static void rx_delay_timeout(void *parameter)
{
	rt_kprintf("%s\n", uart_rx_data);
	rt_memset(uart_rx_data, 0, sizeof(uart_rx_data));
	data_index = 0;
}

static void thread_entry(void *parameter)
{
	rxdelay_timer = rt_timer_create("qr_rxdelay", rx_delay_timeout, 0, 50, RT_TIMER_FLAG_ONE_SHOT|RT_TIMER_FLAG_SOFT_TIMER);
    /* 打开串口 */
    if (uart_open(UART_DEVICE_NAME, BAUD_RATE_9600, 64) != RT_EOK)
    {
        rt_kprintf("%s open error.\n", UART_DEVICE_NAME);
         while (1)
         {
            rt_thread_delay(10);
         }
    }
    /* 写字符串 */
//    uart_putstring("Hello RT-Thread!\r\n");
	
    while (1)
    {   
		if(data_index<RX_DATA_MAX_LEN-1)
		{
			uart_rx_data[data_index++] = uart_getchar();
			rt_timer_start(rxdelay_timer);
		}
    }
}

static int app_uart_thread_start(void)
{
	rt_thread_t rtt = RT_NULL;	

 	rtt =rt_thread_create("qr_scan",         //线程名称。
							thread_entry,    //线程入口函数。
							RT_NULL,         //线程入口参数。
							STACK_SIZE,      //线程栈大小。
							PRIORITY,        //线程优先级。
							TIMESLICE);      //时间片Tick。
	if(rtt != RT_NULL)                       //判断线程是否创建成功。
	{
		rt_thread_startup(rtt);              //线程创建成功，启动线程。
	}
	return RT_EOK; 
}
INIT_APP_EXPORT(app_uart_thread_start); 
