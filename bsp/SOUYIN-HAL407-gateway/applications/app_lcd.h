#ifndef __APP_LCD_H__
#define __APP_LCD_H__

#include <rtthread.h>
#include <app_config.h>

#define LCD_BACK_LIGHT		30		//LCD背光亮度
#define TITLE_SIZE			61

struct temp_card
{
	int num;
	char pwd[17];
};
typedef struct temp_card *temp_card_t;

struct temp_setting
{
	char sys_title[TITLE_SIZE];
	rt_uint8_t en_driver_card;
	rt_uint8_t node_count;
	rt_uint8_t door_count;
	rt_uint16_t open_timeout;	
};
typedef struct temp_setting *temp_setting_t;

/*************************  UI界面 ID  *************************/
#define UI_MAIN				0       //主界面
#define UI_SYS_CFG        	1       //系统配置
#define UI_SYS_DATETIME		2       //系统时间设置（RTC）
#define UI_ABKEY_CARD      	3       //密钥卡设置
#define UI_CFG_CARD			4       //配置卡设置
#define UI_POWER_CARD		5       //授权卡设置
#define UI_EKEY_CARD		6       //钥匙卡设置
#define UI_DRIVER_CARD		7       //司机卡设置
#define UI_LOCK_ABKEY_CARD	8       //锁钥卡设置
#define UI_OTHER_SETTING    9       //其它设置
#define UI_OPEN_DOOR		10      //柜门开门管理
#define UI_ERROR			11      //错误信息
#define UI_MESSAGE			12      //信息提示
#define UI_WAITE			13		//请稍候
/****************************************************************/

/*************************  UI_MAIN 控件ID  ******************/
#define MAIN_TEXT_TITLE		3      //系统标题
/****************************************************************/

/*************************  UI_SYS_CFG 控件ID  ******************/
#define SYS_CFG_BTN_ABKEY_CARD		6      //密钥卡设置
#define SYS_CFG_BTN_CFG_CARD		7      //配置卡设置
#define SYS_CFG_BTN_POWER_CARD     	8      //授权卡设置
#define SYS_CFG_BTN_KEY_CARD		10     //钥匙卡设置
#define SYS_CFG_BTN_DRIVER_CARD    	11     //司机卡设置
#define SYS_CFG_BTN_LOCK_ABKEY_CARD 12     //锁钥卡设置
#define SYS_CFG_BTN_SYS_DATETIME	13     //时间校准
#define SYS_CFG_BTN_OTHER_SETTING   14     //其它设置
#define SYS_CFG_BTN_RESTART    		15     //重启
/****************************************************************/

/************************  UI_ABKEY_CARD 控件ID  ****************/
#define ABKEY_CARD_BTN_INIT    		31      //初始化
#define ABKEY_CARD_BTN_CREATE     	32      //制卡
#define ABKEY_CARD_BTN_RESET     	33      //重置
#define ABKEY_CARD_BTN_BACKUP     	34      //备份
#define ABKEY_CARD_BTN_IMPORT     	35      //还原
#define ABKEY_CARD_BTN_SYS_RESET	6		//系统重置
#define ABKEY_CARD_BTN_RESTART     	36      //重启
/****************************************************************/

/***********************  UI_CFG_CARD 控件ID  *******************/
#define CFG_CARD_BTN_INIT    		43      //初始化
#define CFG_CARD_BTN_RESET     	    41      //重置
#define CFG_CARD_BTN_CLEAR	     	42      //清空卡记录
#define CFG_CARD_TEXT_NUMBER        46      //卡编号
#define CFG_CARD_BTN_CREATE     	44      //制卡
#define CFG_CARD_BTN_BACK     	    45      //返回
/****************************************************************/

/***********************  UI_POWER_CARD 控件ID  *****************/
#define POWER_CARD_BTN_INIT    		15      //初始化
#define POWER_CARD_BTN_RESET     	13      //重置
#define POWER_CARD_BTN_CLEAR	    14      //清空卡记录
#define POWER_CARD_TEXT_NUMBER      2      	//卡编号
#define POWER_CARD_BTN_CREATE     	16      //制卡
#define POWER_CARD_BTN_BACK     	17      //返回
/****************************************************************/

