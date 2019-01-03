#include <rtthread.h>
#include <rtdevice.h>
#include <app_config.h>

rt_uint8_t  sw_pin[16] = {SW1,SW2,SW3,SW4,SW5,SW6,SW7,SW8,SW9,SW10,SW11,SW12,SW13,SW14,SW15,SW16};

struct rt_event timing_out_event;

static void door_scan(void* parameter)
{
	rt_uint8_t i;

	while(1)
	{
		for(i=0;i<16;i++)
		{
            rt_uint8_t pin = (rt_uint8_t)rt_pin_read(sw_pin[i]);
		}
		rt_thread_delay(200);
	}
}

static int door_hadle(void)
{
	rt_event_init(&timing_out_event, "event", RT_IPC_FLAG_FIFO);

	rt_thread_t tas_door = rt_thread_create("tdoor", door_scan,
											RT_NULL,256,13,20);
	if(tas_door != RT_NULL)
		rt_thread_startup(tas_door);
	return 0;
}
INIT_APP_EXPORT(door_hadle);

static void gpio_delay_us(rt_uint32_t nus)
{
	extern void delay_us(rt_uint32_t nus);
	delay_us(nus);
}
void write_h595(rt_uint16_t dat)
{
	rt_uint8_t i;	
	rt_pin_write(OE595,PIN_LOW);           //低电平期间移位
	gpio_delay_us(5);
	for(i=0;i<16;i++)
	{
		rt_pin_write(D595,(rt_uint8_t)(dat>>15));
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

static int door_init(void)
{
	rt_uint8_t i;

	rt_pin_mode(D595,PIN_MODE_OUTPUT);
	rt_pin_mode(EN595,PIN_MODE_OUTPUT);
	rt_pin_mode(OE595,PIN_MODE_OUTPUT);
	rt_pin_mode(CK595,PIN_MODE_OUTPUT);
	
	rt_pin_write(D595,PIN_LOW);
	rt_pin_write(EN595,PIN_HIGH);
	rt_pin_write(OE595,PIN_LOW);
	rt_pin_write(CK595,PIN_LOW);
    
	for(i=0;i<16;i++)
	{
		rt_pin_mode(sw_pin[i],PIN_MODE_INPUT);
	}
	return 0;
}
INIT_DEVICE_EXPORT(door_init);
