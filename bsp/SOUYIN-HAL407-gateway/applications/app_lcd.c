#include <rtthread.h>
#include <uart_lcd_process.h>
#include <hmi_driver.h>
#include <app_config.h>

static const char SYS_ERR[] = {0xCF, 0xB5, 0xCD, 0xB3, 0xB4, 0xED, 0xCE, 0xF3, 0x3A, 0x00};		//系统错误:
static const char MSG_RESULT[] = {0xB2, 0xD9, 0xD7, 0xF7, 0xBD, 0xE1, 0xB9, 0xFB, 0x00};	    //操作结果
static const char MSG_SUCCESS[] = {0xB2, 0xD9, 0xD7, 0xF7, 0xB3, 0xC9, 0xB9, 0xA6, 0x21, 0x00};	//操作成功!
static const char MSG_FAILED[] =  {0xB2, 0xD9, 0xD7, 0xF7, 0xCA, 0xA7, 0xB0, 0xDC, 0x21, 0x00};	//操作失败!
static const char MSG_OUT_FAILED[] = {0xB2, 0xD9, 0xD7, 0xF7, 0xCA, 0xA7, 0xB0, 0xDC, 0xA3, 0xAC, 0xD6, 0xC6, 0xBF, 0xA8, 0xCA, 0xFD, 
									  0xC1, 0xBF, 0xB3, 0xAC, 0xB3, 0xF6, 0xCF, 0xDE, 0xD6, 0xC6, 0xA3, 0xA1, 0x00};//操作失败，制卡数量超出限制！
                                   
static rt_uint16_t screen_id_list[5];
static rt_uint8_t btn_all_group_sta;
static rt_uint8_t btn_gropu_index;								  

static temp_card_t temp_card_info = RT_NULL;
static temp_setting_t temp_setting_info = RT_NULL;									  

void lcd_set_open_door(void)
{
	rt_uint8_t i, j;
	int control_index;
	
	BatchBegin(UI_OPEN_DOOR);
    //设置分组控制按钮初始状态
    BatchSetButtonValue(OPEN_DOOR_BTN_ALL_GROUP, btn_all_group_sta);
	//设置组按钮可见
	control_index = OPEN_DOOR_BTN_A_GROUP;
	for(i = 0; i < sys_config.node_count; i++)
	{
		BatchSetVisible(control_index++, 1);
	}
	for(j = i; j < NODE_MAX_COUNT; j++)
	{
		BatchSetVisible(control_index++, 0);
	}
    //设置组按钮初始状态
	for(i = 0; i < sys_config.node_count; i++)
	{
        if(!btn_all_group_sta && i == btn_gropu_index)
        {
            BatchSetButtonValue(OPEN_DOOR_BTN_A_GROUP + i, 1);
        }
        else
        {
            BatchSetButtonValue(OPEN_DOOR_BTN_A_GROUP + i, 0);
        }		
	}    
	//设置门按钮可见
	control_index = OPEN_DOOR_BTN_1;
	for(i = 0; i < sys_config.door_count; i++)
	{
		BatchSetVisible(control_index++, !btn_all_group_sta);
	}
	for(j = i; j < DOOR_MAX_COUNT; j++)
	{
		BatchSetVisible(control_index++, 0);
	}
	BatchEnd();
}		

#define GET_DOOR_STA(sta,idx)   ((sta & (((rt_uint16_t)0x0001)<<(idx)))>0?1:0)

void lcd_update_door_sta(rt_uint8_t group_index)
{
    if(sys_status.get_workmodel() == WORK_MANAGE_MODEL && ! btn_all_group_sta && group_index == btn_gropu_index)
    {
        rt_uint16_t sta = sys_status.get_door_group_sta(group_index);
        BatchBegin(UI_OPEN_DOOR);
        for(int i = OPEN_DOOR_BTN_1; i < OPEN_DOOR_BTN_1 + sys_config.door_count; i++)
        {                
            BatchSetButtonValue(i, GET_DOOR_STA(sta, i - OPEN_DOOR_BTN_1));
        }
        BatchEnd();
    }
}
									  
void lcd_wakeup(void)
{
	SetBackLight(LCD_BACK_LIGHT);
}

