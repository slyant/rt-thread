#include <rtdevice.h>
#include <app_beep.h>

rt_sem_t beep_sem = RT_NULL;

void beep_on(rt_uint8_t count)
{
    if(beep_sem)
    {
        for(rt_uint8_t i=0; i<count; i++)
        {
            rt_sem_release(beep_sem);
        }
    }
}

static void beep_thread_entry(void* parameter)
{
	rt_pin_mode(BEEP, PIN_MODE_OUTPUT);
	rt_pin_write(BEEP, PIN_HIGH);
	while(1)
	{
		rt_sem_take(beep_sem, RT_WAITING_FOREVER);
		rt_pin_write(BEEP, PIN_LOW);
		rt_thread_mdelay(100);
		rt_pin_write(BEEP, PIN_HIGH);
		rt_thread_mdelay(10);
	}
}

static int beep_thread_startup(void)
{	
	beep_sem = rt_sem_create("sembeep", 1, RT_IPC_FLAG_FIFO);
	RT_ASSERT(beep_sem != RT_NULL);
	
	rt_thread_t tbeep = rt_thread_create("tbeep", beep_thread_entry, RT_NULL,
                                        256, 14, 100);
	if(tbeep != RT_NULL )
		rt_thread_startup(tbeep);
	
	return 0;
}	
INIT_APP_EXPORT(beep_thread_startup);
