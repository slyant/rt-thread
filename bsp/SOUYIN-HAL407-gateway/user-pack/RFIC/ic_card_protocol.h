#ifndef __IC_CARD_PROTOCOL_H__
#define __IC_CARD_PROTOCOL_H__

#define ROOT_CARD  0
#define SYS_CARD   1

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
/******************数据块地址定义**********************/
#define CARD_APP_CHECK_BLOCK			1	//卡签名数据块
#define CARD_APP_LEN_BLOCK				2	//卡信息长度数据块
#define CARD_APP_CHECK_CTRL_BLOCK	    3	//卡签名控制数据块

void creat_sys_key(void) ;

unsigned char card_check(void);
unsigned char card_init(uint8_t typ);
unsigned char card_reset(uint8_t typ);
unsigned char write_inf_card(uint8_t typ , unsigned char* dat,unsigned short length,unsigned char* out_card_id);
unsigned char rfid_scan_handle(unsigned char *card_id, unsigned char *outInf, unsigned short* outLength);

#endif
