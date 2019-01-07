#include <rtthread.h>
#include <rtdevice.h>
#include <app_config.h>

static rt_uint8_t  sw_pin[16] = {SW1,SW2,SW3,SW4,SW5,SW6,SW7,SW8,SW9,SW10,SW11,SW12,SW13,SW14,SW15,SW16};
static rt_uint16_t door_sta;    //关门0，开门1
static rt_uint8_t update_en = 0;

//设置本地单个柜门状态
static void set_local_door_sta(rt_uint8_t door_index, rt_uint8_t sta)
{
    rt_uint16_t tag = 0x0001;
    if(sta)
    {
        door_sta |= (tag << door_index);
    }
    else
    {
        door_sta &= ~(tag << door_index);
    }
}
//获取本地单个柜门状态
static rt_uint8_t get_local_door_sta(rt_uint8_t door_index)
{
    rt_uint8_t result;
    rt_uint16_t tag = 0x0001;
    result = (door_sta & (tag << door_index)) > 0 ? 1:0;
    return result;
}

static void gpio_delay_us(rt_uint32_t nus)
{
	extern void delay_us(rt_uint32_t nus);
	delay_us(nus);
}
static void d595_write(rt_uint16_t dat)
{
	rt_pin_write(OE595, PIN_LOW);           //低电平期间移位
	gpio_delay_us(5);
	for(int i = 0; i < 16; i++)
	{
		rt_pin_write(D595, (rt_uint8_t)(dat>>15));
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

static void local_door_open(rt_uint8_t door_index)
{    
    rt_uint16_t dat = 0x0001;
    dat <<= door_index; 
    d595_write(~dat);
    rt_thread_mdelay(30);
    d595_write(0xFFFF);
    
}

void door_any_open(rt_uint8_t door_index)
{
    local_door_open(door_index);
    update_en = 1;
    rt_kprintf("door %d open\n", door_index);
}

void door_group_open(void)
{ 
    for(int i = 0; i < DOOR_MAX_COUNT; i++)
    {
        local_door_open(i);
        rt_thread_mdelay(10);
    }
    rt_kprintf("door group open\n");
}

rt_uint16_t door_get_group_sta(void)
{
    return door_sta;
}

rt_uint8_t door_get_group_addr(void)
{
	rt_uint8_t res;
	res=0;
//	res |= rt_pin_read(ADD_SET1);
//	res<<=1;
//	res |= rt_pin_read(ADD_SET2);
//	res<<=1;
//	res |= rt_pin_read(ADD_SET3);
//	res<<=1;
	res |= rt_pin_read(ADD_SET4);
	res<<=1;
	res |= rt_pin_read(ADD_SET5);
	res<<=1;
	res |= rt_pin_read(ADD_SET6);
	return res;
}

static int door_init(void)
{
	rt_pin_mode(D595, PIN_MODE_OUTPUT);
	rt_pin_mode(EN595, PIN_MODE_OUTPUT);
	rt_pin_mode(OE595, PIN_MODE_OUTPUT);
	rt_pin_mode(CK595, PIN_MODE_OUTPUT);	
	d595_write(0xFFFF);
    
	rt_pin_mode(ADD_SET1, PIN_MODE_INPUT);
	rt_pin_mode(ADD_SET2, PIN_MODE_INPUT);
	rt_pin_mode(ADD_SET3, PIN_MODE_INPUT);
	rt_pin_mode(ADD_SET4, PIN_MODE_INPUT);
	rt_pin_mode(ADD_SET5, PIN_MODE_INPUT);
	rt_pin_mode(ADD_SET6, PIN_MODE_INPUT);   
    
	for(int i = 0; i < 16; i++)
	{
		rt_pin_mode(sw_pin[i], PIN_MODE_INPUT);
	}
	return 0;
}
INIT_DEVICE_EXPORT(door_init);

static void door_scan_thread(void *parameter)
{
	rt_uint8_t i, update_tag;
	while(1)
	{
        update_tag = 0;
		for(i = 0; i < 16; i++)
		{
            rt_uint8_t pin = (rt_uint8_t)rt_pin_read(sw_pin[i]);
            if(pin != get_local_door_sta(i))
            {
                set_local_door_sta(i, pin);                
                update_tag = 1;
            }
		}
        if(update_en)
        {           
            update_tag = 1;
            update_en = 0;
        }
        if(update_tag)
        {            
            nrf_send_door_sta(door_sta);
        }
		rt_thread_mdelay(100);
	}
}
int app_door_startup(void)
{       
	rt_thread_t thread = rt_thread_create("tdoor", door_scan_thread,
											RT_NULL,1024,13,20);
	if(thread != RT_NULL)
		rt_thread_startup(thread);
	return 0;
}
