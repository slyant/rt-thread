#include <rtthread.h>
#include <board.h>
#include <cJSON_util.h>
#include <ic_card_protocol.h>
#include <db_include.h>

struct sys_status_s
{
	rt_uint8_t keya[KEY_LENGTH];
	rt_uint8_t keyb[KEY_LENGTH];
};
typedef struct sys_status_s *sys_status_t;

sys_status_t sys_status = RT_NULL;

//应用卡处理
static void card_app_handle(card_app_type_t type, cJSON *root)
{
	RT_ASSERT(root != RT_NULL);
	cJSON *fileds = cJSON_GetObjectItem(root, "Fileds");
	RT_ASSERT(fileds != RT_NULL);

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
//查询卡测试
static void rfid_scan_thread_entry(void* params)
{
	rt_uint8_t card_id[4];
	rt_uint8_t *buffer = RT_NULL;
	rt_uint16_t buf_len;
	cJSON *root;
	while(1)
	{
		card_base_type_t type = rfid_scan_handle(sys_status->keya, sys_status->keyb, card_id, buffer, &buf_len);
		switch((int)type)
		{
			case CARD_TYPE_BLANK:	//空白卡
				rt_kprintf("CARD_TYPE_BLANK ID:%02x%02x%02x%02x\n", card_id[0], card_id[1], card_id[2], card_id[3]);
				break;
			case CARD_TYPE_KEY:		//密钥卡
				rt_kprintf("CARD_TYPE_KEY ID:%02x%02x%02x%02x\n", card_id[0], card_id[1], card_id[2], card_id[3]);
				if(buffer != RT_NULL && buf_len > 0)
				{
					root = cJSON_Parse((char*)buffer);
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
					}
					else
					{
						rt_kprintf("Parse json error!\n");
					}
				}
				break;
			case CARD_TYPE_APP:
				rt_kprintf("CARD_TYPE_APP ID:%02x%02x%02x%02x\n", card_id[0], card_id[1], card_id[2], card_id[3]);
				if(buffer != RT_NULL && buf_len > 0)
				{
					root = cJSON_Parse((char*)buffer);
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
					}
					else
					{
						rt_kprintf("Parse json error!\n");
					}
				}
				break;
			case CARD_TYPE_UNKNOW:
				rt_kprintf("CARD_TYPE_UNKNOW ID:%02x%02x%02x%02x\n", card_id[0], card_id[1], card_id[2], card_id[3]);
				break;
			case CARD_TYPE_NULL:				
				break;
			default:
				break;			
		}
		if(buffer != RT_NULL)
			rt_free(buffer);
		if(root != RT_NULL)
			cJSON_Delete(root);
		rt_thread_mdelay(100);
	}
}

static int unit_test(void)
{
	sys_status = rt_calloc(1, sizeof(sys_status_t));
	RT_ASSERT(sys_status != RT_NULL);
	
	rt_thread_t thread = rt_thread_create("rfid_scan", rfid_scan_thread_entry, RT_NULL, 
											20*1024, 5, 20);
	if(thread != RT_NULL)
		rt_thread_startup(thread);
	
	return RT_EOK;
}
MSH_CMD_EXPORT(unit_test, startup unit test thread);
