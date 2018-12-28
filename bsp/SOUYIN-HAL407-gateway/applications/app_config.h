#ifndef __APP_CONFIG_H__
#define __APP_CONFIG_H__

#include <rtthread.h>
#include <board.h>

#include <cJSON_util.h>
#include <ic_card_protocol.h>
#include <db_include.h>
#include <app_beep.h>
#include <any_convert.h>
#include <rng_helper.h>
#include <drv_pcf8563.h>
#include <hmi_driver.h>

#define INIT_KEY_LEN		6	//初始化密钥长度
#define SYSINFO_DB_KEY_ID	1   //系统信息主键ID
#define IC_LOCK()		rt_mutex_take(&sys_status.rfic_lock,RT_WAITING_FOREVER)
#define IC_UNLOCK()		rt_mutex_release(&sys_status.rfic_lock)


struct sys_config
{
    char sys_title[60];
	rt_uint16_t open_timeout;
	rt_uint16_t node_count;
	rt_uint16_t door_count;
	rt_uint8_t keya[INIT_KEY_LEN];
	rt_uint8_t keyb[INIT_KEY_LEN];    
	rt_bool_t (*abkey_exist)(void);
};
typedef struct sys_config *sys_config_t;

struct sys_status
{
	struct rt_mutex rfic_lock;
	struct calendar sys_datetime;
};
typedef struct sys_status *sys_status_t;

extern const char* INIT_SYS_TITLE;
extern const unsigned char INIT_SYS_KEY_A[INIT_KEY_LEN];
extern const unsigned char INIT_SYS_KEY_B[INIT_KEY_LEN];
extern struct sys_status sys_status;
extern struct sys_config sys_config;
#endif
