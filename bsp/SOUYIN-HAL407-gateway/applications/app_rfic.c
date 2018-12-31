#include <rtthread.h>
#include <board.h>

#include <cJSON_util.h>
#include <ic_card_protocol.h>
#include <rng_helper.h>
#include <app_rfic.h>
#include <app_lcd.h>
#include <app_config.h>

static struct rt_mutex mutex_rfic;
#define IC_LOCK()		rt_mutex_take(&mutex_rfic,RT_WAITING_FOREVER)
#define IC_UNLOCK()		rt_mutex_release(&mutex_rfic)

//应用卡处理
static void card_app_handle(card_app_type_t type, const cJSON *root)
{
	if(root == RT_NULL)	return;
	cJSON *fileds = cJSON_GetObjectItem((cJSON*)root, "Fileds");
	if(fileds == RT_NULL) return;

	switch((int)type)
	{
		case CARD_APP_TYPE_ABKEY:
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
					sys_status.set_screen_id(UI_SYS_CFG);
					sys_status.set_workmodel(CONFIG_MANAGE_MODEL);
				}
				else
				{
					beep_on(2);
				}
				rt_free(akey);
				rt_free(bkey);
			}
			break;
		case CARD_APP_TYPE_CONFIG:
			{
				rt_kprintf("CARD_APP_TYPE_CONFIG\n");
				int num;
				cJSON_item_get_number(fileds, "Num", &num);
				char *pwd = (char*)cJSON_item_get_string(fileds, "Pwd");
				rt_kprintf("Num:%d\n", num);
				rt_kprintf("Pwd:%s\n", pwd);
			}
			break;
		case CARD_APP_TYPE_POWER:
			{
				rt_kprintf("CARD_APP_TYPE_POWER\n");
				int num;
				cJSON_item_get_number(fileds, "Num", &num);
				char *pwd = (char*)cJSON_item_get_string(fileds, "Pwd");
				rt_kprintf("Num:%d\n", num);
				rt_kprintf("Pwd:%s\n", pwd);
			}
			break;	
		case CARD_APP_TYPE_EKEY:
			{
				rt_kprintf("CARD_APP_TYPE_EKEY\n");
				int num;
				cJSON_item_get_number(fileds, "Num", &num);
				char *pwd = (char*)cJSON_item_get_string(fileds, "Pwd");
				rt_kprintf("Num:%d\n", num);
				rt_kprintf("Pwd:%s\n", pwd);
			}
			break;
		case CARD_APP_TYPE_DRIVER:
			{
				rt_kprintf("CARD_APP_TYPE_DRIVER\n");
				int num;
				cJSON_item_get_number(fileds, "Num", &num);
				char *pwd = (char*)cJSON_item_get_string(fileds, "Pwd");
				rt_kprintf("Num:%d\n", num);
				rt_kprintf("Pwd:%s\n", pwd);
			}
			break;
		case CARD_APP_TYPE_LOCKKEY:
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

//初始化密钥卡
rt_bool_t init_card_key(void)
{
	rt_bool_t result = RT_FALSE;
	if(sys_status.get_workmodel() != CONFIG_ABKEY_MODEL && sys_status.get_workmodel() != CONFIG_MANAGE_MODEL)
		return result;
	IC_LOCK();
	if(rfid_card_init(CARD_TYPE_KEY, RT_FALSE, RT_NULL, RT_NULL))
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
	IC_LOCK();
	if(rfid_card_reset(CARD_TYPE_KEY, RT_NULL))
	{
		rt_kprintf("reset_card_key OK!\n");
		result = RT_TRUE;
	}
	else
	{
		rt_kprintf("reset_card_key ERROR!\n");
	}
	IC_UNLOCK();
	return result;
}

