#include <rtthread.h>
#include <board.h>

#include <cJSON_util.h>
#include <ic_card_protocol.h>
#include <rng_helper.h>
#include <app_config.h>

static const char MSG_FAILED[] =  {0xB2, 0xD9, 0xD7, 0xF7, 0xCA, 0xA7, 0xB0, 0xDC, 0x00};	//操作失败
static const char MSG_NOT_ENOUGH[] = {0xC3, 0xBB, 0xD3, 0xD0, 0xB8, 0xFC, 0xB6, 0xE0, 0xB5, 0xC4, 0xB9, 0xF1, 0xD7, 0xD3, 0xBF, 0xC9, 
                                     0xB1, 0xBB, 0xB7, 0xD6, 0xC5, 0xE4, 0x21, 0x00};      //没有更多的柜子可被分配!
static const char MSG_OPEN_1[] = {0xC8, 0xA1, 0xB3, 0xF6, 0xBF, 0xD5, 0xD5, 0xA2, 0xBA, 0xD0, 0xBA, 0xF3, 0x2C, 0xC7, 0xEB, 0xB9, 
                                    0xD8, 0xBA, 0xC3, 0xC3, 0xC5, 0x00};  //取出空闸盒后,请关好门
static const char MSG_OPEN_2[] = {0xB7, 0xC5, 0xC8, 0xEB, 0xCA, 0xB5, 0xD5, 0xA2, 0xBA, 0xD0, 
                                    0xBA, 0xF3, 0x2C, 0xC7, 0xEB, 0xB9, 0xD8, 0xBA, 0xC3, 0xC3, 0xC5, 0x00};    //放入实闸盒后,请关好门
static const char MSG_FINISH[] = {0xC4, 0xFA, 0xD2, 0xD1, 0xCD, 0xEA, 0xB3, 0xC9, 0xCA, 0xD5, 0xD2, 0xF8, 0x2C, 0xC7, 0xEB, 0xCE, 
                                    0xF0, 0xD6, 0xD8, 0xB8, 0xB4, 0xCB, 0xA2, 0xBF, 0xA8, 0x00};    //您已完成收银,请勿重复刷卡
    
static struct rt_mutex mutex_rfic;
#define IC_LOCK()		rt_mutex_take(&mutex_rfic,RT_WAITING_FOREVER)
#define IC_UNLOCK()		rt_mutex_release(&mutex_rfic)
#define IC_RESET()		rfic_scan_reset()

static char* get_door_num_from_id(rt_uint8_t group_index, rt_uint8_t door_index)
{
    char *result = rt_calloc(1, 10);
    result[0] = 'A' + group_index;
    rt_sprintf(result + 1, "%d", door_index + 1);
    return result;
}    
                                     
