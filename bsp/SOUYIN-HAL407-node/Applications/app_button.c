#include <rtthread.h> 
#include <rtdevice.h>
#include <multi_button.h>
#include <app_config.h>
#include <app_button.h>

static struct button btn0;

static uint8_t button0_read_pin(void) 
{
    return rt_pin_read(BUTTON_0_PIN); 
}

void button_callback(void *btn)
{
    uint32_t btn_event_val; 
    
    btn_event_val = get_button_event((struct button *)btn); 
    if(btn==&btn0)
	{
		switch(btn_event_val)
		{
			case PRESS_DOWN:
			{
			}
			break; 
			case LONG_RRESS_START: 
				nrf_clear_reset();
			break; 
		}
	}
}

void btn_thread_entry(void* p)
{
    while(1)
    {
        /* 5ms */
        rt_thread_mdelay(5); 
        button_ticks(); 
    }
}

int multi_button(void)
{
    rt_thread_t thread = RT_NULL;
    
    /* Create background ticks thread */
    thread = rt_thread_create("tbtn", btn_thread_entry, RT_NULL, 1024, 10, 10);
    if(thread == RT_NULL)
    {
        return RT_ERROR; 
    }
    rt_thread_startup(thread);

    /* low level drive */
    rt_pin_mode  (BUTTON_0_PIN, PIN_MODE_INPUT_PULLUP); 
	
    button_init  (&btn0, button0_read_pin, PIN_LOW);
	
    button_attach(&btn0, PRESS_DOWN,       button_callback);
    button_attach(&btn0, LONG_RRESS_START, button_callback);
	
    button_start (&btn0);

    return RT_EOK; 
}
INIT_APP_EXPORT(multi_button); 
