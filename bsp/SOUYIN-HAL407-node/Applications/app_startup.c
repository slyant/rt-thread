#include <app_config.h>
#include <stm32f4xx_hal_cortex.h>

static void nrf_cmd_handle_hook(payload_data_t payload_data)
{
    switch(payload_data->cmd)
    {
    case CMD_SESSION_KEY:
        break;
    case CMD_DOOR_OPEN:
        door_any_open(payload_data->args.door_index);
        break;
    case CMD_GROUP_OPEN:
        door_group_open();
        break;
    default:
        break;
    }
}
static void door_update_hook(rt_uint16_t sta)
{
    nrf_send_door_sta(sta);
}
void app_startup(void)
{
    nrf_set_cmd_handle_hook(nrf_cmd_handle_hook);
    door_set_update_hook(door_update_hook);
    
    extern void app_nrf_node_startup(void);	
	app_nrf_node_startup();
}
