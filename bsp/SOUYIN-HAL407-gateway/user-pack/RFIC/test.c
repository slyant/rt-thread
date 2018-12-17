#include <rtthread.h>
static void rfid_thread_entry(void *param)
{	
	while(1)
	{	
		rt_thread_mdelay(50);
	}
}

static int rfid_handle(void)
{
	rt_thread_t rftid = RT_NULL;
	rftid = rt_thread_create("rfic_tid",
							 rfid_thread_entry,
							 RT_NULL,20*1024,10,20);
	if(rftid != RT_NULL ) 
		rt_thread_startup(rftid);
	return 0;
}
INIT_APP_EXPORT(rfid_handle);
