#include <app_config.h>
#include <rtdevice.h>

void app_startup(void)
{    
    extern void app_nrf_node_startup(void);	
	app_nrf_node_startup();
    extern int app_door_startup(void);
    app_door_startup();
}
