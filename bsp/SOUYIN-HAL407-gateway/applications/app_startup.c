#include <app_config.h>
#include <drv_pcf8563.h>
#include <rtdevice.h>

const char *INIT_SYS_TITLE = "System Title\0";
const unsigned char INIT_SYS_KEY_A[INIT_KEY_LEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
const unsigned char INIT_SYS_KEY_B[INIT_KEY_LEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

static rt_mutex_t group_sta_lock = RT_NULL;
static rt_mutex_t door_update_lock = RT_NULL;
static enum sys_workmodel workmodel;
static rt_uint16_t door_sta[GROUP_MAX_COUNT];
struct sys_status sys_status;
struct sys_config sys_config;

//系统重置
static rt_bool_t sys_reset(void)
{
	return db_delete_database() == 0 ? RT_TRUE:RT_FALSE;
}
//系统重启
static void sys_restart(void)
{
    nrf_send_reset();           //发送节点复位指令
    rt_thread_mdelay(2000);
	HAL_NVIC_SystemReset();
}
static void set_datetime(struct calendar *cal)
{	
	rtc_set_time(cal->year, cal->month, cal->mday, cal->hour, cal->min, cal->sec);
	lcd_set_datetime(cal->year, cal->month, cal->mday, cal->wday, cal->hour, cal->min, cal->sec);
}
static rt_bool_t get_datetime(struct calendar **cal)
{
	int year, month, mday, wday, hour, min, sec;	
	if(rtc_get_time(&year, &month, &mday, &wday, &hour, &min, &sec))
	{		
		(*cal)->year = year;
		(*cal)->month = month;
		(*cal)->mday = mday;
		(*cal)->wday = wday;
		(*cal)->hour = hour;
		(*cal)->min = min;
		(*cal)->sec = sec;	
		return RT_TRUE;
	}		
	return RT_FALSE;
}
static void set_door_group_sta(rt_uint8_t group_index, rt_uint16_t sta)
{
    rt_mutex_take(group_sta_lock, RT_WAITING_FOREVER);
    if(group_index < GROUP_MAX_COUNT)
    {
        door_sta[group_index] = sta;
    }
    rt_mutex_release(group_sta_lock);
}
static rt_uint16_t get_door_group_sta(rt_uint8_t group_index)
{
    rt_uint16_t result = 0;
    rt_mutex_take(group_sta_lock, RT_WAITING_FOREVER);
    if(group_index < GROUP_MAX_COUNT)
    {
        result = door_sta[group_index];
    }
    rt_mutex_release(group_sta_lock);
    return result;
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

static void door_update_hook(rt_uint8_t group_index, rt_uint16_t sta)
{
    rt_mutex_take(door_update_lock, RT_WAITING_FOREVER);
    set_door_group_sta(group_index, sta);
    lcd_update_door_sta(group_index);
    lcd_wakeup();
    rt_kprintf("group_index:%d,door_sta:%04X\n", group_index, sta);
    app_workqueue_update_door(group_index, sta);
    rt_mutex_release(door_update_lock);
}

static void load_datetime(void)
{
	calendar_t cal = rt_calloc(1, sizeof(struct calendar));
	if(get_datetime(&cal))
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
		lcd_set_open_door();
		lcd_set_screen_id(UI_OPEN_DOOR);
		break;
	case WORK_ON_MODEL:
		lcd_set_screen_id(UI_MAIN);
		break;
	case WORK_OFF_MODEL:
		lcd_set_screen_id(UI_WAITE);
		break;
	default:
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
	sysinfo_t sysinfo = rt_calloc(1, sizeof(struct sysinfo));
	if(sysinfo_get_by_id(sysinfo, SYSINFO_KEY_ID)>0)
	{
		rt_memcpy(sysinfo->key_a, in_akey, INIT_KEY_LEN);
		rt_memcpy(sysinfo->key_b, in_bkey, INIT_KEY_LEN);
		if(sysinfo_update(sysinfo) == 0)
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
	rt_free(sysinfo);
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
	while(1)rt_thread_mdelay(1000);
}

void app_bat_work(void)
{
	rt_assert_set_hook(assert_hook);
    group_sta_lock = rt_mutex_create("gsta_lock", RT_IPC_FLAG_FIFO);
    RT_ASSERT(group_sta_lock != RT_NULL);
    door_update_lock = rt_mutex_create("door_lock", RT_IPC_FLAG_FIFO);
    RT_ASSERT(door_update_lock != RT_NULL);    
    
    rt_memset(door_sta, 0x00, sizeof(door_sta));	
	rt_memset((rt_uint8_t*)&sys_config, 0x00, sizeof(struct sys_config));
	rt_memset((rt_uint8_t*)&sys_status, 0x00, sizeof(struct sys_status));
	sys_status.get_workmodel = get_workmodel;
	sys_status.set_workmodel = set_workmodel;
	sys_config.update_sys_key = update_sys_key;
	sys_config.sys_reset = sys_reset;
	sys_status.get_datetime = get_datetime;
	sys_status.set_datetime = set_datetime;
    sys_status.get_door_group_sta = get_door_group_sta;
    sys_status.set_door_group_sta = set_door_group_sta;
	sys_status.restart = sys_restart;
	sys_status.set_workmodel(WORK_OFF_MODEL);    
    gps_update_set_hook(gps_update_hook);
    door_update_set_hook(door_update_hook);
    lcd_wakeup();//唤醒屏幕    
	load_datetime();

	app_sqlite_init();      //要先初始化sqlite数据库	

	sysinfo_t sysinfo = rt_calloc(1, sizeof(struct sysinfo));
	RT_ASSERT(sysinfo_get_by_id(sysinfo, SYSINFO_KEY_ID)>0);
	sys_config.en_driver_card = sysinfo->en_driver_card;
	sys_config.door_count = sysinfo->door_count;
	sys_config.node_count = sysinfo->node_count;
	sys_config.open_timeout = sysinfo->open_timeout;
	rt_memcpy(sys_config.keya, sysinfo->key_a, INIT_KEY_LEN);
	rt_memcpy(sys_config.keyb, sysinfo->key_b, INIT_KEY_LEN);
	sys_config.abkey_exist = sys_abkey_exist;	
	lcd_set_sys_title(sysinfo->sys_title);//设置系统标题
	rt_kprintf("\nen_driver_card:%d\nopen_timeout:%d\ndoor_count:%d\nnode_count:%d\n", sys_config.en_driver_card, sys_config.open_timeout, sysinfo->door_count, sys_config.node_count);
	rt_kprintf("keya:%02X%02X%02X%02X%02X%02X\n", sys_config.keya[0], sys_config.keya[1], sys_config.keya[2], sys_config.keya[3], sys_config.keya[4], sys_config.keya[5]);
	rt_kprintf("keyb:%02X%02X%02X%02X%02X%02X\n", sys_config.keyb[0], sys_config.keyb[1], sys_config.keyb[2], sys_config.keyb[3], sys_config.keyb[4], sys_config.keyb[5]);		
	rt_free(sysinfo);    
	
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
    
	app_lcd_startup();
	app_rfic_startup();
	app_nrf_gateway_startup();
    app_door_startup();	
}

void app_startup(void)
{
    app_workqueue_startup();   
    app_workqueue_exe_void(app_bat_work);
}
