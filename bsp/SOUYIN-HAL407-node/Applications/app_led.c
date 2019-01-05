#include <rtdevice.h>
#include <app_led.h>

static void led_thread_entry(void* parameter)   //线程
{
	rt_pin_mode(LED,PIN_MODE_OUTPUT);
	
	while(1)
	{
		rt_pin_write(LED,PIN_LOW);
		rt_thread_mdelay(500);                        //延时500ms，该函数更新操作系统
		rt_pin_write(LED,PIN_HIGH);
		rt_thread_mdelay(500);
	}
}

static int led_tolget(void)
{
	rt_thread_t 
	tled = rt_thread_create("tled",                       //线程名字，在shell里面可以看到
					        led_thread_entry,                //线程入口函数
					        RT_NULL,                         //线程入口函数参数
					        256,                             //线程栈大小
					        15,                              //线程的优先级
					        20);                             //线程时间片
	if(tled != RT_NULL)
		rt_thread_startup(tled);
	
    return 0;
}
INIT_APP_EXPORT(led_tolget);
