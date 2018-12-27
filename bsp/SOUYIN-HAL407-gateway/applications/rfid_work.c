#include <rtthread.h>
#include <board.h>
#include <app_config.h>

const char* INIT_SYS_TITLE = "公交自助收银管理系统V1.0\0";
const unsigned char INIT_SYS_KEY_A[INIT_KEY_LEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
const unsigned char INIT_SYS_KEY_B[INIT_KEY_LEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

struct sys_status sys_status;
struct sys_config sys_config;

static rt_bool_t sys_abkey_exist(void)
{
	if(rt_memcmp(sys_config.keya, INIT_SYS_KEY_A, INIT_KEY_LEN)==0 && rt_memcmp(sys_config.keyb, INIT_SYS_KEY_B, INIT_KEY_LEN)==0)
	{
		return RT_FALSE;
	}
	return RT_TRUE;
}

//应用卡处理
static void card_app_handle(card_app_type_t type, const cJSON *root)
{
	if(root == RT_NULL) return;
	cJSON *fileds = cJSON_GetObjectItem((cJSON*)root, "Fileds");
	if(fileds == RT_NULL) return;

	switch((int)type)
	{
		case CARD_APP_TYPE_ABKEY:
			{
				rt_kprintf("CARD_APP_TYPE_ABKEY\n");
				char *akey = (char*)cJSON_item_get_string(fileds, "AKey");
				char *bkey = (char*)cJSON_item_get_string(fileds, "BKey");
				rt_kprintf("AKey:%s\n", akey);
				rt_kprintf("BKey:%s\n", bkey);
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

//扫描卡工作线程
static void main_scan_thread_entry(void* params)
{	
	struct rfid_scan_info scan_info;
	rt_memset(&scan_info, 0x00, sizeof(struct rfid_scan_info));	
	rt_uint8_t *tem_buf = RT_NULL;
	scan_info.buffer = &tem_buf;
	while(1)
	{					
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
		rt_thread_mdelay(50);
	}
}

//初始化密钥卡
static void init_card_key(void)
{
	IC_LOCK();
	if(rfid_card_init(CARD_TYPE_KEY, RT_FALSE, RT_NULL, RT_NULL))
	{
		rt_kprintf("init_card_key OK!\n");
	}
	else
	{
		rt_kprintf("init_card_key ERROR!\n");
	}
	IC_UNLOCK();
}
MSH_CMD_EXPORT(init_card_key, init_card_key);

//重置密钥卡
static void reset_card_key(void)
{
	IC_LOCK();
	if(rfid_card_reset(CARD_TYPE_KEY, RT_NULL))
	{
		rt_kprintf("reset_card_key OK!\n");
	}
	else
	{
		rt_kprintf("reset_card_key ERROR!\n");
	}
	IC_UNLOCK();	
}
MSH_CMD_EXPORT(reset_card_key, reset_card_key);

//扫描卡子线程
static void scan_card_thread_entry(void *params)
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

//初始化应用卡
static void init_card_app(void)
{
	
}
MSH_CMD_EXPORT(init_card_app, init_card_app);

//重置应用卡
static void reset_card_app(void)
{
	
}
MSH_CMD_EXPORT(reset_card_app, reset_card_app);

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

rt_bool_t write_card_key_and_update_syskey(void)
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
static void create_app_abkey(void)
{
	IC_LOCK();
	if(sys_config.abkey_exist())
	{//存在系统密钥		
		//需要先验证系统密钥
		rt_mailbox_t auth_key_mb = rt_mb_create("auth_key", 1, RT_IPC_FLAG_FIFO);
		RT_ASSERT(auth_key_mb != RT_NULL);
		
		//扫描卡子线程
		rt_thread_t thread = rt_thread_create("sub_scan", scan_card_thread_entry, auth_key_mb, 
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
		}
		else
		{
			rt_kprintf("sys abkey authentication failed!\n");
		}
	}
	else
	{//不存在系统密钥,则创建系统密钥，先写卡，后保存到系统
		rt_kprintf("sys abkey is not exist!\n");
		write_card_key_and_update_syskey();
	}
	IC_UNLOCK();
}
MSH_CMD_EXPORT(create_app_abkey, create_app_abkey);

//创建配置卡
static void create_app_config(void)
{
	
}
MSH_CMD_EXPORT(create_app_config, create_app_config);

//创建授权卡
static void create_app_power(void)
{
	
}
MSH_CMD_EXPORT(create_app_power, create_app_power);

int unit_test(void)
{
	rt_memset((rt_uint8_t*)&sys_config, 0x00, sizeof(struct sys_config));
	rt_memset((rt_uint8_t*)&sys_status, 0x00, sizeof(struct sys_status));
	
	rt_mutex_init(&sys_status.rfic_lock, "ic_lock", RT_IPC_FLAG_FIFO);
	
	struct sysinfo sysinfo;
	if(sysinfo_get_by_id(&sysinfo, SYSINFO_DB_KEY_ID)>0)
	{
		sys_config.door_count = sysinfo.door_count;
		sys_config.node_count = sysinfo.node_count;
		sys_config.open_timeout = sysinfo.open_timeout;
		rt_strncpy(sys_config.sys_title, sysinfo.sys_title, rt_strlen(sysinfo.sys_title));
		rt_memcpy(sys_config.keya, sysinfo.key_a, INIT_KEY_LEN);
		rt_memcpy(sys_config.keyb, sysinfo.key_b, INIT_KEY_LEN);
		sys_config.abkey_exist = sys_abkey_exist;
		
		rt_kprintf("\nsys_title:%s\nopen_timeout:%d\ndoor_count:%d\nnode_count:%d\n",sys_config.sys_title, sys_config.open_timeout, sysinfo.door_count, sys_config.node_count);
		rt_kprintf("keya:%02X%02X%02X%02X%02X%02X\n", sys_config.keya[0], sys_config.keya[1], sys_config.keya[2], sys_config.keya[3], sys_config.keya[4], sys_config.keya[5]);
		rt_kprintf("keyb:%02X%02X%02X%02X%02X%02X\n", sys_config.keyb[0], sys_config.keyb[1], sys_config.keyb[2], sys_config.keyb[3], sys_config.keyb[4], sys_config.keyb[5]);
		
	}
	else
	{
		rt_kprintf("sysinfo table is not exist a record!");
		RT_ASSERT(RT_FALSE);
	}

	rt_thread_t thread = rt_thread_create("ticscan", main_scan_thread_entry, RT_NULL, 
											10*1024, 12, 20);
	if(thread != RT_NULL)
		rt_thread_startup(thread);	

	rtc_get_time(&(sys_status.sys_datetime));
	SetRtc(sys_status.sys_datetime.year, sys_status.sys_datetime.month, sys_status.sys_datetime.mday, sys_status.sys_datetime.wday,
		sys_status.sys_datetime.hour, sys_status.sys_datetime.min, sys_status.sys_datetime.sec);
	
	return RT_EOK;
}