//应用卡处理
static void card_app_handle(rt_uint8_t card_id[4], enum card_app_type type, const cJSON *root)
{
	if(root == RT_NULL)	return;
	cJSON *fileds = cJSON_GetObjectItem((cJSON*)root, "Fileds");
	if(fileds == RT_NULL) return;

	rt_uint32_t c_id = bytes2uint32(card_id);
	switch((int)type)
	{
	case CARD_APP_TYPE_ABKEY://密钥卡
		{
			rt_kprintf("CARD_APP_TYPE_ABKEY\n");
			char *akey_str = (char*)cJSON_item_get_string(fileds, "AKey");
			char *bkey_str = (char*)cJSON_item_get_string(fileds, "BKey");
			rt_kprintf("AKey:%s\n", akey_str);
			rt_kprintf("BKey:%s\n", akey_str);
			rt_uint8_t *akey = rt_calloc(1, KEY_LENGTH);
			rt_uint8_t *bkey = rt_calloc(1, KEY_LENGTH);
			hex2bytes(akey_str, KEY_LENGTH*2, akey);
			hex2bytes(bkey_str, KEY_LENGTH*2, bkey);
			if(rt_memcmp(sys_config.keya, akey, KEY_LENGTH) == 0 && rt_memcmp(sys_config.keyb, bkey, KEY_LENGTH) == 0)
			{//密钥卡验证通过
				sys_status.set_workmodel(CONFIG_ABKEY_MODEL);
			}
			else
			{
				beep_on(2);
			}
			rt_free(akey);
			rt_free(bkey);
		}
		break;
	case CARD_APP_TYPE_CONFIG://配置卡
		{
			rt_kprintf("CARD_APP_TYPE_CONFIG\n");
			int num;
			cJSON_item_get_number(fileds, "Num", &num);
			char *pwd = (char*)cJSON_item_get_string(fileds, "Pwd");
			rt_kprintf("Num:%d\n", num);
			rt_kprintf("Pwd:%s\n", pwd);			
			if(cardinfo_count_by_any(num, c_id, CARD_APP_TYPE_CONFIG, pwd) > 0)
			{//配置卡验证通过
				sys_status.set_workmodel(CONFIG_MANAGE_MODEL);
			}
			else
			{
				beep_on(2);
			}
		}
		break;
	case CARD_APP_TYPE_POWER://管理卡
		{
			rt_kprintf("CARD_APP_TYPE_POWER\n");
			int num;
			cJSON_item_get_number(fileds, "Num", &num);
			char *pwd = (char*)cJSON_item_get_string(fileds, "Pwd");
			rt_kprintf("Num:%d\n", num);
			rt_kprintf("Pwd:%s\n", pwd);
			if(cardinfo_count_by_any(num, c_id, CARD_APP_TYPE_POWER, pwd) > 0)
			{//管理卡验证通过
                sys_status.card_num = num;
				sys_status.set_workmodel(WORK_MANAGE_MODEL);
                sys_status.manage_display_start();
			}
			else
			{
				beep_on(2);
			}			
		}
		break;	
	case CARD_APP_TYPE_EKEY://钥匙卡
		{
			rt_kprintf("CARD_APP_TYPE_EKEY\n");
			int num;
			cJSON_item_get_number(fileds, "Num", &num);
			char *pwd = (char*)cJSON_item_get_string(fileds, "Pwd");
			rt_kprintf("Num:%d\n", num);
			rt_kprintf("Pwd:%s\n", pwd);                       
            if(cardinfo_count_by_any(num, c_id, CARD_APP_TYPE_EKEY, pwd) > 0)
            {
                sys_status.card_num = num; 
                doorinfo_t doorinfo = rt_calloc(1, sizeof(struct doorinfo));
                /*
                这里要注意：不能直接从数据库查询，因为更新工作在工作队列中，可能还未来得及执行，
                那就等待工作队列中的任务全部执行完毕。
                */
                lcd_set_screen_id(UI_WAITE);
                while(app_workqueue_get_length() > 0) rt_thread_mdelay(200);
                if(doorinfo_get_by_card_num(doorinfo, num) > 0)
                {//已刷过卡
                    rt_uint8_t group_index = GET_GROUP_ID(doorinfo->id);
                    rt_uint8_t door_index = GET_DOOR_ID(doorinfo->id);
                    char *door_num_str = get_door_num_from_id(group_index, door_index); 
                    if(doorinfo->status == DOOR_STA_OPEN_1 || doorinfo->status == DOOR_STA_CLOSE_1 || doorinfo->status == DOOR_STA_OPEN_2)
                    {//首次刷卡开门后未关门，或首次关门后再次刷卡，都执行开门动作
                        lcd_show_door_num(door_num_str, MSG_OPEN_2);
                        door_any_open(group_index, door_index);
                        if(doorinfo->status == DOOR_STA_CLOSE_1)
                        {//首次关门后再次刷卡，更新柜门状态
                            //sql
                            char *sql = rt_calloc(1, 128);
                            rt_sprintf(sql, "update doorinfo set status=%d where id=%d;", DOOR_STA_OPEN_2, doorinfo->id);
                            app_workqueue_exe_sql(sql);        //更新柜门状态为:第2次开门     
                        }
                    }
                    else                        
                    {
                        lcd_show_door_num(door_num_str, MSG_FINISH);                          
                    }
                    rt_free(door_num_str);
                }
                else
                {//未刷过卡,首次刷卡开门
                    if(doorinfo_get_by_status(doorinfo, DOOR_STA_LOCK) > 0)
                    {//自动分配得到可用柜门
                        rt_uint8_t group_index = GET_GROUP_ID(doorinfo->id);
                        rt_uint8_t door_index = GET_DOOR_ID(doorinfo->id);
                        char *door_num_str = get_door_num_from_id(group_index, door_index);
                        lcd_show_door_num(door_num_str, MSG_OPEN_1);
                        rt_free(door_num_str);
                        door_any_open(group_index, door_index);
                        //更新柜门状态
                        //sql
                        char *sql = rt_calloc(1, 128);
                        rt_sprintf(sql, "update doorinfo set status=%d,card_num=%d where id=%d;", DOOR_STA_OPEN_1, num, doorinfo->id);
                        app_workqueue_exe_sql(sql);        //更新柜门状态为:第1次开门                        
                    }
                    else
                    {//无可用柜门
                        lcd_show_message(MSG_FAILED, MSG_NOT_ENOUGH);
                        sys_status.open_display_start();
                    }                    
                }
                rt_free(doorinfo);                
            }
            else
			{
				beep_on(2);
			}
		}
		break;
	case CARD_APP_TYPE_DRIVER://司机卡
		{
			rt_kprintf("CARD_APP_TYPE_DRIVER\n");
			int num;
			cJSON_item_get_number(fileds, "Num", &num);
			char *pwd = (char*)cJSON_item_get_string(fileds, "Pwd");
			rt_kprintf("Num:%d\n", num);
			rt_kprintf("Pwd:%s\n", pwd);
            sys_status.card_num = num;
		}
		break;
	case CARD_APP_TYPE_LOCKKEY://锁钥卡
		{
			rt_kprintf("CARD_APP_TYPE_LOCKKEY\n");
			int num;
			cJSON_item_get_number(fileds, "Num", &num);
			char *pwd = (char*)cJSON_item_get_string(fileds, "Pwd");
			rt_kprintf("Num:%d\n", num);
			rt_kprintf("Pwd:%s\n", pwd);
		}
		break;
	// case CARD_APP_TYPE_UNKNOW:
	// 	break;
	default:
		rt_kprintf("CARD_APP_TYPE_UNKNOW\n");
		break;
	}
}

