
/*
 *  IC卡操作程序，完成人机交互功能而不是驱动。
 *  相关驱动程序在RFIC分组内
 *
 *
 *
 *
 */


#include <rtthread.h>
#include <db_include.h>
#include <stdio.h>
#include "rfic-manage.h"
#include "ic_card_protocol.h"
#include "dev_rfic.h"
#include "cJSON.h"
#include "drv_rng.h"

extern unsigned char card_type[2];
extern const unsigned char FACTORY_KEY_A[];
extern const unsigned char FACTORY_KEY_B[];
extern uint8_t sys_key_a[6];
extern uint8_t sys_key_b[6];
extern unsigned char card_id[4];
extern const char* SYS_TITLE;

card_msg card_arg;
cardinfo_t cardinfo;
char card_pwd[16]={0};
char card_iden[16]={0};
volatile uint8_t iden_over=0;

static void hex_to_str(uint8_t *hex, char *str, uint8_t len)   //len为hex数据的长度
{
	uint8_t ch,i;
	
	for(i=0;i<len;i++)
	{
		ch = hex[i]>>4;
		if(ch>9) *str = (ch-10)+'A'; else *str = ch+'0';
		str++;
		ch = hex[i]&0x0F;
		if(ch>9) *str = (ch-10)+'A'; else *str = ch+'0';
		str++;
	}
	*str = 0;
}

static void str_to_hex(char *str, uint8_t *hex, uint8_t len)   //len为str的长度
{
	uint8_t i,t;
	t=0;
	for(i=0;i<len;i++)
	{
		if(i%2==0)
		{
			if(str[i]>'9') hex[t] = (str[i]-'A'+0x0A)<<4; else hex[t] = (str[i]-'0')<<4;
		}
		else{
			if(str[i]>'9') hex[t] |= (str[i]-'A'+0x0A); else hex[t] |= (str[i]-'0');
			t++;
		}
	}
}

static char* creat_json(card_msg *arg)  //函数不对结构体内密钥进行检查
{
	cJSON *json_root;
	cJSON *typ_item;
	cJSON *filed_item;
	cJSON *pwd_item;
	cJSON *iden_item;
	cJSON *Akey_item;
	cJSON *Bkey_item;
	char * out;

	json_root = cJSON_CreateObject();    //创建json根对象
	typ_item = cJSON_CreateNumber(arg->typ);  //创建数字对象
	cJSON_AddItemToObject(json_root,"Type",typ_item);  //将数字对象链接至根对象

	if(arg->typ != ROOT_CARD)   //如果不是密钥卡类型
	{
		iden_item  = cJSON_CreateString(arg->jsiden);   //卡编号
		cJSON_AddItemToObject(json_root,"Iden",iden_item);
		
		pwd_item   = cJSON_CreateString(arg->jspwd);    //卡密码，非密钥对
		cJSON_AddItemToObject(json_root,"Pwd",pwd_item);
	}

	filed_item = cJSON_CreateObject();   //创建json对象
	cJSON_AddItemToObject(json_root,"Filed",filed_item); //将对象链接至根节点，同时定义该对象字段
	
	char*str = rt_malloc(16);
	hex_to_str(arg->jskeya,str,6);
	Akey_item = cJSON_CreateString(str);
	
	hex_to_str(arg->jskeyb,str,6);
	Bkey_item = cJSON_CreateString(str);
	
	cJSON_AddItemToObject(filed_item,"Akey",Akey_item);
	cJSON_AddItemToObject(filed_item,"Bkey",Bkey_item);
	
	out = cJSON_PrintUnformatted(json_root);       //将json信息非格式化字符串输出
	rt_free(str);
	rt_kprintf("creat json: %d \n",rt_strlen(out));
	rt_kprintf(out);
	rt_kprintf("\n");
	if(json_root!=RT_NULL)  cJSON_Delete(json_root);
	return out;
}

