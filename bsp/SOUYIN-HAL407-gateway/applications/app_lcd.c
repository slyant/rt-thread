#include <rtthread.h>
#include <uart_lcd_process.h>
#include <hmi_driver.h>
#include <app_config.h>

const char SYS_ERR[] = {0xCF, 0xB5, 0xCD, 0xB3, 0xB4, 0xED, 0xCE, 0xF3, 0x3A, 0x00};		//系统错误:
const char MSG_RESULT[] = {0xB2, 0xD9, 0xD7, 0xF7, 0xBD, 0xE1, 0xB9, 0xFB, 0x00};	        //操作结果
const char MSG_CARD[] = {0xC7, 0xEB, 0xCB, 0xA2, 0xBF, 0xA8, 0x00};	                        //请刷卡
const char MSG_SUCCESS[] = {0xB2, 0xD9, 0xD7, 0xF7, 0xB3, 0xC9, 0xB9, 0xA6, 0x21, 0x00};	//操作成功!
const char MSG_FAILED[] =  {0xB2, 0xD9, 0xD7, 0xF7, 0xCA, 0xA7, 0xB0, 0xDC, 0x21, 0x00};	//操作失败!

static rt_uint16_t screen_id_list[5];

static temp_card_info_t temp_card_info = RT_NULL;

void lcd_wakeup(void)
{
	SetBackLight(LCD_BACK_LIGHT);
}

static void auto_temp_card_info(void)
{
	if(temp_card_info != RT_NULL)
	{
		rt_memset(temp_card_info, 0x00, sizeof(struct temp_card_info));
		temp_card_info->num = cardinfo_get_max_num();
		temp_card_info->num++;
		get_rnd_string(16, temp_card_info->pwd);
	}
}
static void init_temp_card_info(void)
{
	if(temp_card_info == RT_NULL)
	{
		temp_card_info = rt_calloc(1, sizeof(struct temp_card_info));
	}
}
static void dispose_temp_card_info(void)
{
	if(temp_card_info != RT_NULL)
	{
		rt_free(temp_card_info);
		temp_card_info = RT_NULL;
	}
}

void lcd_set_datetime(int year, int month, int mday, int wday, int hour, int min, int sec)
{
	SetRtc(year, month, mday, wday,	hour, min, sec);
}

void lcd_set_screen_id(rt_uint16_t id)
{
	SetScreen(id);
    screen_id_list[4] = screen_id_list[3];
	screen_id_list[3] = screen_id_list[2];
	screen_id_list[2] = screen_id_list[1];
	screen_id_list[1] = screen_id_list[0];
	screen_id_list[0] = id;
}

rt_uint16_t lcd_get_screen_id(void)
{
	return screen_id_list[0];
}

void lcd_set_screen_back(void)
{
	SetScreen(screen_id_list[1]);
	screen_id_list[0] = screen_id_list[1];
	screen_id_list[1] = screen_id_list[2];
	screen_id_list[2] = screen_id_list[3];
	screen_id_list[3] = screen_id_list[4];
	screen_id_list[4] = UI_MAIN;	
}

void lcd_show_error(const char *err)
{
	GUI_CleanScreen();
	SetFcolor(COLOR_WHITE);
    lcd_set_screen_id(UI_ERROR);
	show_string(UI_ERROR, 5, 10, 0, 6, (unsigned char *)SYS_ERR);
    show_string(UI_ERROR, 5, 50, 0, 6, (unsigned char *)err);
}

void lcd_show_message(const char *title, const char *msg)
{
    lcd_set_screen_id(UI_MESSAGE);
    SetTextValue(UI_MESSAGE, MESSAGE_TEXT_TITLE, (unsigned char *)title);
    SetTextValue(UI_MESSAGE, MESSAGE_TEXT_MSGBOX, (unsigned char *)msg);
}

