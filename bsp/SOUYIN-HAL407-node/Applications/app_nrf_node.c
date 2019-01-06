#include <rtthread.h>
#include <dfs_posix.h>
#include <nrf_node.h>
#include <app_config.h>

#define APP_USING_LOG
#ifdef APP_USING_LOG
	#define LOG		rt_kprintf("[app.nrf.node]:");rt_kprintf
	#define LOGS	rt_kprintf
#else
	#define LOG(...)
	#define LOGS(...)
#endif

#define NRF_NET_OPTION_FILE     "/spi/nrf.bin"
#define CONVERT_TO_NET_ADDR(d)     (ASSIGN_ADDR_START+d)       //将柜门地址转换为网络地址
#define CONVERT_TO_DOOR_INDEX(d)   (d-ASSIGN_ADDR_START)       //将网络地址转换为柜门地址

static rt_mailbox_t ass_addr_mb = RT_NULL;
static nrf_cmd_handle_hook_t nrf_cmd_handle_hook = RT_NULL;

static rt_uint8_t IsLink = 0;

static rt_bool_t read_nrf_net_option(nrf_net_option_t option)
{
    rt_bool_t result = RT_FALSE;
    int fd = open(NRF_NET_OPTION_FILE, O_RDONLY);
    if(fd >= 0)
    {
        if(read(fd, option, sizeof(struct nrf_net_option)) == sizeof(struct nrf_net_option))
        {
            result = RT_TRUE;
        }
        close(fd);        
    }
    return result;
}

static rt_bool_t save_nrf_net_option(nrf_net_option_t option)
{
    rt_bool_t result = RT_FALSE;
    int fd = open(NRF_NET_OPTION_FILE, O_WRONLY | O_CREAT);
    if(fd >= 0)
    {
        lseek(fd, 0, SEEK_SET);
        if(write(fd, option, sizeof(struct nrf_net_option)) == sizeof(struct nrf_net_option))
        {
            result = RT_TRUE;
        }
        close(fd);        
    }
    return result;    
}

static rt_bool_t del_nrf_net_option(void)
{
    rt_bool_t result = RT_FALSE;
    int fd = unlink(NRF_NET_OPTION_FILE);
    return fd >= 0 ? RT_TRUE:RT_FALSE; 
}

void nrf_clear_reset(void)
{
    del_nrf_net_option();
    nrf_reset();
}

static void on_ass_addr_cb(rt_uint8_t addr, rt_uint32_t pwd)
{
    LOG("on_ass_addr_cb:addr:%d, pwd:%08X\n", addr, pwd);
    if(ass_addr_mb != RT_NULL)
    {
        nrf_net_option_t option = rt_calloc(1, sizeof(struct nrf_net_option));
        option->addr = CONVERT_TO_NET_ADDR(door_get_group_addr());
        option->pwd = pwd;
        rt_mb_send(ass_addr_mb, (rt_ubase_t)option);
    }
}

static void on_conn_cb(void* args)
{
	//nrf_node_status *nodestatus = args;
	IsLink = 1;
	LOG("NRF Connected!\r\n");
}
static void on_disconn_cb(void* args)
{
	//nrf_node_status *nodestatus = args;
	IsLink = 0;
	LOG("NRF DisConnected!\r\n");
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
    if(nrf_cmd_handle_hook != RT_NULL)
    {
        payload_data_t payload_data = (payload_data_t)buf;
        nrf_cmd_handle_hook(payload_data);
    }    
}

static void nrfstatus(void)
{
	int i;
	nrf_node_status node;
	nrf_get_nodestatus(&node);
	LOGS("======NodeStatus======\r\n");
	switch(node.link_status)
	{
		case DISCONNECT:
			LOGS("link_status=DISCONNECT\r\n");
			break;
		case WAIT_CONNECT:
			LOGS("link_status=WAIT_CONNECT\r\n");
			break;
		case CONNECTED:
			LOGS("link_status=CONNECTED\r\n");
			break;
		case WAIT_CHANGE:
			LOGS("link_status=WAIT_CHANGE\r\n");
			break;
	}
	LOGS("channel=%d\r\n",node.channel);
	LOGS("addr=%d\r\n",node.addr);
	LOGS("mac:");
	for(i=0;i<MAC_LEN;i++)
	{
		LOGS("%02x",node.node_mac[i]);
	}
	LOGS("\r\n");	
	LOGS("sys_pwd=%08X\r\n",node.pwd);
	LOGS("shift_failed_count=%d\r\n",node.shift_failed_count);
	LOGS("shift_count=%d\r\n",node.shift_count);
	LOGS("recv_count=%d\r\n",node.recv_count);
	LOGS("======================\r\n");
}
MSH_CMD_EXPORT(nrfstatus, show nrf status);

static void nrfreset(void)
{
	nrf_reset();
}
MSH_CMD_EXPORT(nrfreset, nrf wait connect);

void nrf_send_door_sta(rt_uint16_t sta)
{
    if(IsLink)
    {
        payload_data_t payload_data = rt_calloc(1, sizeof(struct payload_data));
        payload_data->cmd = CMD_DOOR_STA;
        payload_data->args.door_sta = sta;
        nrf_send_data(GATEWAY_ADDR, payload_data);    
        rt_free(payload_data);    
    }
    else
    {
        LOG("nrf send data failed! Nrf Net is DisConnected!\n");
    }
}

void nrf_set_cmd_handle_hook(nrf_cmd_handle_hook_t hook)
{
    nrf_cmd_handle_hook = hook;
}

static void ass_addr_thread_entry(void *params)
{    
    rt_ubase_t value;
    while(1) 
    {
        if(rt_mb_recv(ass_addr_mb, &value, RT_WAITING_FOREVER) == RT_EOK)
        {
            nrf_net_option_t option = (nrf_net_option_t)value;
            if(save_nrf_net_option(option))
            {
                LOG("save_nrf_net_option:addr:%d, pwd:%08X\n", option->addr, option->pwd);
                nrf_set_addr(option->addr, option->pwd);
                rt_free(option);
                rt_mb_delete(ass_addr_mb);
                return;
            }            
        }
    }        
}

int app_nrf_node_startup(void)
{
	nrf_net_option_t option = rt_calloc(1, sizeof(struct nrf_net_option));
    if(read_nrf_net_option(option))
    {
        option->addr = CONVERT_TO_NET_ADDR(door_get_group_addr());
        nrf_set_addr(option->addr, option->pwd);
    }
    else
    {
        ass_addr_mb = rt_mb_create("nrf_ass_mb", 1, RT_IPC_FLAG_FIFO);
        RT_ASSERT(ass_addr_mb != RT_NULL);
        
        rt_thread_t thread = rt_thread_create("tass", ass_addr_thread_entry, RT_NULL,
                                                1024, 15, 20);
        if(thread != RT_NULL)
            rt_thread_startup(thread);
    }
    rt_free(option);
	nrf_reg_callback(on_conn_cb, on_disconn_cb,
					on_send_cb, on_recv_cb, on_ass_addr_cb);
	return RT_EOK;
}
