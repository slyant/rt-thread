

#include <rtthread.h>
#include <rtdevice.h>
#include <gpio_oper.h>

DOOR_REG  door[16];
uint16_t dor_oc_time = 30*5 ;
uint8_t  sw_pin[16] = {SW1,SW2,SW3,SW4,SW5,SW6,SW7,SW8,SW9,SW10,SW11,SW12,SW13,SW14,SW15,SW16};
rt_sem_t beep_sem = RT_NULL;

static void gpio_delay_us(rt_uint32_t nus)
{
	extern void delay_us(rt_uint32_t nus);
	delay_us(nus);
}

static int door_sw_init(void)
{
	uint8_t i;
	for(i=0;i<16;i++)
	{
		rt_pin_mode(sw_pin[i],PIN_MODE_INPUT);
		door[i].door_sta = (uint16_t) rt_pin_read(sw_pin[i]);     //为0处于关门状态
		door[i].open_time = 0;
	}
	return 0;
}
INIT_DEVICE_EXPORT(door_sw_init);

static void door_scan(void* parameter)
{
	uint8_t i;
	while(1)
	{
		for(i=0;i<16;i++)
		{
			if(door[i].door_sta != (uint16_t) rt_pin_read(sw_pin[i]))
			{
				door[i].door_sta ^= 1;
				if(door[i].door_sta==0)  door[i].open_time=0;
				rt_kprintf("door code: %d  status: %d \n",i,door[i].door_sta);
			}
			if(door[i].door_sta)
			{
				if(door[i].open_time<dor_oc_time)
					door[i].open_time++;
			}				
		}
		rt_thread_delay(200);
	}
}

static int door_hadle(void)
{
	rt_thread_t tas_door = rt_thread_create("tdoor", door_scan,
											RT_NULL,256,13,20);
	if(tas_door != RT_NULL)
		rt_thread_startup(tas_door);
	return 0;
}
INIT_APP_EXPORT(door_hadle);


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
	led_task = rt_thread_create("tled",                       //线程名字，在shell里面可以看到
					        led_thread_entry,                //线程入口函数
					        RT_NULL,                         //线程入口函数参数
					        256,                             //线程栈大小
					        15,                              //线程的优先级
					        20);                             //线程时间片
	if(led_task != RT_NULL)
		rt_thread_startup(led_task);
	
    return 0;
}
INIT_APP_EXPORT(led_tolget);

static int door_h595_init(void)
{
	rt_pin_mode(D595,PIN_MODE_OUTPUT);
	rt_pin_mode(EN595,PIN_MODE_OUTPUT_OD);                     //该引脚有上拉
	rt_pin_mode(OE595,PIN_MODE_OUTPUT);
	rt_pin_mode(CK595,PIN_MODE_OUTPUT);
	
	rt_pin_write(D595,PIN_LOW);
	rt_pin_write(EN595,PIN_HIGH);
	rt_pin_write(OE595,PIN_LOW);
	rt_pin_write(CK595,PIN_LOW);
	
	return 0;
}	
INIT_DEVICE_EXPORT(door_h595_init);

void write_h595(uint16_t dat)
{
	uint8_t i;
	
	rt_pin_write(OE595,PIN_LOW);           //低电平期间移位
	gpio_delay_us(5);
	for(i=0;i<16;i++)
	{
		rt_pin_write(D595,(uint8_t)(dat>>15));
		gpio_delay_us(5);
		rt_pin_write(CK595,PIN_HIGH);      //时钟上升沿移位
		dat = dat<<1;
		gpio_delay_us(5);
		rt_pin_write(CK595,PIN_LOW);       //时钟下降
		gpio_delay_us(5);
	}
	rt_pin_write(OE595,PIN_HIGH);          //上升沿锁存数据
	gpio_delay_us(5);
	rt_pin_write(EN595,PIN_LOW);           //引脚输出数据
}

static int nrf_addr_pin_init(void)
{
	rt_pin_mode(ADD_SET1,PIN_MODE_INPUT);
	rt_pin_mode(ADD_SET2,PIN_MODE_INPUT);
	rt_pin_mode(ADD_SET3,PIN_MODE_INPUT);
	rt_pin_mode(ADD_SET4,PIN_MODE_INPUT);
	rt_pin_mode(ADD_SET5,PIN_MODE_INPUT);
	rt_pin_mode(ADD_SET6,PIN_MODE_INPUT);
	return 0;
}
INIT_DEVICE_EXPORT(nrf_addr_pin_init);

uint8_t get_nrf_addr(void)
{
	uint8_t res;
	res=0;
	res |= rt_pin_read(ADD_SET1);
	res<<=1;
	res |= rt_pin_read(ADD_SET2);
	res<<=1;
	res |= rt_pin_read(ADD_SET3);
	res<<=1;
	res |= rt_pin_read(ADD_SET4);
	res<<=1;
	res |= rt_pin_read(ADD_SET5);
	res<<=1;
	res |= rt_pin_read(ADD_SET6);
	return res;
}

void beep_on(rt_uint8_t count)
{
	rt_uint8_t i;
	for(i=0; i<count; i++)
	{
		rt_sem_release(beep_sem);
	}
}

static void beep_entry(void* parameter)
{
	rt_pin_mode(BEEP,PIN_MODE_OUTPUT);
	rt_pin_write(BEEP,PIN_HIGH);
	while(1)
	{
		rt_sem_take(beep_sem,RT_WAITING_FOREVER);
		rt_pin_write(BEEP,PIN_LOW);
		rt_thread_mdelay(100);
		rt_pin_write(BEEP,PIN_HIGH);
		rt_thread_mdelay(10);
	}
}

static int beep_sound(void)
{
	rt_thread_t  beep_task;	
	beep_sem = rt_sem_create("sembeep", 1, RT_IPC_FLAG_FIFO);
	RT_ASSERT(beep_sem != RT_NULL);
	
	beep_task = rt_thread_create("tbeep",beep_entry,
								  RT_NULL,256,14,100);
	if(beep_task != RT_NULL )
		rt_thread_startup(beep_task);
	
	return 0;
}	
INIT_APP_EXPORT(beep_sound);
