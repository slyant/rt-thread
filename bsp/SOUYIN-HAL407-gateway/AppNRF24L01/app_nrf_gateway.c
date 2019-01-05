#include <rtthread.h>
#include <rtdevice.h>
#include <nrf_gateway.h>
#include <app_config.h>

#define APP_USING_LOG
#ifdef APP_USING_LOG
	#define LOG		rt_kprintf("[app.nrf.gateway]:");rt_kprintf
	#define LOGS	rt_kprintf
#else
	#define LOG(...)
	#define LOGS(...)
#endif

#define CONVERT_TO_NET_ADDR(d)     (ASSIGN_ADDR_START+d)       //将柜门地址转换为网络地址
#define CONVERT_TO_DOOR_INDEX(d)   (d-ASSIGN_ADDR_START)       //将网络地址转换为柜门地址

static nrf_door_update_hook_t nrf_door_update_hook = RT_NULL;

static void nrf_send_session_key(rt_uint8_t node)
{
    
}

static void on_conn_cb(void* args)
{
	uint8_t node = *(uint8_t*)args;
	LOG("NRF Node %d Connected!\r\n", node);
}
static void on_disconn_cb(void* args)
{
	uint8_t node = *(uint8_t*)args;
	LOG("NRF Node %d DisConnected!\r\n", node);
}
static void on_send_cb(rt_uint8_t src_addr,rt_uint8_t dst_addr,rt_uint8_t pack_type,rt_uint16_t ID,rt_uint8_t *buf,rt_uint8_t status)
{
	LOG("on_send_cb:id=%d src=%d dst=%d type=%d data[0]=%d data[1]=%d ",ID,src_addr,dst_addr,pack_type,buf[0],buf[1]);
	if(status==PACKAGE_BROADCAST)
	{
		LOGS("broadcast!\r\n");
	}
	else if(status==PACKAGE_SUCCESS)
	{
		LOGS("ok!\r\n");
	}
	else
	{
		LOGS("fail!\r\n");
	}
}
static void on_recv_cb(rt_uint8_t src_addr,rt_uint8_t dst_addr,rt_uint8_t pack_type,rt_uint8_t *buf)
{
	LOG("on_recv_cb:src=%d,dst=%d,type=%d,data[0]=%d,data[1]=%d\r\n",src_addr,dst_addr,pack_type,buf[0],buf[1]);
    if(nrf_door_update_hook != RT_NULL)
    {
        payload_data_t payload_data = (payload_data_t)buf;
        if(payload_data->cmd == CMD_DOOR_STA)
        {
            rt_uint8_t group_index = CONVERT_TO_DOOR_INDEX(src_addr);
            rt_uint16_t sta = payload_data->args.door_sta;
            nrf_door_update_hook(group_index, sta);
        }
    }
}

static void nrfstatus(void)
{
	int i;
	nrf_node_status node;
	nrf_get_nodestatus(&node);
	LOGS("======node_status======\r\n");
	LOGS("channel=%d\r\n",node.channel);
	LOGS("addr=%d\r\n",node.addr);
	LOGS("assign_addr=%d\r\n",node.assign_addr);
	LOGS("mac:");
	for(i=0;i<MAC_LEN;i++)
	{
		LOGS("%02x",node.node_mac[i]);
	}
	LOGS("\r\n");	
	LOGS("sys_pwd=%08X\r\n",node.pwd);
	LOGS("heart_status=%d\r\n",node.heart_status);
	LOGS("shift_failed_count=%d\r\n",node.shift_failed_count);
	LOGS("shift_count=%d\r\n",node.shift_count);
	LOGS("recv_count=%d\r\n",node.recv_count);
	LOGS("======================\r\n");
}
MSH_CMD_EXPORT(nrfstatus, show nrf status);

void nrfreset(void)
{	
	nrf_reset();
    LOG("nrfreset\n");
}

void nrf_send_net_pwd(void)
{
    nrf_assign_addr(0);
    LOG("nrf_assign_addr\n");
}

void nrf_send_door_open(rt_uint8_t group_index, rt_uint8_t door_index)
{
    payload_data_t payload_data = rt_calloc(1, sizeof(struct payload_data));
    payload_data->cmd = CMD_DOOR_OPEN;
    payload_data->args.door_index = door_index;
    nrf_send_data(CONVERT_TO_NET_ADDR(group_index), payload_data);    
    rt_free(payload_data);
}

void nrf_send_group_open(rt_uint8_t group_index)
{
    payload_data_t payload_data = rt_calloc(1, sizeof(struct payload_data));
    payload_data->cmd = CMD_GROUP_OPEN;
    nrf_send_data(CONVERT_TO_NET_ADDR(group_index), payload_data);    
    rt_free(payload_data);    
}

void nrf_set_remote_door_update_hook(nrf_door_update_hook_t hook)
{
    nrf_door_update_hook = hook;
}

void app_nrf_gateway_startup(void)
{
	nrf_reg_callback(on_conn_cb, on_disconn_cb,
					on_send_cb, on_recv_cb);	
}
