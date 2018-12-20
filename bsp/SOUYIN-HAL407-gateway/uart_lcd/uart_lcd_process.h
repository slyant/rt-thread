#ifndef __CMD_PROCESS_H__
#define __CMD_PROCESS_H__

#include <uart_lcd_device.h>

/*************************  UI界面 ID   *********************/
#define MAIN_INDEX      0     //主界面
#define SYS_CFG_INDEX   1     //系统配置
#define SYS_TIME_SETUP  2     //系统时间设置（RTC）
#define CARD_MANAG      3     //卡管理
#define SARK_MANAGE     4     //银柜管理
#define DOOR_OPEN_MANA  5     //柜门开门管理
#define TEST_UI         6     //测试
#define KEY_CARD_COF    7     //密钥卡管理
/*************************  卡管理映射UI-->ID  **************/
#define COF_CARD_SET   8      //配置卡设置
#define MANA_CARD_SET   9     //管理卡设置
#define NORM_CARD_SET   10    //普通卡设置
/*************************  UI界面 ID  **********************/

#define NOTIFY_TOUCH_PRESS      0X01   //触摸屏按下通知
#define NOTIFY_TOUCH_RELEASE  0X03  //触摸屏松开通知
#define NOTIFY_WRITE_FLASH_OK  0X0C  //写FLASH成功
#define NOTIFY_WRITE_FLASH_FAILD  0X0D  //写FLASH失败
#define NOTIFY_READ_FLASH_OK  0X0B  //读FLASH成功
#define NOTIFY_READ_FLASH_FAILD  0X0F  //读FLASH失败
#define NOTIFY_MENU                        0X14  //菜单事件通知
#define NOTIFY_TIMER                       0X43  //定时器超时通知
#define NOTIFY_CONTROL                0XB1  //控件更新通知
#define NOTIFY_READ_RTC               0XF7  //读取RTC时间
#define MSG_GET_CURRENT_SCREEN 0X01//画面ID变化通知
#define MSG_GET_DATA                            0X11//控件数据通知

#define PTR2U16(PTR) ((((unsigned char *)(PTR))[0]<<8)|((unsigned char *)(PTR))[1])  //从缓冲区取16位数据
#define PTR2U32(PTR) ((((unsigned char *)(PTR))[0]<<24)|(((unsigned char *)(PTR))[1]<<16)|(((unsigned char *)(PTR))[2]<<8)|((unsigned char *)(PTR))[3])  //从缓冲区取32位数据

enum CtrlType
{
	kCtrlUnknown=0x0,
	kCtrlButton=0x10,  //按钮
	kCtrlText,  //文本
	kCtrlProgress,  //进度条
	kCtrlSlider,    //滑动条
	kCtrlMeter,  //仪表
	kCtrlDropList, //下拉列表
	kCtrlAnimation, //动画
	kCtrlRTC, //时间显示
	kCtrlGraph, //曲线图控件
	kCtrlTable, //表格控件
	kCtrlMenu,//菜单控件
	kCtrlSelector,//选择控件
	kCtrlQRCode,//二维码
};

typedef struct
{
	unsigned char    cmd_head;  //帧头
	unsigned char    cmd_type;  //命令类型(UPDATE_CONTROL)	
	unsigned char    ctrl_msg;   //CtrlMsgType-指示消息的类型
	unsigned char    screen_id_high;  //产生消息的画面ID
	unsigned char    screen_id_low;
	unsigned char    control_id_high;  //产生消息的控件ID
	unsigned char    control_id_low;
	unsigned char    control_type; //控件类型
	unsigned char    param[256];//可变长度参数，最多256个字节
	unsigned char    cmd_tail[4];   //帧尾
}CTRL_MSG,*PCTRL_MSG;

static void NotifyScreen(unsigned short screen_id);
static void NotifyTouchXY(unsigned char press,unsigned short x,unsigned short y);
static void NotifyButton(unsigned short screen_id, unsigned short control_id, unsigned char  state);
static void NotifyText(unsigned short screen_id, unsigned short control_id, unsigned char *str);
static void NotifyProgress(unsigned short screen_id, unsigned short control_id, unsigned long value);
static void NotifySlider(unsigned short screen_id, unsigned short control_id, unsigned long value);
static void NotifyMeter(unsigned short screen_id, unsigned short control_id, unsigned long value);
static void NotifyMenu(unsigned short screen_id, unsigned short control_id, unsigned char  item, unsigned char  state);
static void NotifySelector(unsigned short screen_id, unsigned short control_id, unsigned char  item);
static void NotifyTimer(unsigned short screen_id, unsigned short control_id);
static void NotifyReadFlash(unsigned char status,unsigned char *_data,unsigned short length);
static void NotifyWriteFlash(unsigned char status);
static void NotifyReadRTC(unsigned char year,unsigned char month,unsigned char week,unsigned char day,unsigned char hour,unsigned char minute,unsigned char second);
static void SetTextValueInt32(unsigned short screen_id, unsigned short control_id,long value);

static long StringToInt32(unsigned char *str);
static void send_lcd_update_event(void);

void lcd_device_reg(lcd_device_t device);
void lcd_load_default_screen(void);
void lcd_load_error_screen(char *msg);
void lcd_change_screen(rt_uint16_t screen_id);
void lcd_update_ui(void);
void ProcessMessage(PCTRL_MSG msg, unsigned short size);
#endif