//扫描卡主线程
static void main_scan_thread_entry(void* params)
{	
	struct rfic_scan_info scan_info;
	rt_memset(&scan_info, 0x00, sizeof(struct rfic_scan_info));	
	rt_uint8_t *tem_buf = RT_NULL;
	scan_info.buffer = &tem_buf;
	IC_RESET();
	while(1)
	{		
		rt_thread_mdelay(50);		
		if(sys_status.get_workmodel() != WORK_ON_MODEL) continue;
		
		IC_LOCK();
		*scan_info.buffer = RT_NULL;
		enum card_base_type type = rfic_scan_handle(sys_config.keya, sys_config.keyb, &scan_info);
		IC_UNLOCK();
		switch((int)type)
		{
			case CARD_TYPE_BLANK:	//空白卡
				lcd_wakeup();
				beep_on(3);
				rt_kprintf("CARD_TYPE_BLANK ID:%02x%02x%02x%02x\n", scan_info.card_id[0], scan_info.card_id[1], scan_info.card_id[2], scan_info.card_id[3]);				
				break;
			case CARD_TYPE_KEY:		//密钥卡
				lcd_wakeup();
				beep_on(1);
				rt_kprintf("CARD_TYPE_KEY ID:%02x%02x%02x%02x\n", scan_info.card_id[0], scan_info.card_id[1], scan_info.card_id[2], scan_info.card_id[3]);
				if(*scan_info.buffer != RT_NULL && scan_info.buf_len > 0)
				{
					cJSON *root = cJSON_Parse((char*)*scan_info.buffer);
					if(root != RT_NULL)
					{
						int type ;
						int ret = cJSON_item_get_number(root, "Type", &type);
						if(ret == 0 && type == CARD_APP_TYPE_ABKEY)
						{
							card_app_handle(scan_info.card_id, (enum card_app_type)type, root);							
						}
						else
						{
							beep_on(2);
							rt_kprintf("Parse json error!\n");
						}
						cJSON_Delete(root);
					}
					else
					{
						beep_on(2);
						rt_kprintf("Parse json error!\n");
					}
				}
				break;
			case CARD_TYPE_APP:
				lcd_wakeup();
				beep_on(1);
				rt_kprintf("CARD_TYPE_APP ID:%02x%02x%02x%02x\n", scan_info.card_id[0], scan_info.card_id[1], scan_info.card_id[2], scan_info.card_id[3]);
				if(*scan_info.buffer != RT_NULL && scan_info.buf_len > 0)
				{
					cJSON *root = cJSON_Parse((char*)*scan_info.buffer);
					if(root != RT_NULL)
					{
						int type ;
						int ret = cJSON_item_get_number(root, "Type", &type);
						if(ret == 0)
						{
							card_app_handle(scan_info.card_id, (enum card_app_type)type, root);
						}
						else
						{
							rt_kprintf("Parse json error!\n");
						}
						cJSON_Delete(root);
					}
					else
					{
						rt_kprintf("Parse json error!\n");
					}
				}
				break;
			case CARD_TYPE_UNKNOW:
				lcd_wakeup();
				beep_on(3);
				rt_kprintf("CARD_TYPE_UNKNOW ID:%02x%02x%02x%02x\n", scan_info.card_id[0], scan_info.card_id[1], scan_info.card_id[2], scan_info.card_id[3]);
				break;
//			case CARD_TYPE_NULL:				
//				break;
			default:				
				break;			
		}
		if(*scan_info.buffer != RT_NULL)
			rt_free(*scan_info.buffer);	
	}
}

