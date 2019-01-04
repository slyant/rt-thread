#include <rtthread.h>
#include <rtdevice.h>
#include <app_config.h>

static rt_uint8_t  sw_pin[16] = {SW1,SW2,SW3,SW4,SW5,SW6,SW7,SW8,SW9,SW10,SW11,SW12,SW13,SW14,SW15,SW16};
static rt_uint16_t door_sta;
static rt_mutex_t sta_lock = RT_NULL;

static void set_sta(rt_uint8_t door_index, rt_uint8_t sta)
{
    rt_uint16_t tag = 0x0001;
    rt_mutex_take(sta_lock, RT_WAITING_FOREVER);
    if(sta)
    {
        door_sta |= (tag << door_index);
    }
    else
    {
        door_sta &= ~(tag << door_index);
    }
    rt_mutex_release(sta_lock);
}

static rt_uint8_t get_sta(rt_uint8_t door_index)
{
    rt_uint8_t result;
    rt_uint16_t tag = 0x0001;
    rt_mutex_take(sta_lock, RT_WAITING_FOREVER);
    result = (door_sta & (tag << door_index)) > 0 ? 1:0;
    rt_mutex_release(sta_lock);
    return result;
}

static rt_uint16_t get_door_sta(void)
{
    rt_uint16_t result;
    rt_mutex_take(sta_lock, RT_WAITING_FOREVER);
    result = door_sta;
    rt_mutex_release(sta_lock);
    return result;
}
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

static void door_scan_thread(void* parameter)
{
	rt_uint8_t i;

	while(1)
	{
		for(i=0;i<16;i++)
		{
            rt_uint8_t pin = (rt_uint8_t)rt_pin_read(sw_pin[i]);
            if(pin != get_sta(i))
            {
                set_sta(i, pin);
                write_h595(get_door_sta());
            }
		}
		rt_thread_delay(200);
	}
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

static int app_door_startup(void)
{
    door_sta = 0;
    sta_lock = rt_mutex_create("sta_lock", RT_IPC_FLAG_FIFO);
    RT_ASSERT(sta_lock != RT_NULL);
    
	rt_thread_t thread = rt_thread_create("tdoor", door_scan_thread,
											RT_NULL,256,13,20);
	if(thread != RT_NULL)
		rt_thread_startup(thread);
	return 0;
}
INIT_APP_EXPORT(app_door_startup);