//密钥卡设置
static void abkey_card_handle(unsigned short control_id)
{
    switch(control_id)
    {
    case ABKEY_CARD_BTN_INIT:		//密钥卡初始化
		if(init_card_key())
		{
			lcd_show_message(MSG_RESULT, MSG_SUCCESS);
		}
		else
		{
			lcd_show_message(MSG_RESULT, MSG_FAILED);
		}        
        break;
    case ABKEY_CARD_BTN_CREATE:		//密钥卡创建
		if(create_card_key())
		{
			lcd_show_message(MSG_RESULT, MSG_SUCCESS);
		}
		else
		{
			lcd_show_message(MSG_RESULT, MSG_FAILED);
		}
        break;
    case ABKEY_CARD_BTN_RESET:		//密钥卡重置
		if(reset_card_key())
		{
			lcd_show_message(MSG_RESULT, MSG_SUCCESS);
		}
		else
		{
			lcd_show_message(MSG_RESULT, MSG_FAILED);
		}
        break;
    case ABKEY_CARD_BTN_BACKUP:		//密钥卡备份
		if(backup_card_key())
		{
			lcd_show_message(MSG_RESULT, MSG_SUCCESS);
		}
		else
		{
			lcd_show_message(MSG_RESULT, MSG_FAILED);
		}		
        break;
    case ABKEY_CARD_BTN_IMPORT:		//密钥卡恢复
		if(restore_card_key())
		{
			lcd_show_message(MSG_RESULT, MSG_SUCCESS);
		}
		else
		{
			lcd_show_message(MSG_RESULT, MSG_FAILED);
		}		
        break;
    case ABKEY_CARD_BTN_SYS_RESET:	//系统重置
		if(sys_config.sys_reset())
		{
			sys_status.set_workmodel(WAITE_RESTART_MODEL);
			lcd_show_message(MSG_RESULT, MSG_SUCCESS);			
		}
		else
		{
			lcd_show_message(MSG_RESULT, MSG_FAILED);
		}		
        break;	
    case ABKEY_CARD_BTN_RESTART:	//重启
        sys_status.restart();
        break;
    default:
        break;
    }
}

//系统配置
static void sys_config_handle(unsigned short control_id)
{
	switch(control_id)
    {
    case SYS_CFG_BTN_ABKEY_CARD:	//密钥卡设置
		lcd_set_screen_id(UI_ABKEY_CARD);
		break;
	case SYS_CFG_BTN_CFG_CARD:		//配置卡设置
		init_temp_card_info();
		auto_temp_card_info();
		SetTextInt32(UI_CFG_CARD, CFG_CARD_TEXT_NUMBER, temp_card_info->num, 0, 0);
        lcd_set_screen_id(UI_CFG_CARD);		
		break;
	case SYS_CFG_BTN_POWER_CARD:	//授权卡设置
		break;
	case SYS_CFG_BTN_RESTART:			//退出重启
		sys_status.restart();
		break;
	default:
		break;
	}	
}	
//配置卡设置
static void cfg_card_handle(unsigned short control_id)
{
	switch(control_id)
    {
    case CFG_CARD_BTN_INIT:         //初始化
		if(init_card_app(RT_FALSE))
		{
			lcd_show_message(MSG_RESULT, MSG_SUCCESS);
		}
		else
		{
			lcd_show_message(MSG_RESULT, MSG_FAILED);
		}        
		break;
	case CFG_CARD_BTN_RESET:		//重置
		if(reset_card_app())
		{
			lcd_show_message(MSG_RESULT, MSG_SUCCESS);
		}
		else
		{
			lcd_show_message(MSG_RESULT, MSG_FAILED);
		}        
		break;
	case CFG_CARD_BTN_CLEAR:	    //清空卡记录
		break;
	case CFG_CARD_BTN_CREATE:	    //制卡		
		if(cardinfo_count_by_type(CARD_APP_TYPE_CONFIG) < CONFIG_CARD_MAX_COUNT)
		{
			if(create_card_config(temp_card_info->num, temp_card_info->pwd))
			{
				temp_card_info->num++;
				SetTextInt32(UI_CFG_CARD, CFG_CARD_TEXT_NUMBER, temp_card_info->num, 0, 0);
				lcd_show_message(MSG_RESULT, MSG_SUCCESS);
				return;
			}
		}
		lcd_show_message(MSG_RESULT, MSG_FAILED);
		break;    
	case CFG_CARD_BTN_BACK:			//返回
		dispose_temp_card_info();
		lcd_set_screen_back();
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
    case UI_ABKEY_CARD:		//密钥卡设置
        abkey_card_handle(control_id);
        break;
	case UI_SYS_CFG:		//系统配置
		sys_config_handle(control_id);
		break;
    case UI_CFG_CARD:
        cfg_card_handle(control_id);
        break;
    case UI_MESSAGE:		//提示信息
        if(control_id == MESSAGE_BTN_BACK)
		{			
			if(sys_status.get_workmodel() == WAITE_RESTART_MODEL)
			{
				sys_status.restart();
			}
			else
			{
				lcd_set_screen_back();
			}
		}
        break;
    default:
        break;
    }
}

static void lcd_notify_text(unsigned short screen_id, unsigned short control_id, void *params)
{
    unsigned char *str = (unsigned char *)params;
	switch(screen_id)
	{
	case UI_CFG_CARD:
		if(control_id == CFG_CARD_TEXT_NUMBER)
		{
			temp_card_info->num = str2int32(str);
		}
		break;
	default:
		break;
	}
}

void app_lcd_startup(void)
{	
	uart_lcd_set_button_notify_hook(lcd_notify_button);
    uart_lcd_set_text_notify_hook(lcd_notify_text);
}