//初始化密钥卡
rt_bool_t init_card_key(void)
{
	rt_bool_t result = RT_FALSE;
	if(sys_status.get_workmodel() != CONFIG_ABKEY_MODEL && sys_status.get_workmodel() != CONFIG_MANAGE_MODEL)
		return result;	
	
	IC_LOCK();IC_RESET();
	if(rfic_card_init(CARD_TYPE_KEY, RT_FALSE, RT_NULL, RT_NULL))
	{
		rt_kprintf("init_card_key ok!\n");
		result = RT_TRUE;
	}
	else
	{
		rt_kprintf("init_card_key error!\n");
	}
	IC_UNLOCK();
	return result;
}

//重置密钥卡
rt_bool_t reset_card_key(void)
{
	rt_bool_t result = RT_FALSE;
	if(sys_status.get_workmodel() != CONFIG_ABKEY_MODEL && sys_status.get_workmodel() != CONFIG_MANAGE_MODEL)
		return result;	
	
	IC_LOCK();IC_RESET();
	if(rfic_card_reset(CARD_TYPE_KEY, RT_NULL))
	{
		rt_kprintf("reset_card_key ok!\n");
		result = RT_TRUE;
	}
	else
	{
		rt_kprintf("reset_card_key error!\n");
	}
	IC_UNLOCK();
	return result;
}

//扫描卡子线程
static void sub_scan_thread_entry(void *params)
{	
	rt_mailbox_t sub_scan_mb = (rt_mailbox_t)params;		
	while(1)
	{		
		rt_uint8_t *temp = RT_NULL;
		rfic_scan_info_t scan_info = rt_calloc(1, sizeof(struct rfic_scan_info));
		RT_ASSERT(scan_info != RT_NULL);
		scan_info->buffer = &temp;
		enum card_base_type type = rfic_scan_handle(sys_config.keya, sys_config.keyb, scan_info);		
		if(type != CARD_TYPE_NULL)
		{
			if(rt_mb_send(sub_scan_mb, (rt_base_t)scan_info) != RT_EOK)
			{
				if(*scan_info->buffer != RT_NULL) 
					rt_free(*scan_info->buffer);
				rt_free(scan_info);
			}
		}
		else
		{
			if(*scan_info->buffer != RT_NULL)
				rt_free(*scan_info->buffer);
			rt_free(scan_info);
		}		
		rt_thread_mdelay(50);
	}
}
//解析密钥卡
static rt_bool_t parse_abkey(char *buffer, rt_uint8_t out_akey[KEY_LENGTH], rt_uint8_t out_bkey[KEY_LENGTH])
{
	rt_bool_t result = RT_FALSE;
	cJSON *root = cJSON_Parse(buffer);
	if(root != RT_NULL)
	{
		int type;
		if(cJSON_item_get_number(root, "Type", &type) == 0)
		{
			if(type == CARD_APP_TYPE_ABKEY)
			{//是密钥卡
				cJSON *fileds = cJSON_GetObjectItem(root, "Fileds");
				if(fileds != RT_NULL)
				{
					char *_akey = (char*)cJSON_item_get_string(fileds, "AKey");
					char *_bkey = (char*)cJSON_item_get_string(fileds, "BKey");
					if(_akey != RT_NULL && _bkey != RT_NULL)
					{
						hex2bytes(_akey, KEY_LENGTH * 2, out_akey);
						hex2bytes(_bkey, KEY_LENGTH * 2, out_bkey);
						result = RT_TRUE;
					}
				}
			}			
		}
		cJSON_Delete(root);
	}
	return result;
}

