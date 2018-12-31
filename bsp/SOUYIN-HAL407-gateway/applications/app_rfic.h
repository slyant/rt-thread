#ifndef __APP_RFIC_H__
#define __APP_RFIC_H__

#include <rtthread.h>

typedef enum
{
	CARD_APP_TYPE_ABKEY = 0,	//密钥卡
	CARD_APP_TYPE_CONFIG,		//配置卡
	CARD_APP_TYPE_POWER,        //授权卡
	CARD_APP_TYPE_EKEY,			//钥匙卡
	CARD_APP_TYPE_DRIVER,		//司机卡
	CARD_APP_TYPE_LOCKKEY,		//锁钥卡
	CARD_APP_TYPE_UNKNOW		//未知卡
}card_app_type_t;//IC卡应用类型枚举

rt_bool_t init_card_key(void);
rt_bool_t reset_card_key(void);
rt_bool_t create_card_key(void);
rt_bool_t backup_card_key(void);
rt_bool_t restore_card_key(void);

rt_bool_t init_card_app(void);
rt_bool_t reset_card_app(void);

rt_bool_t create_app_config(void);
rt_bool_t create_app_power(void);

#endif