uint8_t decode_json(char *json)   //函数仅对密钥卡进行判定，所有参数直接解析不做判定
{
	rt_kprintf(json); rt_kprintf("\n");
	cJSON *root=cJSON_Parse(json); // 获取根目录
	if(root==RT_NULL) {rt_kprintf("json root get faulted ! \n"); return 0;}
	
	cJSON*typ=cJSON_GetObjectItem(root,"Type");         //获取Type对象的成员	
	
	card_arg.typ = typ->valueint;
	if(card_arg.typ != ROOT_CARD)
	{
		cJSON*pwd=cJSON_GetObjectItem(root,"Pwd");
		cJSON*iden=cJSON_GetObjectItem(root,"Iden");
		rt_memcpy(card_arg.jspwd,pwd->valuestring,rt_strlen(pwd->valuestring));
		rt_memcpy(card_arg.jsiden,iden->valuestring,rt_strlen(iden->valuestring));
		rt_kprintf(pwd->valuestring); rt_kprintf("\n");
	}
	
	cJSON*fileds=cJSON_GetObjectItem(root,"Filed");     //fileds对象
	cJSON*akey = cJSON_GetObjectItem(fileds,"Akey");    //fileds对象的Akey
	cJSON*bkey = cJSON_GetObjectItem(fileds,"Bkey");    //fileds对象的Bkey
	
	str_to_hex(akey->valuestring,card_arg.jskeya,12);
	str_to_hex(bkey->valuestring,card_arg.jskeyb,12);
	rt_kprintf("typ: %d \n",card_arg.typ);
	if(card_arg.typ != ROOT_CARD)
	{
		rt_kprintf("Iden: ");  rt_kprintf(card_arg.jsiden);  rt_kprintf("\n");
		rt_kprintf("Pwd:  ");   rt_kprintf(card_arg.jspwd);   rt_kprintf("\n");
	}
	rt_kprintf("keya: 0x%x%x%x%x%x%x \n",card_arg.jskeya[0],card_arg.jskeya[1],card_arg.jskeya[2],
										 card_arg.jskeya[3],card_arg.jskeya[4],card_arg.jskeya[5]);
	rt_kprintf("keyb: 0x%x%x%x%x%x%x \n",card_arg.jskeyb[0],card_arg.jskeyb[1],card_arg.jskeyb[2],
										 card_arg.jskeyb[3],card_arg.jskeyb[4],card_arg.jskeyb[5]);
	
	rt_free(root);
	return card_arg.typ+1;
}

uint8_t read_json_from_card(void)
{
	uint16_t json_len;
	uint8_t *pjson, *chk, ts;
	
	if(card_into_head()==0) return 0;
	if(card_type[0]!=0x04 || card_type[1]!=0x00) return 0;  //判定卡
	if(pcd_auth_state_ex(PICC_AUTHENT1A,1,(unsigned char*)FACTORY_KEY_A,card_id)!=MI_OK)  return 0; //验证密码通过,读json长度

	uint8_t* rd_buf = rt_malloc(16);
	pcd_read_ex(1,rd_buf);   //读第一扇区块1
	json_len = (uint16_t)rd_buf[0]<<8 | rd_buf[1];    //拼接，获取json字符串长度
	if(json_len>500)  {rt_free(rd_buf); return 0;}
	pcd_read_ex(2,rd_buf);   //读第一扇区块2，得到MD5
	if(pcd_auth_state_ex(PICC_AUTHENT1B,1,(unsigned char*)FACTORY_KEY_B,card_id)!=MI_OK) goto exit2;  //验证密码通过,读json信息
	pjson = rt_malloc(json_len);    //申请内存
	ts = 5;                         //开始读json
	do{
		if(json_len>16) json_len-=16; else json_len = 0;
		if(pcd_read_ex(ts-1,pjson)!=MI_OK) goto exit1;    //从块4读起（第5块）
		pjson+=16; ts++;
		if(ts%4 == 0) ts++;
	}while(json_len>0);
	ts = decode_json((char*)pjson); //解析json信息,成功: ts=卡类+1，失败: ts=0  
	chk = rt_malloc(16);	//验证MD5
	creat_md5_val(card_arg.jskeya,card_arg.jskeyb,(char*)pjson,chk);  //这里是否应该用json解析出来的密钥待分析
	if(rt_memcmp(rd_buf,chk,16)!=0)  ts = 0;   //校验不成功
	rt_free(chk);
exit1:  rt_free(pjson);
exit2:  rt_free(rd_buf);
	    return ts;
}