//创建密钥卡信息
static rt_uint16_t create_card_key_info(rt_uint8_t in_akey[KEY_LENGTH], rt_uint8_t in_bkey[KEY_LENGTH], rt_uint8_t **out_buffer)
{
	rt_uint16_t buf_len = 0;
	cJSON *root = cJSON_CreateObject();
	if(root != RT_NULL)
	{
		cJSON_AddNumberToObject(root, "Type", CARD_APP_TYPE_ABKEY);
		cJSON *fileds = cJSON_CreateObject();
		if(fileds != RT_NULL)
		{           
            char* akey_str = rt_calloc(1, KEY_LENGTH * 2 + 1);
            char* bkey_str = rt_calloc(1, KEY_LENGTH * 2 + 1);
            buffer2hex(in_akey, KEY_LENGTH, akey_str);
            buffer2hex(in_bkey, KEY_LENGTH, bkey_str);
            cJSON_AddStringToObject(fileds, "AKey", akey_str);
            cJSON_AddStringToObject(fileds, "BKey", bkey_str);
			rt_free(akey_str); rt_free(bkey_str);
			cJSON_AddItemToObject(root, "Fileds", fileds);
			*out_buffer = (rt_uint8_t*)cJSON_PrintUnformatted(root);     
            buf_len = rt_strlen((char*)*out_buffer);
		}
		cJSON_Delete(root);   
	}
	return buf_len;
}
//写密钥卡
static rt_bool_t write_card_key(rt_uint8_t in_akey[KEY_LENGTH], rt_uint8_t in_bkey[KEY_LENGTH])
{
	rt_bool_t result = RT_FALSE;
    rt_uint16_t buf_len;
	rt_uint8_t *buffer = RT_NULL;
	
    buf_len = create_card_key_info(in_akey, in_bkey, &buffer);   
	if(buf_len > 0 && buffer != RT_NULL)
	{
		if(rfic_card_write(CARD_TYPE_KEY, in_akey, in_bkey, buffer, buf_len) == RT_TRUE)
		{//写卡成功
			rt_kprintf("rfic_card_write ok:\n%s\n", buffer);
			result = RT_TRUE;
		}
		else
		{
			rt_kprintf("rfic_card_write error:\n%s\n", buffer);
		}
	}
	if(buffer != RT_NULL)
		rt_free(buffer);
	return result;
}
//创建密钥卡
rt_bool_t create_card_key(void)
{	
	rt_bool_t result = RT_FALSE;
	if(sys_status.get_workmodel() != CONFIG_ABKEY_MODEL)
		return result;
		
	rt_uint8_t akey[KEY_LENGTH], bkey[KEY_LENGTH];	
	get_rnd_bytes(KEY_LENGTH, akey);
	get_rnd_bytes(KEY_LENGTH, bkey);
	IC_LOCK();IC_RESET();
	result = write_card_key(akey, bkey);
	IC_UNLOCK();
	if(result)
	{
		result = sys_config.update_sys_key(akey, bkey);
	}	
	return result;
}

