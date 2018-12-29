#include <rtthread.h>
#include <app_lcd.h>
#include <uart_lcd_process.h>
#include <hmi_driver.h>
#include <app_rfic.h>
#include <app_config.h>

const char SYS_ERR[] = {0xCF, 0xB5, 0xCD, 0xB3, 0xB4, 0xED, 0xCE, 0xF3, 0x3A, 0x00};		//系统错误:
const char MSG_RESULT[] = {0xB2, 0xD9, 0xD7, 0xF7, 0xBD, 0xE1, 0xB9, 0xFB, 0x00};	        //操作结果
const char MSG_CARD[] = {0xC7, 0xEB, 0xCB, 0xA2, 0xBF, 0xA8, 0x00};	                        //请刷卡
const char MSG_SUCCESS[] = {0xB2, 0xD9, 0xD7, 0xF7, 0xB3, 0xC9, 0xB9, 0xA6, 0x21, 0x00};	//操作成功!
const char MSG_FAILED[] =  {0xB2, 0xD9, 0xD7, 0xF7, 0xCA, 0xA7, 0xB0, 0xDC, 0x21, 0x00};	//操作失败!

void lcd_set_datetime(int year, int month, int mday, int wday, int hour, int min, int sec)
{
	SetRtc(year, month, mday, wday,	hour, min, sec);
}

void lcd_set_screen(rt_uint16_t screen_id)
{
	SetScreen(screen_id);
}

void lcd_show_error(const char *err)
{
	GUI_CleanScreen();
	SetFcolor(COLOR_WHITE);
    sys_status.set_screen_id(UI_ERROR);
	show_string(UI_ERROR, 5, 10, 0, 6, (unsigned char *)SYS_ERR);
    show_string(UI_ERROR, 5, 50, 0, 6, (unsigned char *)err);
}

void lcd_show_message(const char *title, const char *msg)
{
    sys_status.set_screen_id(UI_MESSAGE);
    SetTextValue(UI_MESSAGE, MESSAGE_TEXT_TITLE, (unsigned char *)title);
    SetTextValue(UI_MESSAGE, MESSAGE_TEXT_MSGBOX, (unsigned char *)msg);
}

static void abkey_card_handle(unsigned short control_id)
{
    switch(control_id)
    {
    case ABKEY_CARD_BTN_INIT:
		if(init_card_key())
		{
			lcd_show_message(MSG_RESULT, MSG_SUCCESS);
		}
		else
		{
			lcd_show_message(MSG_RESULT, MSG_FAILED);
		}        
        break;
    case ABKEY_CARD_BTN_CREATE:
		if(create_app_abkey())
		{
			lcd_show_message(MSG_RESULT, MSG_SUCCESS);
		}
		else
		{
			lcd_show_message(MSG_RESULT, MSG_FAILED);
		}
        break;
    case ABKEY_CARD_BTN_RESET:
		if(reset_card_key())
		{
			lcd_show_message(MSG_RESULT, MSG_SUCCESS);
		}
		else
		{
			lcd_show_message(MSG_RESULT, MSG_FAILED);
		}
        break;
    case ABKEY_CARD_BTN_BACKUP:
        break;
    case ABKEY_CARD_BTN_IMPORT:
        break;
    case ABKEY_CARD_BTN_BACK:
        sys_status.restart();
        break;
    default:
        break;
    }
}

static void lcd_notify_button(unsigned short screen_id, unsigned short control_id, void *params)
{
    //unsigned char state = (unsigned char)params;
    switch(screen_id)
    {
    case UI_ABKEY_CARD:
        abkey_card_handle(control_id);
        break;
    case UI_MESSAGE:
        if(control_id == MESSAGE_BTN_BACK)
            sys_status.set_screen_back();
        break;
    default:
        break;
    }
}

static void lcd_notify_text(unsigned short screen_id, unsigned short control_id, void *params)
{
//    unsigned char *str = (unsigned char *)params;
}

void app_lcd_startup(void)
{	
	uart_lcd_set_button_notify_hook(lcd_notify_button);
    uart_lcd_set_text_notify_hook(lcd_notify_text);
}