static char* card_make_inf(uint8_t typ)   //卡信息结构体赋值，调用函数前已经判定编号输入是否有效
{
	char *strjson;

	card_arg.typ = typ;                        //卡类型，传入的参数
	rt_memcpy(card_arg.jskeya,sys_key_a,6);    //系统工作密钥对
	rt_memcpy(card_arg.jskeyb,sys_key_b,6);
	rt_memcpy(card_arg.jsiden,card_iden,sizeof(card_iden)); //编号，用户输入 
	create_rnd_str(16,card_pwd);               //生成随机密码，结果在card_pwd数组内，16字节
	rt_memcpy(card_arg.jspwd,card_pwd,16);     //密码拷贝至结构体相应成员
	strjson = creat_json(&card_arg);           //生成json信息
	return strjson;              //使用该函数后需要释放内存
}

/*******************************************************************************************/
/*
 *  密钥卡的初始化，读写等程序
 *
 */

void key_card_init(void)
{
	if(card_init(ROOT_CARD))
	{
		rt_kprintf("key card reset ok ! \n");
	}
	else rt_kprintf("key card reset faulted ! \n");    //仅空白卡可以初始化，成功后卡内仅有A,B密钥及控制信息
}

#include "na_queue.h"

void test_get_byid(void)
{
	uint8_t sysinfo_id;
	sysinfo_t sysinfo;
	
	sysinfo_id = 1; 
	if(sysinfo_get_by_id(&sysinfo, sysinfo_id)) //读系统数据库信息
	{
		rt_kprintf("id:%d\nsys_title:%s\nopen_timeout:%d\nnode_count:%d\ndoor_count:%d\n",\
		sysinfo.id, sysinfo.sys_title, sysinfo.open_timeout,sysinfo.node_count, sysinfo.door_count);
		rt_kprintf("keya:%02x%02x%02x%02x%02x%02x\nkeyb:%02x%02x%02x%02x%02x%02x",\
		sysinfo.key_a[0],sysinfo.key_a[1],sysinfo.key_a[2],sysinfo.key_a[03],\
		sysinfo.key_a[04],sysinfo.key_a[5],sysinfo.key_b[0],sysinfo.key_b[1],sysinfo.key_b[2],\
		sysinfo.key_b[3],sysinfo.key_b[4],sysinfo.key_b[5]);
	}		
}
MSH_CMD_EXPORT(test_get_byid, test get byid);

void key_card_make(void)
{
	uint8_t i,ts,sysinfo_id;
	sysinfo_t sysinfo;
	char *pjson;
	
	sysinfo_id = 1; ts = 0;
/*------  读取系统信息为0，认为是新系统或者系统前期所有信息已经被人为作废（删除系统信息）------*/
	if(sysinfo_get_by_id(&sysinfo, sysinfo_id)) //读系统数据库信息
	{
		for(i=0;i<6;i++)                   //有信息则检查密钥对,密钥对不允许有任何一个字节为0xFF？
		{
			if(sysinfo.key_a[i]!=0xFF) ts++;  if(sysinfo.key_b[i]!=0xFF) ts++;
		}
		if(ts==0)       //没有密钥
		{
			creat_sys_key();        //生成系统工作密钥，函数直接给全局系统工作密钥赋值
			rt_memcpy(sysinfo.key_a,sys_key_a,6);
			rt_memcpy(sysinfo.key_b,sys_key_b,6);
			sysinfo.id = sysinfo_id;
			sysinfo_update(&sysinfo);
		}
	}
	else
	{
		creat_sys_key();        //生成系统工作密钥，函数直接给全局系统工作密钥赋值
		rt_memcpy(sysinfo.key_a,sys_key_a,6);
		rt_memcpy(sysinfo.key_b,sys_key_b,6);
		rt_strncpy(sysinfo.sys_title, SYS_TITLE, rt_strlen(SYS_TITLE));
		sysinfo.id = sysinfo_id;
		sysinfo_add(&sysinfo);   //更新 还有三项内容在操作lcd<银柜设置>里进行更新
	}
/*---------------------------------------------------------------------------------------------*/
	if(read_json_from_card())      //读取json信息并解析
	{
		if(rt_memcmp(card_arg.jskeya,sysinfo.key_a,6)==0 && rt_memcmp(card_arg.jskeyb,sysinfo.key_b,6)==0)
		return;//如果一致，不需要更新
	}
	rt_memcpy(card_arg.jskeya,sys_key_a,6);
	rt_memcpy(card_arg.jskeyb,sys_key_b,6);
	card_arg.typ = ROOT_CARD;
	pjson=creat_json(&card_arg);    //创建json信息，系统密钥本就一直存在或者在该函数内前部生成
	if(pjson!=RT_NULL)
	{
		if(write_inf_card(ROOT_CARD,(uint8_t*)pjson,rt_strlen(pjson)))    //写json信息，函数内会生成0扇区数据并写入
			rt_kprintf("key card make ok ! \n");
		else rt_kprintf("key card make faulted ! \n");
	}
	else rt_kprintf("key card create json faulted ! \n");
	rt_free(pjson);
}