//备份密钥卡
rt_bool_t backup_card_key(void)
{	
	rt_bool_t result = RT_FALSE;
	if(sys_status.get_workmodel() != CONFIG_ABKEY_MODEL && sys_status.get_workmodel() != CONFIG_MANAGE_MODEL)
		return result;
	
	IC_LOCK();IC_RESET();
	if(sys_config.abkey_exist())
	{//存在系统密钥
		result = write_card_key(sys_config.keya, sys_config.keyb);
	}
	else
	{//不存在系统密钥
		rt_kprintf("sys abkey is not exist!\n");
	}
	IC_UNLOCK();
	return result;
}

//恢复密钥卡
rt_bool_t restore_card_key(void)
{	
	rt_bool_t result = RT_FALSE;
	if(sys_status.get_workmodel() != CONFIG_ABKEY_MODEL)
		return result;
	
    rt_mailbox_t sub_scan_mb = rt_mb_create("sub_scan", 1, RT_IPC_FLAG_FIFO);
    RT_ASSERT(sub_scan_mb != RT_NULL);   
    
    //扫描卡子线程
    IC_LOCK();IC_RESET();
    rt_thread_t thread = rt_thread_create("sub_scan", sub_scan_thread_entry, sub_scan_mb, 
                                            1*1024, 5, 20);
    if(thread != RT_NULL)
        rt_thread_startup(thread);
    
    rt_ubase_t p = 0;
    //等待接收邮件
    if(rt_mb_recv(sub_scan_mb, &p, rt_tick_from_millisecond(500))==RT_EOK)
    {
        if(p != 0)
        {
            rfic_scan_info_t scan_info = (rfic_scan_info_t)p;
            if(scan_info->buf_len > 0 && *scan_info->buffer != RT_NULL)
            {					
                rt_uint8_t akey[KEY_LENGTH], bkey[KEY_LENGTH];
                //解析密钥卡
                if(parse_abkey((char*)*scan_info->buffer, akey, bkey))
                {//解析成功
                    result = sys_config.update_sys_key(akey, bkey);
                }
            }
            if(*scan_info->buffer != RT_NULL)
                rt_free(*scan_info->buffer);
            if(scan_info != RT_NULL)
                rt_free(scan_info);
        }
    }    
    if(thread != RT_NULL)
        rt_thread_delete(thread);
    if(sub_scan_mb != RT_NULL)
        rt_mb_delete(sub_scan_mb);
    IC_UNLOCK();
    
	return result;
}