static void auto_temp_card_info(void)
{
	if(temp_card_info != RT_NULL)
	{
		rt_memset(temp_card_info, 0x00, sizeof(struct temp_card));
		temp_card_info->num = cardinfo_get_max_num();
		temp_card_info->num++;
		get_rnd_string(16, temp_card_info->pwd);
	}
}
static void auto_temp_setting_info(void)
{
	if(temp_setting_info != RT_NULL)
	{
		rt_memset(temp_setting_info, 0x00, sizeof(struct temp_setting));
		sysinfo_t sysinfo = rt_calloc(1, sizeof(struct sysinfo));
		if(sysinfo_get_by_id(sysinfo, SYSINFO_KEY_ID) > 0)
		{
			rt_strncpy(temp_setting_info->sys_title, sysinfo->sys_title, rt_strlen(sysinfo->sys_title));	
			temp_setting_info->en_driver_card = sysinfo->en_driver_card;
			temp_setting_info->node_count = sysinfo->node_count;
			temp_setting_info->door_count = sysinfo->door_count;
			temp_setting_info->open_timeout = sysinfo->open_timeout;
		}
		rt_free(sysinfo);
	}
}
static void init_temp_card_info(void)
{
	if(temp_card_info == RT_NULL)
	{
		temp_card_info = rt_calloc(1, sizeof(struct temp_card));
	}
}
static void init_temp_stting_info(void)
{
	if(temp_setting_info == RT_NULL)
	{
		temp_setting_info = rt_calloc(1, sizeof(struct temp_setting));
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
static void dispose_temp_stting_info(void)
{
	if(temp_setting_info != RT_NULL)
	{
		rt_free(temp_setting_info);
		temp_setting_info = RT_NULL;
	}
}
void lcd_set_datetime(int year, int month, int mday, int wday, int hour, int min, int sec)
{
	SetRtc(year, month, mday, wday,	hour, min, sec);
}

void lcd_set_sys_title(char *title)
{
	SetTextValue(UI_MAIN, MAIN_TEXT_TITLE, (unsigned char*)title);
	SetTextValue(UI_OPEN_DOOR, OPEN_DOOR_TEXT_SYS_TITLE, (unsigned char*)title);
}

void lcd_set_screen_id(rt_uint16_t id)
{
	SetScreen(id);
	if(id == UI_WAITE) return;
	
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
		lcd_set_screen_id(UI_WAITE);
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
		lcd_set_screen_id(UI_WAITE);
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
		lcd_set_screen_id(UI_WAITE);
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
		lcd_set_screen_id(UI_WAITE);
		if(backup_card_key())
		{
			lcd_show_message(MSG_RESULT, MSG_SUCCESS);
		}
		else
		{
			lcd_show_message(MSG_RESULT, MSG_FAILED);
		}		
        break;
    case ABKEY_CARD_BTN_IMPORT:		//密钥卡还原
		lcd_set_screen_id(UI_WAITE);
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
		lcd_set_screen_id(UI_WAITE);
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
		init_temp_card_info();
		auto_temp_card_info();
		SetTextInt32(UI_POWER_CARD, POWER_CARD_TEXT_NUMBER, temp_card_info->num, 0, 0);
        lcd_set_screen_id(UI_POWER_CARD);			
		break;
	case SYS_CFG_BTN_KEY_CARD:		//钥匙卡设置
		init_temp_card_info();
		auto_temp_card_info();
		SetTextInt32(UI_EKEY_CARD, EKEY_CARD_TEXT_NUMBER, temp_card_info->num, 0, 0);
        lcd_set_screen_id(UI_EKEY_CARD);			
		break;	
	case SYS_CFG_BTN_DRIVER_CARD:	//司机卡设置
		init_temp_card_info();
		auto_temp_card_info();
		SetTextInt32(UI_DRIVER_CARD, DRIVER_CARD_TEXT_NUMBER, temp_card_info->num, 0, 0);
        lcd_set_screen_id(UI_DRIVER_CARD);
		break;	
	case SYS_CFG_BTN_OTHER_SETTING:	//其它设置
		init_temp_stting_info();
		auto_temp_setting_info();
		SetTextValue(UI_OTHER_SETTING, OTHER_SETTING_TEXT_SYS_TITLE, (unsigned char*)temp_setting_info->sys_title);
		SetTextInt32(UI_OTHER_SETTING, OTHER_SETTING_TEXT_EN_DRIVER_CARD, temp_setting_info->en_driver_card, 0, 0);
		SetTextInt32(UI_OTHER_SETTING, OTHER_SETTING_TEXT_NODE_COUNT, temp_setting_info->node_count, 0, 0);
		SetTextInt32(UI_OTHER_SETTING, OTHER_SETTING_TEXT_DOOR_COUNT, temp_setting_info->door_count, 0, 0);
		SetTextInt32(UI_OTHER_SETTING, OTHER_SETTING_TEXT_OPEN_TIMEROUT, temp_setting_info->open_timeout, 0, 0);
		lcd_set_screen_id(UI_OTHER_SETTING);
		break;
	case SYS_CFG_BTN_RESTART:		//退出重启
        nrfreset();
        rt_thread_mdelay(100);
		sys_status.restart();
		break;
    case SYS_CFG_BTN_PAIR:          //节点配对
        nrf_send_net_pwd();
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
		lcd_set_screen_id(UI_WAITE);
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
		lcd_set_screen_id(UI_WAITE);
		if(reset_card_app())
		{
			lcd_show_message(MSG_RESULT, MSG_SUCCESS);
		}
		else
		{
			lcd_show_message(MSG_RESULT, MSG_FAILED);
		}        
		break;
	case CFG_CARD_BTN_CLEAR:	    //清空配置卡记录
		lcd_set_screen_id(UI_WAITE);
		if(cardinfo_del_by_type(CARD_APP_TYPE_CONFIG) == 0)
		{
			sys_status.set_workmodel(WAITE_RESTART_MODEL);
			lcd_show_message(MSG_RESULT, MSG_SUCCESS);			
		}
		else
		{
			lcd_show_message(MSG_RESULT, MSG_FAILED);
		}
		break;
	case CFG_CARD_BTN_CREATE:	    //制卡		
		lcd_set_screen_id(UI_WAITE);
		if(cardinfo_count_by_type(CARD_APP_TYPE_CONFIG) < CONFIG_CARD_MAX_COUNT)
		{
			if(create_card_app(CARD_APP_TYPE_CONFIG, temp_card_info->num, temp_card_info->pwd))
			{
				lcd_show_message(MSG_RESULT, MSG_SUCCESS);
			}
			else
			{
				lcd_show_message(MSG_RESULT, MSG_FAILED);
			}
		}
		else
		{
			lcd_show_message(MSG_RESULT, MSG_OUT_FAILED);
		}
		auto_temp_card_info();
		SetTextInt32(UI_CFG_CARD, CFG_CARD_TEXT_NUMBER, temp_card_info->num, 0, 0);
		break;    
	case CFG_CARD_BTN_BACK:			//返回
		dispose_temp_card_info();
		lcd_set_screen_back();
		break;
	default:
		break;
	}	
}
//授权卡设置
static void power_card_handle(unsigned short control_id)
{
	switch(control_id)
    {
    case POWER_CARD_BTN_INIT:         //初始化
		lcd_set_screen_id(UI_WAITE);
		if(init_card_app(RT_FALSE))
		{
			lcd_show_message(MSG_RESULT, MSG_SUCCESS);
		}
		else
		{
			lcd_show_message(MSG_RESULT, MSG_FAILED);
		}        
		break;
	case POWER_CARD_BTN_RESET:		//重置
		lcd_set_screen_id(UI_WAITE);
		if(reset_card_app())
		{
			lcd_show_message(MSG_RESULT, MSG_SUCCESS);
		}
		else
		{
			lcd_show_message(MSG_RESULT, MSG_FAILED);
		}        
		break;
	case POWER_CARD_BTN_CLEAR:	    //清空授权卡记录
		lcd_set_screen_id(UI_WAITE);
		if(cardinfo_del_by_type(CARD_APP_TYPE_POWER) == 0)
		{
			sys_status.set_workmodel(WAITE_RESTART_MODEL);
			lcd_show_message(MSG_RESULT, MSG_SUCCESS);			
		}
		else
		{
			lcd_show_message(MSG_RESULT, MSG_FAILED);
		}
		break;
	case POWER_CARD_BTN_CREATE:	    //制卡		
		lcd_set_screen_id(UI_WAITE);
		if(cardinfo_count_by_type(CARD_APP_TYPE_POWER) < POWER_CARD_MAX_COUNT)
		{
			if(create_card_app(CARD_APP_TYPE_POWER, temp_card_info->num, temp_card_info->pwd))
			{
				lcd_show_message(MSG_RESULT, MSG_SUCCESS);
			}
			else
			{
				lcd_show_message(MSG_RESULT, MSG_FAILED);
			}
		}
		else
		{
			lcd_show_message(MSG_RESULT, MSG_OUT_FAILED);
		}
		auto_temp_card_info();
		SetTextInt32(UI_POWER_CARD, POWER_CARD_TEXT_NUMBER, temp_card_info->num, 0, 0);
		break;    
	case POWER_CARD_BTN_BACK:		//返回
		dispose_temp_card_info();
		lcd_set_screen_back();
		break;
	default:
		break;
	}	
}
//钥匙卡设置
static void ekey_card_handle(unsigned short control_id)
{
	switch(control_id)
    {
    case EKEY_CARD_BTN_INIT:         //初始化
		lcd_set_screen_id(UI_WAITE);
		if(init_card_app(RT_FALSE))
		{
			lcd_show_message(MSG_RESULT, MSG_SUCCESS);
		}
		else
		{
			lcd_show_message(MSG_RESULT, MSG_FAILED);
		}        
		break;
	case EKEY_CARD_BTN_RESET:		//重置
		lcd_set_screen_id(UI_WAITE);
		if(reset_card_app())
		{
			lcd_show_message(MSG_RESULT, MSG_SUCCESS);
		}
		else
		{
			lcd_show_message(MSG_RESULT, MSG_FAILED);
		}        
		break;
	case EKEY_CARD_BTN_CLEAR:	    //清空钥匙卡记录
		lcd_set_screen_id(UI_WAITE);
		if(cardinfo_del_by_type(CARD_APP_TYPE_EKEY) == 0)
		{
			sys_status.set_workmodel(WAITE_RESTART_MODEL);
			lcd_show_message(MSG_RESULT, MSG_SUCCESS);			
		}
		else
		{
			lcd_show_message(MSG_RESULT, MSG_FAILED);
		}
		break;
	case EKEY_CARD_BTN_CREATE:	    //制卡		
		lcd_set_screen_id(UI_WAITE);
		if(cardinfo_count_by_type(CARD_APP_TYPE_EKEY) < EKEY_CARD_MAX_COUNT)
		{
			if(create_card_app(CARD_APP_TYPE_EKEY, temp_card_info->num, temp_card_info->pwd))
			{
				lcd_show_message(MSG_RESULT, MSG_SUCCESS);
			}
			else
			{
				lcd_show_message(MSG_RESULT, MSG_FAILED);
			}
		}
		else
		{
			lcd_show_message(MSG_RESULT, MSG_OUT_FAILED);
		}
		auto_temp_card_info();
		SetTextInt32(UI_EKEY_CARD, EKEY_CARD_TEXT_NUMBER, temp_card_info->num, 0, 0);
		break;    
	case EKEY_CARD_BTN_BACK:		//返回
		dispose_temp_card_info();
		lcd_set_screen_back();
		break;
	default:
		break;
	}	
}
//司机卡设置
static void driver_card_handle(unsigned short control_id)
{
	switch(control_id)
    {
    case DRIVER_CARD_BTN_INIT:			//初始化
		lcd_set_screen_id(UI_WAITE);
		if(init_card_app(sys_config.en_driver_card > 0 ? RT_TRUE:RT_FALSE))	//启用电子钱包
		{
			lcd_show_message(MSG_RESULT, MSG_SUCCESS);
		}
		else
		{
			lcd_show_message(MSG_RESULT, MSG_FAILED);
		}        
		break;
	case DRIVER_CARD_BTN_RESET:			//重置
		lcd_set_screen_id(UI_WAITE);
		if(reset_card_app())
		{
			lcd_show_message(MSG_RESULT, MSG_SUCCESS);
		}
		else
		{
			lcd_show_message(MSG_RESULT, MSG_FAILED);
		}        
		break;
	case DRIVER_CARD_BTN_CLEAR:	    	//清空司机卡记录
		lcd_set_screen_id(UI_WAITE);
		if(cardinfo_del_by_type(CARD_APP_TYPE_DRIVER) == 0)
		{
			sys_status.set_workmodel(WAITE_RESTART_MODEL);
			lcd_show_message(MSG_RESULT, MSG_SUCCESS);			
		}
		else
		{
			lcd_show_message(MSG_RESULT, MSG_FAILED);
		}
		break;
	case DRIVER_CARD_BTN_CREATE:	    //制卡		
		lcd_set_screen_id(UI_WAITE);
		if(cardinfo_count_by_type(CARD_APP_TYPE_DRIVER) < DRIVER_CARD_MAX_COUNT)
		{
			if(create_card_app(CARD_APP_TYPE_DRIVER, temp_card_info->num, temp_card_info->pwd))
			{
				lcd_show_message(MSG_RESULT, MSG_SUCCESS);
			}
			else
			{
				lcd_show_message(MSG_RESULT, MSG_FAILED);
			}
		}
		else
		{
			lcd_show_message(MSG_RESULT, MSG_OUT_FAILED);
		}
		auto_temp_card_info();
		SetTextInt32(UI_DRIVER_CARD, DRIVER_CARD_TEXT_NUMBER, temp_card_info->num, 0, 0);
		break;    
	case DRIVER_CARD_BTN_BACK:		//返回
		dispose_temp_card_info();
		lcd_set_screen_back();
		break;
	default:
		break;
	}	
}
//开门处理
static void open_door_handle(unsigned short control_id, unsigned char state)
{
	if(control_id == OPEN_DOOR_BTN_ALL_GROUP)
    {
        //设置分组按钮状态
        btn_all_group_sta = !btn_all_group_sta;
        SetButtonValue(UI_OPEN_DOOR, OPEN_DOOR_BTN_ALL_GROUP, btn_all_group_sta);
        if(btn_all_group_sta)
        {            
            BatchBegin(UI_OPEN_DOOR);
            BatchSetButtonValue(OPEN_DOOR_BTN_A_GROUP + btn_gropu_index, 0);//设置分组按钮弹起
            //设置柜门按钮不可见
            for(int i = OPEN_DOOR_BTN_1; i < OPEN_DOOR_BTN_1 + sys_config.door_count; i++)
            {                
                BatchSetVisible(i, 0);
            }
            BatchEnd();            
        }
        else
        {            
            BatchBegin(UI_OPEN_DOOR);
            BatchSetButtonValue(OPEN_DOOR_BTN_A_GROUP + btn_gropu_index, 1);//设置分组按钮按下              
            //设置柜门按钮可见
            for(int i = OPEN_DOOR_BTN_1; i < OPEN_DOOR_BTN_1 + sys_config.door_count; i++)
            {                
                BatchSetVisible(i, 1);
            }
            BatchEnd();
            lcd_update_door_sta(btn_gropu_index);
        }
    }
    else if(control_id >= OPEN_DOOR_BTN_A_GROUP && control_id <= OPEN_DOOR_BTN_H_GROUP)
    {
        if(btn_all_group_sta)
        {//按组开门
            door_group_open(control_id - OPEN_DOOR_BTN_A_GROUP);
        }
        else
        {           
            BatchBegin(UI_OPEN_DOOR);
            BatchSetButtonValue(OPEN_DOOR_BTN_A_GROUP + btn_gropu_index, 0);//设置之前的分组按钮弹起
            btn_gropu_index = control_id - OPEN_DOOR_BTN_A_GROUP;
            BatchSetButtonValue(OPEN_DOOR_BTN_A_GROUP + btn_gropu_index, 1);//设置新的分组按钮按下                 
            BatchEnd();
            lcd_update_door_sta(btn_gropu_index); 
        }
	}
    else if(control_id >= OPEN_DOOR_BTN_1 && control_id <= OPEN_DOOR_BTN_16)
    {
        door_any_open(btn_gropu_index, control_id - OPEN_DOOR_BTN_1);
    }
    else if(control_id == OPEN_DOOR_BTN_EXIT)
    {
        sys_status.set_workmodel(WORK_ON_MODEL);
    }
}
//保存其它设置到数据库
static rt_bool_t save_other_setting(void)
{
	rt_bool_t result = RT_FALSE;
	//验证数据合法性
	if(temp_setting_info->node_count <= NODE_MAX_COUNT && temp_setting_info->door_count <= DOOR_MAX_COUNT)
	{
		sysinfo_t sysinfo = rt_calloc(1, sizeof(struct sysinfo));
		RT_ASSERT(sysinfo != RT_NULL);
		if(sysinfo_get_by_id(sysinfo, SYSINFO_KEY_ID) >0 )
		{
			if(rt_strlen(temp_setting_info->sys_title) > 0)			
			{
				rt_strncpy(sysinfo->sys_title, temp_setting_info->sys_title, rt_strlen(temp_setting_info->sys_title) + 1);
			}
			sysinfo->en_driver_card = temp_setting_info->en_driver_card;
			sysinfo->node_count = temp_setting_info->node_count;
			sysinfo->door_count = temp_setting_info->door_count;
			sysinfo->open_timeout = temp_setting_info->open_timeout;
			if(sysinfo_update(sysinfo) == 0)
			{				
				result = RT_TRUE;
			}
		}
		rt_free(sysinfo);
	}
	return result;
}
//其它设置
static void other_setting_handle(unsigned short control_id)
{
	switch(control_id)
    {
    case OTHER_SETTING_BTN_ENTER:		//确定
		lcd_set_screen_id(UI_WAITE);
		if(save_other_setting())
		{
			sys_status.set_workmodel(WAITE_RESTART_MODEL);
			lcd_show_message(MSG_RESULT, MSG_SUCCESS);
		}
		else
		{
			lcd_show_message(MSG_RESULT, MSG_FAILED);
		}
		break;
    case OTHER_SETTING_BTN_CANCEL:		//取消
		dispose_temp_stting_info();
		lcd_set_screen_back();
		break;	
	default:		
		break;
	}
}

static void lcd_notify_button(unsigned short screen_id, unsigned short control_id, void *params)
{
    unsigned char state = (unsigned char)params;
    switch(screen_id)
    {
    case UI_ABKEY_CARD:		//密钥卡设置
        abkey_card_handle(control_id);
        break;
	case UI_SYS_CFG:		//系统配置
		sys_config_handle(control_id);
		break;
    case UI_CFG_CARD:		//配置卡设置
        cfg_card_handle(control_id);
        break;
    case UI_POWER_CARD:		//授权卡设置
        power_card_handle(control_id);
        break;	
    case UI_EKEY_CARD:		//钥匙卡设置
        ekey_card_handle(control_id);
        break;	
    case UI_DRIVER_CARD:	//司机卡设置
        driver_card_handle(control_id);
        break;	
	case UI_OPEN_DOOR:		//开门UI
		open_door_handle(control_id, state);
		break;
    case UI_OTHER_SETTING:	//其它设置
        other_setting_handle(control_id);
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
    char *str = (char *)params;
	if(screen_id == UI_OTHER_SETTING)	//其它设置
	{
		switch(control_id)
		{
		case OTHER_SETTING_TEXT_SYS_TITLE:
			{
				rt_size_t len = rt_strlen(str);
				rt_memset(temp_setting_info->sys_title, 0x00, TITLE_SIZE);
				rt_strncpy(temp_setting_info->sys_title, str,  len >= TITLE_SIZE ? (TITLE_SIZE - 1):len);
			}
			break;
		case OTHER_SETTING_TEXT_NODE_COUNT:
			temp_setting_info->node_count = str2int32(str);
			break;
		case OTHER_SETTING_TEXT_DOOR_COUNT:
			temp_setting_info->door_count = str2int32(str);
			break;
		case OTHER_SETTING_TEXT_OPEN_TIMEROUT:
			temp_setting_info->open_timeout	= str2int32(str);
			break;
		case OTHER_SETTING_TEXT_EN_DRIVER_CARD:
			temp_setting_info->en_driver_card = str2int32(str);
		break;		
		default:
			break;
		}
	}
	else
	{
		if(control_id == CFG_CARD_TEXT_NUMBER)
		{
			temp_card_info->num = str2int32(str);
		}
	}
}

void app_lcd_startup(void)
{	
    btn_all_group_sta = 1;
    btn_gropu_index = 0;
	uart_lcd_set_button_notify_hook(lcd_notify_button);
    uart_lcd_set_text_notify_hook(lcd_notify_text);
}