void key_card_clear(void)
{
	if(card_reset(ROOT_CARD))
		rt_kprintf("key card clear success, now your card is a space card ! \n");
	else rt_kprintf("your card clear wrong, may be space card, or init card,and may be other card ! \n");//只有密钥卡才可以重置，其他卡请在相应的卡管理程序内实现
}

void key_card_backup(void)       //备份
{
	key_card_make();  //系统密钥写入密钥卡。实际上在密钥卡制卡中就该完成该动作
}

void key_card_recovery(void)    //恢复数据库的密钥
{
	sysinfo_t sysinfo;
	if(read_json_from_card()==(ROOT_CARD+1)) //成功解析密钥卡,在密钥卡以及系统数据库信息丢失时，只要系统重新上电则永久不可恢复
	{  
		rt_memcpy(card_arg.jskeya,sys_key_a,6);
		rt_memcpy(card_arg.jskeyb,sys_key_b,6);
		if(sysinfo_get_by_id(&sysinfo,SYSINFO_DB_KEY_ID)>0)    //向数据库更新系统信息
		{
			sysinfo.id = SYSINFO_DB_KEY_ID;
			rt_memcpy(sysinfo.key_a,sys_key_a,6);
			rt_memcpy(sysinfo.key_b,sys_key_b,6);
			sysinfo_update(&sysinfo);                           //更新
			rt_kprintf("sysinfo recovery success !\n");
		}
		else rt_kprintf("sysinfo does not exist ! You need to create it !\n");
	}
	else rt_kprintf("the card is not a key card !\n");
}

/*
 *  根据参数查询数据库
 *  typ：   要查询的卡类型，传入0表示不根据该参数查询
 *  cardid: 要查询的卡ID，传入0表示不根据该参数查询
 *  numb：  需要返回的卡编号，多条记录时返回0
 *  返回值：查询到的记录数，可以判断查询是否成功
 */
static int find_card_in_db(uint8_t typ, int cardid, int numbr)
{
	int rcount=0;
	cardinfo_t * e = NULL;
	
	na_queue_t *q = rt_calloc(sizeof(cardinfo_t), 1);
	if(typ)   //如果类型不为0，密钥卡不需要查询
	{
		rcount = db_query_by_varpara("select * from cardinfo where type=?;", cardinfo_queue_bind, q, "%d", typ);
		numbr = 0;
	}
	else if(cardid)
	{
		rcount = db_query_by_varpara("select * from cardinfo where id=?;", cardinfo_queue_bind, q, "%d", cardid);
		if(rcount)
		{
			na_queue_foreach(e,q,cardinfo_t,queue) { numbr = e->num; }
		}
	}
	else
	{
		rt_kprintf("Input parameters are not valid !\n"); 
		numbr=0; 
	}
	cardinfo_free_queue(q); rt_free(q);
	return rcount;
}

/*******************************************************************************************/
/*
 *  卡的初始化，读写等程序
 *  参数传入卡类型
 */
void card_init_fun(uint8_t typ)
{
	char msg[16];

	switch(typ)
	{
		case SYS_COF_CARD: rt_strncpy(msg,"config ",7);  break;
		case MANAG_CARD  : rt_strncpy(msg,"manager ",8); break;
		case NORMNA_CARD : rt_strncpy(msg,"normna ",7);  break;
		default : rt_strncpy(msg,"other ",6); break;
	}
	if(card_init(typ))
	{
		rt_kprintf(msg); rt_kprintf("card init ok ! \n");
	}
	else {rt_kprintf(msg); rt_kprintf("card init faulted ! \n");}
}

