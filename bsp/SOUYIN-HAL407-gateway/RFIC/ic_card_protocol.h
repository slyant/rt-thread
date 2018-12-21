#ifndef __IC_CARD_PROTOCOL_H__
#define __IC_CARD_PROTOCOL_H__
#include <rtthread.h>

#define BLOCK_SIZE				16
#define KEY_LENGTH				6
#define CTRL_LENGTH				4
#define SIGNATURE_LENGTH		16
#define CARD_INF_BLOCK_COUNT	18	//最大值45(15*3=45)
#define CARD_INF_MAX_LEN		(BLOCK_SIZE * CARD_INF_BLOCK_COUNT)

/******************数据块地址定义**********************/
#define CARD_CHECK_BLOCK			1	//卡签名数据块
#define CARD_LEN_BLOCK				2	//卡信息长度数据块
#define CARD_CTRL_BLOCK				3	//卡控制数据块
#define MONEY_BAG_ENABLE			32	//电子钱包启用标记数据块
#define MONEY_BAG_VALUE				33	//电子钱包值数据块
#define MONEY_BAG_BF				34	//电子钱包备份数据块 
#define MONEY_BAG_CTRL				35	//电子钱包备份数据块 

typedef enum
{
	CARD_TYPE_BLANK = 0,    //空白卡
	CARD_TYPE_KEY,			//密钥卡
	CARD_TYPE_APP,			//应用卡
	CARD_TYPE_UNKNOW,		//未知卡
	CARD_TYPE_NULL			//无卡
}card_base_type_t;//IC卡基础类型枚举

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

card_base_type_t rfid_scan_handle(rt_uint8_t *in_key_a, rt_uint8_t *in_key_b, rt_uint8_t *out_card_id, rt_uint8_t *out_buffer, rt_uint16_t *out_buffer_len);
rt_bool_t rfid_card_init(card_base_type_t type, rt_bool_t use_money_bag, rt_uint8_t *in_key_a, rt_uint8_t *in_key_b);
rt_bool_t rfid_card_reset(card_base_type_t type, rt_uint8_t *in_key_b);
rt_bool_t rfid_card_write(card_base_type_t type, rt_uint8_t *in_key_a, rt_uint8_t *in_key_b, rt_uint8_t *buffer, rt_uint16_t buf_length);
rt_bool_t rfic_money_read(rt_uint8_t *in_key_a, rt_bool_t *out_stat, rt_uint32_t *out_value);
rt_bool_t rfic_money_write(rt_uint8_t *in_key_b, rt_int32_t value);

#endif