//扫描卡子线程
static void sub_scan_thread_entry(void *params)
{	
	rt_mailbox_t auth_key_mb = (rt_mailbox_t)params;
	
	rt_kprintf("please auth key card...\n");	
	while(1)
	{		
		rt_uint8_t *temp = RT_NULL;
		rfid_scan_info_t scan_info = rt_calloc(1, sizeof(struct rfid_scan_info));
		RT_ASSERT(scan_info != RT_NULL);
		scan_info->buffer = &temp;
		card_base_type_t type = rfid_scan_handle(sys_config.keya, sys_config.keyb, scan_info);		
		if(type != CARD_TYPE_NULL)
		{
			if(rt_mb_send(auth_key_mb, (rt_base_t)scan_info) != RT_EOK)
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
//验证密钥卡
static rt_bool_t check_abkey(char *buffer)
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
						rt_uint8_t akey[KEY_LENGTH], bkey[KEY_LENGTH];
						hex2bytes(_akey, KEY_LENGTH * 2, akey);
						hex2bytes(_bkey, KEY_LENGTH * 2, bkey);
						if(rt_memcmp(sys_config.keya, akey, KEY_LENGTH) == 0 && rt_memcmp(sys_config.keyb, bkey, KEY_LENGTH) == 0)
						{//验证通过
							result = RT_TRUE;
						}
					}
				}
			}			
		}
		cJSON_Delete(root);
	}
	return result;
}

//创建密钥卡信息
static rt_uint16_t create_card_key_info(rt_uint8_t *out_akey, rt_uint8_t *out_bkey, rt_uint8_t **out_buffer)
{
	rt_uint16_t buf_len = 0;
	get_rnd_bytes(KEY_LENGTH, out_akey);
	get_rnd_bytes(KEY_LENGTH, out_bkey);

	cJSON *root = cJSON_CreateObject();
	if(root != RT_NULL)
	{
		cJSON_AddNumberToObject(root,"Type",CARD_APP_TYPE_ABKEY);
		cJSON *fileds = cJSON_CreateObject();
		if(fileds != RT_NULL)
		{           
            char* akey_str = rt_calloc(1, KEY_LENGTH * 2 + 1);
            char* bkey_str = rt_calloc(1, KEY_LENGTH * 2 + 1);
            buffer2hex(out_akey, KEY_LENGTH, akey_str);
            buffer2hex(out_bkey, KEY_LENGTH, bkey_str);
            cJSON_AddStringToObject(fileds, "AKey", akey_str);
            cJSON_AddStringToObject(fileds, "BKey", bkey_str);
			rt_free(akey_str); rt_free(bkey_str);
			cJSON_AddItemToObject(root,"Fileds",fileds);
			*out_buffer = (rt_uint8_t*)cJSON_PrintUnformatted(root);     
            buf_len = rt_strlen((char*)*out_buffer);
		}
		cJSON_Delete(root);   
	}
	return buf_len;
}

static rt_bool_t write_card_key_and_update_syskey(void)
{
	rt_bool_t rest = RT_FALSE;
    rt_uint16_t buf_len;
	rt_uint8_t *buffer = RT_NULL;
	rt_uint8_t akey[KEY_LENGTH], bkey[KEY_LENGTH];	 
    buf_len = create_card_key_info(akey, bkey, &buffer);   
	if(buf_len > 0 && buffer != RT_NULL)
	{
		if(rfid_card_write(CARD_TYPE_KEY, akey, bkey, buffer, buf_len) == RT_TRUE)
		{//写密钥卡成功
			rt_kprintf("rfid_card_write OK:\n%s\n", buffer);
			//保存系统密钥
			struct sysinfo sysinfo;
			if(sysinfo_get_by_id(&sysinfo, SYSINFO_DB_KEY_ID)>0)
			{
				rt_memcpy(sysinfo.key_a, akey, KEY_LENGTH);
				rt_memcpy(sysinfo.key_b, bkey, KEY_LENGTH);
				if(sysinfo_update(&sysinfo) == 0)
				{
					rt_memcpy(sys_config.keya, akey, KEY_LENGTH);
					rt_memcpy(sys_config.keyb, bkey, KEY_LENGTH);
					rt_kprintf("sysinfo_update OK!\n");
					rest = RT_TRUE;
				}
			}
			else
			{
				rt_kprintf("sysinfo_update ERROR!\n");
			}
		}
		else
		{
			rt_kprintf("rfid_card_write ERROR:\n%s\n", buffer);
		}
	}
	if(buffer != RT_NULL)
		rt_free(buffer);
	return rest;
}

