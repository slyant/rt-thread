#include <rtthread.h>
#include <nrf_gateway.h>
#include <stdlib.h>
#include <rtdevice.h>
#include <uart_lcd_process.h>

#define APP_USING_LOG
#ifdef APP_USING_LOG
	#define LOG		rt_kprintf("[NRF_APP]:");rt_kprintf
	#define LOGS	rt_kprintf
#else
	#define NRF_LOG(...)
	#define NRF_LOGS(...)
#endif

static void on_conn_cb(void* args)
{
	uint8_t node = *(uint8_t*)args;
	LOG("NRF Node %d Connected!\r\n", node);
	//sark_join_cfg(node);
}
static void on_disconn_cb(void* args)
{
	uint8_t node = *(uint8_t*)args;
	LOG("NRF Node %d DisConnected!\r\n", node);
	//sark_discon_remov(node);
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
//	rt_pin_write(LED_0_PIN,buf[0]);
//	rt_pin_write(LED_1_PIN,buf[1]);
}
static void nrfstatus(void)
{
	int i;
	nrf_node_status node;
	nrf_get_nodestatus(&node);
	rt_kprintf("======node_status======\r\n");
	rt_kprintf("channel=%d\r\n",node.channel);
	rt_kprintf("addr=%d\r\n",node.addr);
	rt_kprintf("assign_addr=%d\r\n",node.assign_addr);
	rt_kprintf("mac:");
	for(i=0;i<MAC_LEN;i++)
	{
		rt_kprintf("%02x",node.node_mac[i]);
	}
	rt_kprintf("\r\n");	
	rt_kprintf("sys_pwd=%08X\r\n",node.pwd);
	rt_kprintf("heart_status=%d\r\n",node.heart_status);
	rt_kprintf("shift_failed_count=%d\r\n",node.shift_failed_count);
	rt_kprintf("shift_count=%d\r\n",node.shift_count);
	rt_kprintf("recv_count=%d\r\n",node.recv_count);
	rt_kprintf("======================\r\n");
}
MSH_CMD_EXPORT(nrfstatus, show nrf status);

static void nrfaddr(int argc, char** argv)
{	
	rt_uint8_t addr = 0;
	if(argc>=2)
	{
		addr = atol(argv[1]);
	}
	nrf_assign_addr(addr);
}
MSH_CMD_EXPORT(nrfaddr, nrf assign addr);

static void nrfreset(void)
{	
	nrf_reset();
}
MSH_CMD_EXPORT(nrfreset, nrf reset);

static void nrfsend(int argc, char** argv)
{
	if(argc==3)
	{
		rt_uint8_t addr = atol(argv[1]);
		char* data = argv[2];
		data[0] = (data[0]=='0')?0:1;
		data[1] = (data[1]=='0')?0:1;
		if(addr==0)
		{
			nrf_send_data_all_nodes(data);
		}
		else
		{
			nrf_send_data(addr, data);
		}
	}
}
MSH_CMD_EXPORT(nrfsend, nrf send data);

void app_nrf_gateway_startup(void)
{
	nrf_reg_callback(on_conn_cb, on_disconn_cb,
					on_send_cb, on_recv_cb);	
}
