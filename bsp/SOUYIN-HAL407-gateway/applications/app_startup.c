#include <app_config.h>
#include <drv_pcf8563.h>
#include <stm32f4xx_hal_cortex.h>

const char* INIT_SYS_TITLE = "公交自助收银管理系统V1.0\0";
const unsigned char INIT_SYS_KEY_A[INIT_KEY_LEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
const unsigned char INIT_SYS_KEY_B[INIT_KEY_LEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

static enum sys_workmodel workmodel;
struct sys_status sys_status;
struct sys_config sys_config;

static void sys_restart(void)
{
	HAL_NVIC_SystemReset();
}
static void set_datetime(calendar_t cal)
{	
	rtc_set_time(cal->year, cal->month, cal->mday, cal->hour, cal->min, cal->sec);
	lcd_set_datetime(cal->year, cal->month, cal->mday, cal->wday, cal->hour, cal->min, cal->sec);
}

static void gps_update_hook(calendar_t cal)
{
	static int count = 0;
	if(count == 0)
	{
		set_datetime(cal);
		rt_kprintf("datetime:%04d-%02d-%02d %02d:%02d:%02d\n", cal->year, cal->month, cal->mday, cal->hour, cal->min, cal->sec);
	}
	else if(count >= 5*60)	//5分钟同步1次
	{	
		count = 0;
	}
	count++;
}

static rt_bool_t get_datetime(calendar_t cal)
{
	int year, month, mday, wday, hour, min, sec;	
	if(rtc_get_time(&year, &month, &mday, &wday, &hour, &min, &sec))
	{		
		cal->year = year;
		cal->month = month;
		cal->mday = mday;
		cal->wday = wday;
		cal->hour = hour;
		cal->min = min;
		cal->sec = sec;	
		return RT_TRUE;
	}		
	return RT_FALSE;
}

static void load_datetime(void)
{
	calendar_t cal = rt_calloc(1, sizeof(struct calendar));
	if(get_datetime(cal))
	{
		lcd_set_datetime(cal->year, cal->month, cal->mday, cal->wday, cal->hour, cal->min, cal->sec);
	}
	rt_free(cal);
}

static void set_workmodel(enum sys_workmodel model)
{
	workmodel = model;
	switch(model)
	{
	case CONFIG_ABKEY_MODEL:
		lcd_set_screen_id(UI_ABKEY_CARD);
		break;
	case CONFIG_MANAGE_MODEL:
		lcd_set_screen_id(UI_SYS_CFG);
		break;
	case WORK_MANAGE_MODEL:
		lcd_set_screen_id(UI_OPEN_DOOR);
		break;
	case WORK_ON_MODEL:
		lcd_set_screen_id(UI_MAIN);
		break;
	case WORK_OFF_MODEL:
		lcd_set_screen_id(UI_MAIN);
		break;
	default:
		lcd_set_screen_id(UI_MAIN);
		break;
	}
}

static enum sys_workmodel get_workmodel(void)
{
	return workmodel;
}

static rt_bool_t update_sys_key(rt_uint8_t *in_akey, rt_uint8_t *in_bkey)
{
	rt_bool_t result = RT_FALSE;
	//保存系统密钥
	struct sysinfo sysinfo;
	if(sysinfo_get_by_id(&sysinfo, SYSINFO_DB_KEY_ID)>0)
	{
		rt_memcpy(sysinfo.key_a, in_akey, INIT_KEY_LEN);
		rt_memcpy(sysinfo.key_b, in_bkey, INIT_KEY_LEN);
		if(sysinfo_update(&sysinfo) == 0)
		{
			rt_memcpy(sys_config.keya, in_akey, INIT_KEY_LEN);
			rt_memcpy(sys_config.keyb, in_bkey, INIT_KEY_LEN);
			rt_kprintf("sysinfo_update OK!\n");
			result = RT_TRUE;
		}
	}
	else
	{
		rt_kprintf("sysinfo_update ERROR!\n");
	}	
	return result;
}

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
	rt_thread_mdelay(1*60*1000);
	while(1);
}

void app_startup(void)
{
	rt_assert_set_hook(assert_hook);
	gps_set_update_hook(gps_update_hook);
	
	rt_memset((rt_uint8_t*)&sys_config, 0x00, sizeof(struct sys_config));
	rt_memset((rt_uint8_t*)&sys_status, 0x00, sizeof(struct sys_status));
	sys_status.get_workmodel = get_workmodel;
	sys_status.set_workmodel = set_workmodel;
	sys_config.update_sys_key = update_sys_key;
	sys_status.get_datetime = get_datetime;
	sys_status.set_datetime = set_datetime;
	sys_status.restart = sys_restart;	
	sys_status.set_workmodel(WORK_OFF_MODEL);
	
	load_datetime();

	extern void app_sqlite_init(void);
	app_sqlite_init();	
	extern void app_lcd_startup(void);
	app_lcd_startup();
	extern void app_rfic_startup(void);
	app_rfic_startup();
	extern void app_nrf_gateway_startup(void);	
	app_nrf_gateway_startup();

	struct sysinfo sysinfo;
	RT_ASSERT(sysinfo_get_by_id(&sysinfo, SYSINFO_DB_KEY_ID)>0);

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
		if(cardinfo_count_by_type(CARD_APP_TYPE_CONFIG) < CONFIG_CARD_MAX_COUNT)
		{
			sys_status.set_workmodel(CONFIG_MANAGE_MODEL);
		}
		else
		{
			sys_status.set_workmodel(WORK_ON_MODEL);
		}
	}
	else
	{
		sys_status.set_workmodel(CONFIG_ABKEY_MODEL);
	}
}
