#ifndef __APP_CONFIG_H__
#define __APP_CONFIG_H__

#include <rtthread.h>
#include <board.h>

#include <db_include.h>
#include <app_beep.h>
#include <any_convert.h>
#include <drv_pcf8563.h>
#include <app_gps.h>
#include <app_lcd.h>

#define INIT_KEY_LEN		6	//初始化密钥长度
#define SYSINFO_DB_KEY_ID	1   //系统信息主键ID

enum sys_workmodel
{
	CONFIG_ABKEY_MODEL = 0,
	CONFIG_MANAGE_MODEL,
	WORK_MANAGE_MODEL,
	WORK_ON_MODEL,
	WORK_OFF_MODEL
};

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
	void (*restart)(void);
	void (*set_workmodel)(enum sys_workmodel);
	enum sys_workmodel (*get_workmodel)(void);
	rt_bool_t (*get_datetime)(calendar_t cal);
	void (*set_datetime)(calendar_t cal);
	void (*set_screen_id)(rt_uint16_t id);
	void (*set_screen_back)(void);
	rt_uint16_t (*get_screen_id)(void);
};
typedef struct sys_status *sys_status_t;

extern const char* INIT_SYS_TITLE;
extern const unsigned char INIT_SYS_KEY_A[INIT_KEY_LEN];
extern const unsigned char INIT_SYS_KEY_B[INIT_KEY_LEN];
extern struct sys_status sys_status;
extern struct sys_config sys_config;

#endif
