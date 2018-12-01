

#ifndef _LCD_OPER_
#define _LCD_OPER_

#include <rtthread.h>
#include "cmd_process.h"

//flash存储地址定义(共占用20KB/256KB，10个PAGE*2KB/PAGE)
#define data_base_addr	0x0803B000L
#define data_end_addr	0x0803FFFFL
#define sys_config_addr					(data_base_addr+FLASH_PAGE_SIZE*0)
#define sys_config_addr2				(data_base_addr+FLASH_PAGE_SIZE*1)

/*************************  UI界面 ID*********************/
#define MAIN_INDEX      0     //主界面
#define SYS_CFG_INDEX   1     //系统配置
#define SYS_TIME_SETUP  2     //系统时间设置（RTC）
#define CARD_MANAG      3
#define SARK_MANAGE     4     //银柜管理
#define DOOR_OPEN_MANA  5     //柜门开门管理
#define TEST_UI         6     //测试
/*************************  卡管理映射UI-->ID**************/
#define SYS_KEY_CRCF    7     //密钥卡管理
#define MANA_CARD_SET   8     //配置卡设置
#define PRIV_CARD_SET   9     //特权卡设置
#define NORM_CARD_SET   10    //普通卡设置
/*************************  UI界面 ID**********************/


//#define SYS_CFG_DATETIME_INDEX          3
//#define SYS_CFG_CLOSETIMEOUT_INDEX      4
//#define SYS_CFG_ADDRESS_INDEX           5
//#define SYS_CFG_CARD0_INDEX             6
//#define SYS_CFG_CARD1_INDEX             7
//#define OPEN_CARD_INDEX                 8
//#define MESSAGE_INDEX                   9

#define  MAX_SARK    8
typedef struct
{
	uint8_t  node;    //银柜节点（地址）
	uint8_t  num;     //柜门数量
	uint8_t  ov_time; //开门超时限定
	char     name[5]; //银柜名称 
}SARK_MSG,*sark_msg_t;

typedef struct
{
	uint8_t  cofg;
	uint8_t  del ;
	uint8_t  item;
	uint8_t  lins;
}SARK_POINT;



void ProcessMessage( PCTRL_MSG msg, uint16_t size );
static void NotifyScreen(unsigned short screen_id);
static void NotifyButton(unsigned short screen_id, unsigned short control_id, unsigned char  state);
static void make_config_card_handle(unsigned short screen_id, unsigned short control_id, unsigned char  state);
static void NotifyText(unsigned short screen_id, unsigned short control_id, unsigned char *str);
static void NotifyProgress(unsigned short screen_id, unsigned short control_id, unsigned long value);
static void NotifyMeter(unsigned short screen_id, unsigned short control_id, unsigned long value);
static void NotifyMenu(unsigned short screen_id, unsigned short control_id, unsigned char  item, unsigned char  state);
static void NotifySelector(unsigned short screen_id, unsigned short control_id, unsigned char  item);
static void NotifyTimer(unsigned short screen_id, unsigned short control_id);
static void NotifySlider(unsigned short screen_id, unsigned short control_id, unsigned long value);
static void NotifyTouchXY(unsigned char press,unsigned short x,unsigned short y);
static void NotifyWriteFlash(unsigned char status);
static void NotifyReadFlash(unsigned char status,unsigned char *_data,unsigned short length);

void sark_msg_init(void);
void sark_join_cfg(uint8_t node_ad);
void sark_discon_remov(uint8_t addr);

void UpdateUI(void);


#endif

