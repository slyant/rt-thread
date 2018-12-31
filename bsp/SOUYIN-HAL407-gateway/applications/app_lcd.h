#ifndef __APP_LCD_H__
#define __APP_LCD_H__

/*************************  UI界面 ID  *************************/
#define UI_MAIN				0       //主界面
#define UI_SYS_CFG        	1       //系统配置
#define UI_SYS_DATETIME		2       //系统时间设置（RTC）
#define UI_ABKEY_CARD      	3       //密钥卡设置
#define UI_CFG_CARD			4       //配置卡设置
#define UI_POWER_CARD		5       //授权卡设置
#define UI_KEY_CARD			6       //钥匙卡设置
#define UI_DRIVER_CARD		7       //司机卡设置
#define UI_LOCK_ABKEY_CARD	8       //锁钥卡设置
#define UI_OTHER_SETTING    9       //银柜管理
#define UI_OPEN_DOOR		10      //柜门开门管理
#define UI_ERROR			11      //错误信息
#define UI_MESSAGE			12      //信息提示
/****************************************************************/

/*************************  UI_SYS_CFG 控件ID  ******************/
#define SYS_CFG_BTN_ABKEY_CARD		6      //密钥卡设置
#define SYS_CFG_BTN_CFG_CARD		7      //配置卡设置
#define SYS_CFG_BTN_POWER_CARD     	8      //授权卡设置
#define SYS_CFG_BTN_KEY_CARD		10     //钥匙卡设置
#define SYS_CFG_BTN_DRIVER_CARD    	11     //司机卡设置
#define SYS_CFG_BTN_LOCK_ABKEY_CARD 12     //锁钥卡设置
#define SYS_CFG_BTN_SYS_DATETIME	13     //时间校准
#define SYS_CFG_BTN_OTHER_SETTING   14     //银柜设置
#define SYS_CFG_BTN_EXIT    		15     //退出
/****************************************************************/

/************************  UI_ABKEY_CARD 控件ID  ****************/
#define ABKEY_CARD_BTN_INIT    		31      //初始化
#define ABKEY_CARD_BTN_CREATE     	32      //制卡
#define ABKEY_CARD_BTN_RESET     	33      //重置
#define ABKEY_CARD_BTN_BACKUP     	34      //备份
#define ABKEY_CARD_BTN_IMPORT     	35      //恢复(导入)
#define ABKEY_CARD_BTN_RESTART     	36      //重启
/****************************************************************/

/***********************  UI_CFG_CARD 控件ID  *******************/
#define CFG_CARD_BTN_INIT    		43      //初始化
#define CFG_CARD_BTN_RESET     	    41      //重置
#define CFG_CARD_BTN_MANAGE     	42      //卡管理
#define CFG_CARD_TEXT_NUMBER        46      //卡编号
#define CFG_CARD_BTN_CREATE     	44      //制卡
#define CFG_CARD_BTN_BACK     	    45      //返回
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

#endif
