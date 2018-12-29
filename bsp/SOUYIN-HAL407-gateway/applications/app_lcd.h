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
#define SYS_CFG_BTN_CFG_CARD		11      //配置卡设置
#define SYS_CFG_BTN_POWER_CARD     	12      //授权卡设置
#define SYS_CFG_BTN_KEY_CARD		13      //钥匙卡设置
#define SYS_CFG_BTN_DRIVER_CARD    	14      //司机卡设置
#define SYS_CFG_BTN_LOCK_ABKEY_CARD 15      //锁钥卡设置
#define SYS_CFG_BTN_SYS_DATETIME	16      //时间校准
#define SYS_CFG_BTN_OTHER_SETTING   17      //银柜设置
#define SYS_CFG_BTN_EXIT    		18      //退出
/****************************************************************/

/*************************  UI_ABKEY_CARD 控件ID  ******************/
#define ABKEY_CARD_BTN_INIT    		31      //初始化
#define ABKEY_CARD_BTN_CREATE     	32      //制卡
#define ABKEY_CARD_BTN_RESET     	33      //重置
#define ABKEY_CARD_BTN_BACKUP     	34      //备份
#define ABKEY_CARD_BTN_IMPORT     	35      //恢复(导入)
#define ABKEY_CARD_BTN_BACK     	36      //返回
/****************************************************************/

/*************************  UI_MESSAGE 控件ID  ******************/
#define MESSAGE_BTN_BACK    		6      //返回
#define MESSAGE_TEXT_TITLE          1      //信息标题
#define MESSAGE_TEXT_MSGBOX     	4      //信息文本框
/****************************************************************/

void lcd_show_error(const char* err);
void lcd_show_message(const char *title, const char *msg);
void lcd_set_screen(rt_uint16_t screen_id);
void lcd_set_datetime(int year, int month, int mday, int wday, int hour, int min, int sec);

#endif
