#ifndef __APP_CONFIG_H__
#define __APP_CONFIG_H__

#include <rtthread.h>
#include <board.h>

#define INIT_KEY_LEN			6		//初始化密钥长度
#define SYSINFO_KEY_ID			1   	//系统信息主键ID
#define CONFIG_CARD_MAX_COUNT	3		//配置卡最大数量
#define POWER_CARD_MAX_COUNT	3		//授权卡最大数量
#define EKEY_CARD_MAX_COUNT		50		//钥匙卡最大数量
#define DRIVER_CARD_MAX_COUNT	65535	//司机卡最大数量
#define GROUP_MAX_COUNT			8		//节点最大数量
#define DOOR_MAX_COUNT			16		//门最大数量
#define OPEN_DISPLAY_TIME       5000    //柜门编号显示时间ms
#define MANAGE_DISPLAY_TIME     10000   //管理界面显示时间ms
//柜门状态
#define DOOR_STA_INIT       0   //初始
#define DOOR_STA_LOCK       1   //上锁
#define DOOR_STA_OPEN_1     2   //第1次开门
#define DOOR_STA_CLOSE_1    3   //第1次关门
#define DOOR_STA_OPEN_2     4   //第2次开门
#define DOOR_STA_CLOSE_2    5   //第2次关门
//根据组ID和柜门ID计算柜门全局ID
#define GET_GLOBAL_ID(g,d)          ((g)*DOOR_MAX_COUNT+(d))
//根据柜门全局ID计算柜组ID
#define GET_GROUP_ID(id)            ((id)/DOOR_MAX_COUNT)
//根据柜门全局ID计算柜门ID
#define GET_DOOR_ID(id)             ((id)%DOOR_MAX_COUNT)

#include <db_include.h>
#include <rng_helper.h>
#include <any_convert.h>
#include <app_sqlite.h>
#include <app_workqueue.h>
#include <app_beep.h>
#include <app_gps.h>

#include <app_rfic.h>
#include <app_lcd.h>
#include <app_door.h>
#include <app_nrf_gateway.h>

enum sys_workmodel
{
	WAITE_RESTART_MODEL = 0,	//等待重启模式
	CONFIG_ABKEY_MODEL,			//密钥卡设置模式
	CONFIG_MANAGE_MODEL,		//系统配置模式
	WORK_MANAGE_MODEL,			//管理员模式
	WORK_ON_MODEL,				//正常工作模式
	WORK_OFF_MODEL				//停止工作模式
};

struct sys_config
{
	rt_uint8_t en_driver_card;
	rt_uint16_t open_timeout;
	rt_uint8_t node_count;
	rt_uint8_t door_count;
	rt_uint8_t keya[INIT_KEY_LEN];
	rt_uint8_t keyb[INIT_KEY_LEN];  
	rt_bool_t (*sys_reset)(void);	
	rt_bool_t (*abkey_exist)(void);
	rt_bool_t (*update_sys_key)(rt_uint8_t*, rt_uint8_t*);
};
typedef struct sys_config *sys_config_t;

struct sys_status
{		
	void (*restart)(void);
    void (*open_display_start)(void);
    void (*manage_display_start)(void);
	void (*set_workmodel)(enum sys_workmodel);
	enum sys_workmodel(*get_workmodel)(void);	
	rt_bool_t (*get_datetime)(struct calendar **);
	void (*set_datetime)(struct calendar *);
    void (*set_door_group_sta)(rt_uint8_t, rt_uint16_t);
    rt_uint16_t (*get_door_group_sta)(rt_uint8_t);
    rt_uint32_t card_num;    
};
typedef struct sys_status *sys_status_t;

extern const char *INIT_SYS_TITLE;
extern const unsigned char INIT_SYS_KEY_A[INIT_KEY_LEN];
extern const unsigned char INIT_SYS_KEY_B[INIT_KEY_LEN];
extern struct sys_status sys_status;
extern struct sys_config sys_config;

#endif