/***********************  UI_EKEY_CARD 控件ID  *******************/
#define EKEY_CARD_BTN_INIT    		15      //初始化
#define EKEY_CARD_BTN_RESET     	13      //重置
#define EKEY_CARD_BTN_CLEAR	    	14      //清空卡记录
#define EKEY_CARD_TEXT_NUMBER      	2      	//卡编号
#define EKEY_CARD_BTN_CREATE     	16      //制卡
#define EKEY_CARD_BTN_BACK     		17      //返回
/****************************************************************/

/***********************  UI_DRIVER_CARD 控件ID  ****************/
#define DRIVER_CARD_BTN_INIT    	15      //初始化
#define DRIVER_CARD_BTN_RESET     	13      //重置
#define DRIVER_CARD_BTN_CLEAR	    14      //清空卡记录
#define DRIVER_CARD_TEXT_NUMBER     2      	//卡编号
#define DRIVER_CARD_BTN_CREATE     	16      //制卡
#define DRIVER_CARD_BTN_BACK     	17      //返回
/****************************************************************/

/*********************  UI_LOCK_ABKEY_CARD 控件ID  **************/
#define LOCK_ABKEY_CARD_BTN_INIT    	15      //初始化
#define LOCK_ABKEY_CARD_BTN_RESET     	13      //重置
#define LOCK_ABKEY_CARD_BTN_CLEAR	    14      //清空卡记录
#define LOCK_ABKEY_CARD_TEXT_NUMBER     2      	//卡编号
#define LOCK_ABKEY_CARD_BTN_CREATE     	16      //制卡
#define LOCK_ABKEY_CARD_BTN_BACK     	17      //返回
/****************************************************************/

/*********************  UI_OTHER_SETTING 控件ID  **************/
#define OTHER_SETTING_TEXT_SYS_TITLE   		96      //系统标题
#define OTHER_SETTING_TEXT_NODE_COUNT  		91      //节点数量
#define OTHER_SETTING_TEXT_DOOR_COUNT  		92      //柜门数量
#define OTHER_SETTING_TEXT_OPEN_TIMEROUT	93      //开门超时时间
#define OTHER_SETTING_TEXT_EN_DRIVER_CARD	3		//启用司机卡
#define OTHER_SETTING_BTN_ENTER    			10      //确定
#define OTHER_SETTING_BTN_CANCEL     		11      //取消
/****************************************************************/

/*********************  UI_OPEN_DOOR 控件ID  **************/
#define OPEN_DOOR_TEXT_SYS_TITLE   			1		//系统标题
#define OPEN_DOOR_BTN_ALL_GROUP				100		//所有组
#define OPEN_DOOR_BTN_A_GROUP				101		//A组
#define OPEN_DOOR_BTN_H_GROUP				108		//H组
#define OPEN_DOOR_BTN_EXIT					109		//退出
#define OPEN_DOOR_BTN_1						901		//1
#define OPEN_DOOR_BTN_16					916		//16
/****************************************************************/

/*************************  UI_MESSAGE 控件ID  ******************/
#define MESSAGE_BTN_BACK    		6      //返回
#define MESSAGE_TEXT_TITLE          1      //信息标题
#define MESSAGE_TEXT_MSGBOX     	4      //信息文本框
/****************************************************************/

void lcd_show_error(const char* err);
void lcd_show_message(const char *title, const char *msg);
void lcd_set_screen_id(rt_uint16_t screen_id);
rt_uint16_t lcd_get_screen_id(void);
void lcd_set_screen_back(void);
void lcd_set_datetime(int year, int month, int mday, int wday, int hour, int min, int sec);
void lcd_set_sys_title(char *title);
void lcd_wakeup(void);
void lcd_set_open_door(void);
void lcd_update_door_sta(rt_uint8_t group_index, rt_uint16_t sta);

#endif
