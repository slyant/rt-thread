#include <app_config.h>

extern void app_sqlite_init(void);
extern void uart_lcd_startup(void);
extern void app_rfic_startup(void);
extern void app_nrf_gateway_startup(void);

const char* INIT_SYS_TITLE = "公交自助收银管理系统V1.0\0";
const unsigned char INIT_SYS_KEY_A[INIT_KEY_LEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
const unsigned char INIT_SYS_KEY_B[INIT_KEY_LEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

struct sys_status sys_status;
struct sys_config sys_config;

static rt_bool_t sys_abkey_exist(void)
{
	if(rt_memcmp(sys_config.keya, INIT_SYS_KEY_A, INIT_KEY_LEN)==0 && rt_memcmp(sys_config.keyb, INIT_SYS_KEY_B, INIT_KEY_LEN)==0)
	{
		return RT_FALSE;
	}
	return RT_TRUE;
}

static void assert_hook(const char *ex, const char *func, rt_size_t line)
{
	char *err = rt_calloc(1, 512);
	rt_sprintf(err, "(%s) assertion failed at function:%s, line number:%d \n", ex, func, line);
	rt_kprintf("%s", err);
	lcd_show_error(err);
	rt_free(err);
	
	while(1);
}

void app_startup(void)
{
	rt_assert_set_hook(assert_hook);
//	RT_ASSERT(RT_FALSE);
	app_sqlite_init();
	

	rt_memset((rt_uint8_t*)&sys_config, 0x00, sizeof(struct sys_config));
	rt_memset((rt_uint8_t*)&sys_status, 0x00, sizeof(struct sys_status));
	
	rt_mutex_init(&sys_status.rfic_lock, "ic_lock", RT_IPC_FLAG_FIFO);
	
	struct sysinfo sysinfo;
	if(sysinfo_get_by_id(&sysinfo, SYSINFO_DB_KEY_ID)>0)
	{
		uart_lcd_startup();
		
		sys_config.door_count = sysinfo.door_count;
		sys_config.node_count = sysinfo.node_count;
		sys_config.open_timeout = sysinfo.open_timeout;
		rt_strncpy(sys_config.sys_title, sysinfo.sys_title, rt_strlen(sysinfo.sys_title));
		rt_memcpy(sys_config.keya, sysinfo.key_a, INIT_KEY_LEN);
		rt_memcpy(sys_config.keyb, sysinfo.key_b, INIT_KEY_LEN);
		sys_config.abkey_exist = sys_abkey_exist;
		
		rt_kprintf("\nsys_title:%s\nopen_timeout:%d\ndoor_count:%d\nnode_count:%d\n",sys_config.sys_title, sys_config.open_timeout, sysinfo.door_count, sys_config.node_count);
		rt_kprintf("keya:%02X%02X%02X%02X%02X%02X\n", sys_config.keya[0], sys_config.keya[1], sys_config.keya[2], sys_config.keya[3], sys_config.keya[4], sys_config.keya[5]);
		rt_kprintf("keyb:%02X%02X%02X%02X%02X%02X\n", sys_config.keyb[0], sys_config.keyb[1], sys_config.keyb[2], sys_config.keyb[3], sys_config.keyb[4], sys_config.keyb[5]);
			
		if(sys_config.abkey_exist())
		{
			lcd_set_screen(UI_MAIN);
		}
		else
		{
			lcd_set_screen(UI_ABKEY_CARD);
			return;
		}		
	}
	else
	{
		rt_kprintf("sysinfo table is not exist a record!");
		RT_ASSERT(RT_FALSE);
	}	
	
	app_rfic_startup();
	app_nrf_gateway_startup();
}