//初始化应用卡
rt_bool_t init_card_app(rt_bool_t use_money_bag)
{
	rt_bool_t result = RT_FALSE;
	if(sys_status.get_workmodel() != CONFIG_MANAGE_MODEL)
		return result;
	
	IC_LOCK();IC_RESET();
	if(rfic_card_init(CARD_TYPE_APP, use_money_bag, sys_config.keya, sys_config.keyb))
	{
		rt_kprintf("init_card_app ok!\n");
		result = RT_TRUE;
	}
	else
	{
		rt_kprintf("init_card_app error!\n");
	}
	IC_UNLOCK();
	return result;
}
//重置应用卡
rt_bool_t reset_card_app(void)
{
	rt_bool_t result = RT_FALSE;
	if(sys_status.get_workmodel() != CONFIG_MANAGE_MODEL)
		return result;
	
	IC_LOCK();IC_RESET();
	if(rfic_card_reset(CARD_TYPE_APP, sys_config.keyb))
	{
		rt_kprintf("init_card_key ok!\n");
		result = RT_TRUE;
	}
	else
	{
		rt_kprintf("init_card_key error!\n");
	}
	IC_UNLOCK();
	return result;
}
//创建应用卡信息
static rt_uint16_t create_card_app_info(enum card_app_type type, rt_uint16_t num, char *pwd, rt_uint8_t **out_buffer)
{
	rt_uint16_t buf_len = 0;
	cJSON *root = cJSON_CreateObject();
	if(root != RT_NULL)
	{
		cJSON_AddNumberToObject(root, "Type", type);
		cJSON *fileds = cJSON_CreateObject();
		if(fileds != RT_NULL)
		{           
            cJSON_AddNumberToObject(fileds, "Num", num);
            cJSON_AddStringToObject(fileds, "Pwd", pwd);
			cJSON_AddItemToObject(root, "Fileds", fileds);
			*out_buffer = (rt_uint8_t*)cJSON_PrintUnformatted(root);     
            buf_len = rt_strlen((char*)*out_buffer);
		}
		cJSON_Delete(root);   
	}
	return buf_len;
}
//写应用卡信息
static rt_bool_t write_card_app_info(enum card_app_type type, rt_uint16_t num, char *pwd)
{
	rt_bool_t result = RT_FALSE;
    rt_uint16_t buf_len;
	rt_uint8_t *buffer = RT_NULL;
	
    buf_len = create_card_app_info(type, num, pwd, &buffer);   
	if(buf_len > 0 && buffer != RT_NULL)
	{
		if(rfic_card_write(CARD_TYPE_APP, sys_config.keya, sys_config.keyb, buffer, buf_len) == RT_TRUE)
		{//写卡成功
			rt_kprintf("rfic_card_write ok:\n%s\n", buffer);
			result = RT_TRUE;
		}
		else
		{
			rt_kprintf("rfic_card_write error:\n%s\n", buffer);
		}
	}
	if(buffer != RT_NULL)
		rt_free(buffer);
	return result;
}
//创建应用卡
rt_bool_t create_card_app(enum card_app_type type, rt_uint16_t num, char *pwd)
{
	rt_bool_t result = RT_FALSE;
	if(sys_status.get_workmodel() != CONFIG_MANAGE_MODEL)
		return result;
	
    rt_mailbox_t sub_scan_mb = rt_mb_create("sub_scan", 1, RT_IPC_FLAG_FIFO);
    RT_ASSERT(sub_scan_mb != RT_NULL);   
    
    //扫描卡子线程
    IC_LOCK();IC_RESET();
    rt_thread_t thread = rt_thread_create("sub_scan", sub_scan_thread_entry, sub_scan_mb, 
                                            1*1024, 5, 20);
    if(thread != RT_NULL)
        rt_thread_startup(thread);
	
    rt_uint32_t card_id;
    rt_ubase_t p = 0;
    //等待接收邮件
    if(rt_mb_recv(sub_scan_mb, &p, rt_tick_from_millisecond(500))==RT_EOK)
    {
        if(p != 0)
        {
            rfic_scan_info_t scan_info = (rfic_scan_info_t)p;
			if(scan_info->base_type == CARD_TYPE_APP)
				result = RT_TRUE;
			
			card_id = bytes2uint32(scan_info->card_id);
				
            if(*scan_info->buffer != RT_NULL)
                rt_free(*scan_info->buffer);
            if(scan_info != RT_NULL)
                rt_free(scan_info);
        }
    }
    if(thread != RT_NULL)
        rt_thread_delete(thread);
    if(sub_scan_mb != RT_NULL)
        rt_mb_delete(sub_scan_mb);    	
	
	if(result)
	{
		result = RT_FALSE;
		cardinfo_t cardinfo = rt_calloc(1, sizeof(struct cardinfo));
		RT_ASSERT(cardinfo != RT_NULL);		
		cardinfo->num = num;
		cardinfo->id = card_id;
		rt_strncpy(cardinfo->pwd, pwd, rt_strlen(pwd));
		cardinfo->type = type;
		if(cardinfo_add(cardinfo) == 0)
		{
			result = RT_TRUE;
		}
		if(result)
		{
			result = write_card_app_info(type, cardinfo->num, cardinfo->pwd);
			if(! result)
			{
				cardinfo_del(cardinfo->num);
			}
		}			
		rt_free(cardinfo);
	}
	IC_UNLOCK();
	return result;
}	


void app_rfic_startup(void)
{
	rt_mutex_init(&mutex_rfic, "ic_lock", RT_IPC_FLAG_FIFO);
	
	rt_thread_t thread = rt_thread_create("ticscan", main_scan_thread_entry, RT_NULL, 
											10*1024, 12, 20);
	if(thread != RT_NULL)
		rt_thread_startup(thread);	
}
