#ifndef __APP_RFIC_H__
#define __APP_RFIC_H__

#include <rtthread.h>

//IC卡应用类型枚举
enum card_app_type
{
	CARD_APP_TYPE_ABKEY = 0,	//密钥卡
	CARD_APP_TYPE_CONFIG,		//配置卡
	CARD_APP_TYPE_POWER,        //授权卡
	CARD_APP_TYPE_EKEY,			//钥匙卡
	CARD_APP_TYPE_DRIVER,		//司机卡
	CARD_APP_TYPE_LOCKKEY,		//锁钥卡
	CARD_APP_TYPE_UNKNOW		//未知卡
};

rt_bool_t init_card_key(void);
rt_bool_t reset_card_key(void);
rt_bool_t create_card_key(void);
rt_bool_t backup_card_key(void);
rt_bool_t restore_card_key(void);

void app_rfic_startup(void);
rt_bool_t init_card_app(rt_bool_t use_money_bag);
rt_bool_t reset_card_app(void);
rt_bool_t create_card_app(enum card_app_type type, rt_uint16_t num, char *pwd);
#endif
