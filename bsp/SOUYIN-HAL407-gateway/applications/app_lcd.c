#include <rtthread.h>
#include <app_lcd.h>
#include <uart_lcd_process.h>
#include <hmi_driver.h>
#include <app_config.h>

const char SYS_ERR[] = {0xCF, 0xB5, 0xCD, 0xB3, 0xB4, 0xED, 0xCE, 0xF3, 0x3A, 0x00};		//系统错误:
const char MSG_SUCCESS[] = {0xB2, 0xD9, 0xD7, 0xF7, 0xB3, 0xC9, 0xB9, 0xA6, 0x21, 0x00};	//操作成功!
const char MSG_FAILED[] =  {0xB2, 0xD9, 0xD7, 0xF7, 0xCA, 0xA7, 0xB0, 0xDC, 0x21, 0x00};	//操作失败!

void lcd_set_screen(rt_uint16_t screen_id)
{
	SetScreen(screen_id);
    sys_status.last_screen_id = sys_status.screen_id;
    sys_status.screen_id = screen_id;
}

void lcd_show_error(const char *err)
{
	GUI_CleanScreen();
	SetFcolor(COLOR_WHITE);
    lcd_set_screen(UI_ERROR);
	show_string(UI_ERROR, 5, 10, 0, 6, (unsigned char *)SYS_ERR);
    show_string(UI_ERROR, 5, 50, 0, 6, (unsigned char *)err);
}

void lcd_show_message(const char *msg)
{
    lcd_set_screen(UI_MESSAGE);
    SetTextValue(UI_MESSAGE, MESSAGE_TEXT_MSGBOX, (unsigned char *)msg);
}

static void abkey_card_handle(unsigned short control_id)
{
    switch(control_id)
    {
    case ABKEY_CARD_BTN_INIT:
        lcd_show_message(MSG_SUCCESS);
        break;
    case ABKEY_CARD_BTN_CREATE:
        break;
    case ABKEY_CARD_BTN_RESET:
        break;
    case ABKEY_CARD_BTN_BACKUP:
        break;
    case ABKEY_CARD_BTN_IMPORT:
        break;
    case ABKEY_CARD_BTN_BACK:
        lcd_set_screen(sys_status.last_screen_id);
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
            lcd_set_screen(sys_status.last_screen_id);
        break;
    default:
        break;
    }
}

static void lcd_notify_text(unsigned short screen_id, unsigned short control_id, void *params)
{
//    unsigned char *str = (unsigned char *)params;
}

void uart_lcd_startup(void)
{	
	uart_lcd_reg_button_notify_handle(lcd_notify_button);
    uart_lcd_reg_text_notify_handle(lcd_notify_text);
}