void card_make_fun(uint8_t typ)   //收到LCD字符串存在card_iden数组内，制卡
{
	char *pjson, msg[16];
	int iden, rcount;
	
	rcount = find_card_in_db(typ,0,iden);  //查询数据库中该卡类型记录数量，这里iden变量仅临时使用，函数会将其置0
	/*  限制制卡数量 */
	switch(typ)
	{
		case SYS_COF_CARD: if(rcount>1) return;                           //配置卡
						   else  rt_strncpy(msg,"config ",7);  
						   break;           
		case MANAG_CARD  : if(rcount>4) return;                           //管理卡
						   else rt_strncpy(msg,"manager ",8); 
						   break;              
		case NORMNA_CARD : if(rcount>100) return;                         //钥匙卡
						   else  rt_strncpy(msg,"normna ",7); 
						   break;  
	}
	
	if(iden_over==0)             //如果没有生成编号
	{rt_kprintf("the serial number of card is not set ! \n"); return;}
	iden_over = 0;
	
	sscanf((char*)card_iden,"%d",&iden);        //编号，用户输入
	if(cardinfo_get_by_num(&cardinfo, iden))  return; //读数据库信息，有记录=1，无记录=0
		 
	pjson = card_make_inf(typ);  //返回json信息串，函数内生成卡信息结构体成员的值
	rt_memset(card_iden,0,16);
	if(write_inf_card(typ,(uint8_t*)pjson,rt_strlen(pjson)))  //写json信息，函数内部会处理并存储MD5以及json长度
	{
		rt_kprintf(msg); rt_kprintf("card make ok ! \n");
		cardinfo.type = typ;                     //卡类型：配置、管理、钥匙
		cardinfo.id   = card_id[0]<<24 | card_id[1]<<16 | card_id[2]<<8 | card_id[3];   //卡ID号
		cardinfo.num  = iden;                    //卡编号，用户输入
		rt_memcpy(cardinfo.pwd,card_pwd,16);     //卡密码在card_make_inf函数内已经生成，存储在card_pwd数组内
		cardinfo_add(&cardinfo);                 //添加
		rt_kprintf("card info add in database success !\n");
	}
	else { rt_kprintf(msg); rt_kprintf("config card make faulted ! \n"); }
	rt_free(pjson);
}

void card_clear_fun(uint8_t typ)    //执行卡重置
{
	char msg[16];
	int cardid, numb;

	switch(typ)
	{
		case SYS_COF_CARD: rt_strncpy(msg,"config ",7);  break;
		case MANAG_CARD  : rt_strncpy(msg,"manager ",8); break;
		case NORMNA_CARD : rt_strncpy(msg,"normna ",7);  break;
		default : rt_strncpy(msg,"other ",6); break;
	}

	if(card_reset(typ))
	{
		rt_kprintf(msg);
		rt_kprintf("card clear done ! \n");
		cardid=card_id[0]<<24 | card_id[1]<<16 | card_id[2]<<8 | card_id[3];
		if(find_card_in_db(0,cardid,numb)) cardinfo_del(numb);	  //数据库中找到该卡的记录则删除该记录
	}
	else {rt_kprintf(msg); rt_kprintf("card clear faulted ! \n");}
}

/*
 *  卡维护----删除数据库记录中的信息
 *  参数传入卡类型
 */
uint8_t card_service_fun(uint8_t typ)   //在数据库中查找符合条件的信息并删除
{
	uint8_t gs;
	int iden;
	
	if(iden_over==0)             //如果没有生成编号
	{rt_kprintf("the serial number of card is not input ! \n"); return 0;}
	iden_over = 0;
	if(rt_strncmp((char*)card_iden,"0000",4)==0)  //如果输入4个0表示删除全部记录
	{                                          //这里应该依据传参的卡类型删除所有该类型的记录，而不是所有卡记录
		cardinfo_del_all(); 
		return 1;
	}
	sscanf((char*)card_iden,"%d",&iden);       //取编号
	gs = cardinfo_get_by_num(&cardinfo, iden);    //依据输入的编号查询数据库的记录
	if(gs==0) {rt_kprintf("system is not record !\n");return 0;}  //如果没有记录，返回
	cardinfo_del(iden);                        //如果有则删除该条记录
	rt_memset(card_iden,0,16);
	return 1;
}

int check_cof_card_cun(void)
{
	int tmp;
	return find_card_in_db(SYS_COF_CARD,0,tmp);
}



