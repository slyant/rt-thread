#ifndef __IC_CARD_PROTOCOL_H__
#define __IC_CARD_PROTOCOL_H__

/******************数据块地址定义**********************/
#define CARD_CHECK_BLOCK			1	//卡签名数据块
#define CARD_LEN_BLOCK				2	//卡信息长度数据块

typedef enum
{
	CARD_TYPE_NULL = 0,     //空卡
	CARD_TYPE_KEY,			//密钥卡
	CARD_TYPE_APP,			//应用卡
	CARD_TYPE_UNKNOW		//未知卡
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







#define APP_USE_SECTOR_SIZE  9

#define FALSE 0
#define TRUE  1

//卡应用类型
#define CardAppType_00		0		//空白卡
#define CardAppType_01		1		//信息卡
#define CardAppType_FD		253		//武汉公交员工卡(司机卡)
#define CardAppType_FE		254		//初始卡
#define CardAppType_FF		255		//未知卡
//信息卡类型
#define CardInfType_00		0		//身份卡
#define CardInfType_01		1		//司机卡
#define CardInfType_02		2		//车辆卡
#define CardInfType_03		3		//开启台卡
#define CardInfType_FF		255		//未知卡


void create_rnd_str(int length,char* out_str);

void creat_sys_key(void) ;
void creat_md5_val(uint8_t *keya, uint8_t *keyb, char *json, uint8_t *val);

uint8_t card_into_head(void);

unsigned char card_check(void);
unsigned char card_init(uint8_t typ);
unsigned char card_reset(uint8_t typ);
unsigned char write_inf_card(uint8_t typ , unsigned char* dat,unsigned short length);
uint8_t rfid_scan_handle(void);

#endif