//创建密钥卡
rt_bool_t create_app_abkey(void)
{	
	rt_bool_t rest = RT_FALSE;
	if(sys_status.get_workmodel() != CONFIG_ABKEY_MODEL)
		return rest;
	IC_LOCK();
	if(sys_config.abkey_exist())
	{//存在系统密钥		
		//需要先验证系统密钥
		rt_mailbox_t auth_key_mb = rt_mb_create("auth_key", 1, RT_IPC_FLAG_FIFO);
		RT_ASSERT(auth_key_mb != RT_NULL);
		
		//扫描卡子线程
		rt_thread_t thread = rt_thread_create("sub_scan", sub_scan_thread_entry, auth_key_mb, 
												1*1024, 5, 20);
		if(thread != RT_NULL)
			rt_thread_startup(thread);
		
		rt_uint8_t pass = 0, retry = 3;		
		rt_ubase_t p = 0;
		do
		{
			//等待接收邮件
			if(rt_mb_recv(auth_key_mb, &p, RT_WAITING_FOREVER)==RT_EOK)
			{
				if(p != 0)
				{
					rfid_scan_info_t scan_info = (rfid_scan_info_t)p;
					if(scan_info->buf_len > 0 && *scan_info->buffer != RT_NULL)
					{					
						//验证密钥卡
						if(check_abkey((char*)*scan_info->buffer))
						{//验证通过							
							pass = 1;
						}
						else
						{//验证失败
							rt_kprintf("sys abkey authentication failed\n");
						}
					}
					else
					{
						rt_kprintf("rx_data:null\n");
					}
					if(*scan_info->buffer != RT_NULL)
						rt_free(*scan_info->buffer);
					if(scan_info != RT_NULL)
						rt_free(scan_info);
				}
			}		
			retry--;
		}
		while(pass == 0 && retry > 0);
		
		if(thread != RT_NULL)
			rt_thread_delete(thread);

		if(auth_key_mb != RT_NULL)
			rt_mb_delete(auth_key_mb);
		if(pass)
		{//验证通过,更新卡密钥
			write_card_key_and_update_syskey();
			rest = RT_TRUE;
		}
		else
		{
			rt_kprintf("sys abkey authentication failed!\n");
		}
	}
	else
	{//不存在系统密钥,则创建系统密钥，先写卡，后保存到系统
		rt_kprintf("sys abkey is not exist!\n");
		rest = write_card_key_and_update_syskey();
	}
	IC_UNLOCK();
	return rest;
}

//初始化应用卡
rt_bool_t init_card_app(void)
{
	return RT_FALSE;
}

//重置应用卡
rt_bool_t reset_card_app(void)
{
	return RT_FALSE;
}

//创建配置卡
rt_bool_t create_app_config(void)
{
	return RT_FALSE;
}

//创建授权卡
rt_bool_t create_app_power(void)
{
	return RT_FALSE;
}

//扫描卡工作线程
static void main_scan_thread_entry(void* params)
{	
	struct rfid_scan_info scan_info;
	rt_memset(&scan_info, 0x00, sizeof(struct rfid_scan_info));	
	rt_uint8_t *tem_buf = RT_NULL;
	scan_info.buffer = &tem_buf;
	while(1)
	{		
		rt_thread_mdelay(50);		
		if(sys_status.get_workmodel() != WORK_ON_MODEL) continue;
		IC_LOCK();
		*scan_info.buffer = RT_NULL;
		card_base_type_t type = rfid_scan_handle(sys_config.keya, sys_config.keyb, &scan_info);
		IC_UNLOCK();
		switch((int)type)
		{
			case CARD_TYPE_BLANK:	//空白卡
				beep_on(1);
				rt_kprintf("CARD_TYPE_BLANK ID:%02x%02x%02x%02x\n", scan_info.card_id[0], scan_info.card_id[1], scan_info.card_id[2], scan_info.card_id[3]);				
				break;
			case CARD_TYPE_KEY:		//密钥卡
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
							card_app_handle((card_app_type_t)type, root);							
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
							card_app_handle((card_app_type_t)type, root);
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

void app_rfic_startup(void)
{
	rt_mutex_init(&mutex_rfic, "ic_lock", RT_IPC_FLAG_FIFO);
	
	rt_thread_t thread = rt_thread_create("ticscan", main_scan_thread_entry, RT_NULL, 
											10*1024, 12, 20);
	if(thread != RT_NULL)
		rt_thread_startup(thread);	
}
