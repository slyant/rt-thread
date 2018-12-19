#include <rtthread.h>
#include <tinycrypt.h>
#include "dev_rfic.h"
#include "drv_rng.h"
#include "ic_card_protocol.h"

#define BLOCK_SIZE				16
#define CARD_INF_BLOCK_COUNT	18	//最大值45(15*3=45)
#define KEY_LENGTH				6
#define SIGNATURE_LENGTH		16
const unsigned char factory_signature[SIGNATURE_LENGTH] = {0x61, 0x6D, 0x50, 0x75, 0x38, 0x39, 0x52, 0x33, 0x47, 0x44, 0x77, 0x78, 0x21, 0x7E, 0x6A, 0x23};
const unsigned char factory_key_a[KEY_LENGTH] = {0x25, 0xFD, 0xC4, 0x96, 0xAA, 0x06};
const unsigned char factory_key_b[KEY_LENGTH] = {0xA5, 0xF2, 0x3D, 0x90, 0xF7, 0x43};
const unsigned char default_data_block[BLOCK_SIZE] = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
const unsigned char card_default_ctrl[BLOCK_SIZE] = {0xff,0xff,0xff,0xff,0xff,0xff,0x08,0x77,0x8f,0x69,0xff,0xff,0xff,0xff,0xff,0xff};
const unsigned char default_key[KEY_LENGTH] = {0xff,0xff,0xff,0xff,0xff,0xff};
const unsigned char card_inf_blocks[CARD_INF_BLOCK_COUNT] = {4,5,6,8,9,10,12,13,14,16,17,18,20,21,22,24,25,26};//,28,29,30,32,33,34,36,37,38,40,41,42,44,45,46,48,49,50,52,53,54,56,57,58,60,61,62

/*
function: 比较2个地址的值在指定的长度内是否相同
param: 
a: 第一个地址
b: 第二个地址
length: 要比较的长度
*/
rt_bool_t compare_bytes(rt_uint8_t *a, rt_uint8_t *b, rt_uint16_t length)
{
	while(length--)
	{
		if(a++ != b++)return RT_FALSE;
	}
	return RT_TRUE;
}
/*
function: 扫描IC卡,并返回扫描结果
param:
in_key_a: in 应用卡A钥
in_key_b: in 应用卡B钥
out_buffer: out 应用卡数据,传入空指针,在外部释放
out_buffer_len: out 应用卡数据长度
out_card_id： out 卡ID 4Byte
out_card_base_type: out 卡基础类型
return: RT_TRUE or RT_FALSE
*/
rt_bool_t rfid_scan_handle(rt_uint8_t *in_key_a, rt_uint8_t *in_key_b, rt_uint8_t *out_buffer, rt_uint16_t *out_buffer_len, rt_uint8_t *out_card_id, card_base_type_t *out_card_base_type)
{
	static rt_uint8_t find_tag;
	rt_uint8_t status, block_count, result = RT_FALSE;	
	rt_uint16_t i, inf_len;
	rt_uint8_t check_buf[BLOCK_SIZE*2];//用于存放CARD_CHECK_BLOCK和CARD_LEN_BLOCK/MD5结果

	*out_buffer_len = 0;
	if((status = pcd_request_ex(PICC_REQALL, out_card_id)) != MI_OK)
		status = pcd_request_ex(PICC_REQALL, out_card_id);
	if(status != MI_OK){//寻卡获得卡类型
		find_tag = 0;
	}
	if(status == MI_OK && find_tag == 0)
	{		
		if(pcd_anticoll_ex(out_card_id) == MI_OK)//读卡号
		{
			if(pcd_select_ex(out_card_id) == MI_OK)//选择卡
			{
				if(pcd_auth_state_ex(PICC_AUTHENT1A, CARD_CHECK_BLOCK, default_key, out_card_id) == MI_OK)
				{//空卡
					*out_card_base_type = CARD_TYPE_NULL;
					result = RT_TRUE;
				}
				else if(pcd_auth_state_ex(PICC_AUTHENT1A, CARD_CHECK_BLOCK, factory_key_a, out_card_id) == MI_OK)
				{//密钥卡
					*out_card_base_type = CARD_TYPE_KEY;
					if(pcd_read_ex(CARD_CHECK_BLOCK, check_buf) != MI_OK)
					{
						goto _EXIT;
					}
					if(pcd_read_ex(CARD_LEN_BLOCK, check_buf + BLOCK_SIZE) != MI_OK)
					{
						goto _EXIT;
					}					
					inf_len = check_buf[BLOCK_SIZE]; inf_len <<= 8; inf_len |= check_buf[BLOCK_SIZE + 1];
					if(inf_len > CARD_INF_BLOCK_COUNT * BLOCK_SIZE)
					{//超出最大长度
						goto _EXIT;
					}
					block_count = inf_len / BLOCK_SIZE + (inf_len % BLOCK_SIZE > 0 ? 1:0);//实际在读取的数据块数量
					//注意要在外部释放此空间
					out_buffer = rt_calloc(1, inf_len + (KEY_LENGTH * 2) + SIGNATURE_LENGTH);
					for(i=0; i<block_count; i++)
					{
						if(i % 3 == 0){
							if(pcd_auth_state_ex(PICC_AUTHENT1A, card_inf_blocks[i], factory_key_a, out_card_id) != MI_OK)
							{
								goto _EXIT;
							}
						}
						if(pcd_read_ex(card_inf_blocks[i], out_buffer + (i * BLOCK_SIZE)) != MI_OK)
						{
							goto _EXIT;
						}
					}
					//校验卡
					rt_memcpy(out_buffer + inf_len, factory_key_a, KEY_LENGTH);
					rt_memcpy(out_buffer + inf_len + KEY_LENGTH, factory_key_b, KEY_LENGTH);
					rt_memcpy(out_buffer + inf_len + KEY_LENGTH + KEY_LENGTH, factory_signature, SIGNATURE_LENGTH);
					tiny_md5(out_buffer, inf_len + KEY_LENGTH + KEY_LENGTH + SIGNATURE_LENGTH, check_buf + BLOCK_SIZE);
					if(compare_bytes(check_buf, check_buf + BLOCK_SIZE, BLOCK_SIZE))
					{
						*out_buffer_len = inf_len;
						rt_uint8_t *new_temp = rt_realloc(out_buffer, inf_len);//重新分配内存，释放掉多余的内存
						if(new_temp != RT_NULL)
						{
							out_buffer = new_temp;
						}						
						result = RT_TRUE;
					}					
				}
				else if(pcd_auth_state_ex(PICC_AUTHENT1A, CARD_CHECK_BLOCK, in_key_a, out_card_id) == MI_OK)
				{//应用卡
					*out_card_base_type = CARD_TYPE_APP;
					if(pcd_read_ex(CARD_CHECK_BLOCK, check_buf) != MI_OK)
					{
						goto _EXIT;
					}
					if(pcd_read_ex(CARD_LEN_BLOCK, check_buf + BLOCK_SIZE) != MI_OK)
					{
						goto _EXIT;
					}					
					inf_len = check_buf[BLOCK_SIZE]; inf_len <<= 8; inf_len |= check_buf[BLOCK_SIZE + 1];
					if(inf_len > CARD_INF_BLOCK_COUNT * BLOCK_SIZE)
					{//超出最大长度
						goto _EXIT;
					}
					block_count = inf_len / BLOCK_SIZE + (inf_len % BLOCK_SIZE > 0 ? 1:0);//实际在读取的数据块数量
					//注意要在外部释放此空间
					out_buffer = rt_calloc(1, inf_len + (KEY_LENGTH * 2) + SIGNATURE_LENGTH);
					for(i=0; i<block_count; i++)
					{
						if(i % 3 == 0){
							if(pcd_auth_state_ex(PICC_AUTHENT1A, card_inf_blocks[i], in_key_a, out_card_id) != MI_OK)
							{
								goto _EXIT;
							}
						}
						if(pcd_read_ex(card_inf_blocks[i], out_buffer + (i * BLOCK_SIZE)) != MI_OK)
						{
							goto _EXIT;
						}
					}
					//校验卡
					rt_memcpy(out_buffer + inf_len, in_key_a, KEY_LENGTH);
					rt_memcpy(out_buffer + inf_len + KEY_LENGTH, in_key_b, KEY_LENGTH);
					rt_memcpy(out_buffer + inf_len + KEY_LENGTH + KEY_LENGTH, factory_signature, SIGNATURE_LENGTH);
					tiny_md5(out_buffer, inf_len + KEY_LENGTH + KEY_LENGTH + SIGNATURE_LENGTH, check_buf + BLOCK_SIZE);
					if(compare_bytes(check_buf, check_buf + BLOCK_SIZE, BLOCK_SIZE))
					{
						*out_buffer_len = inf_len;
						rt_uint8_t *new_temp = rt_realloc(out_buffer, inf_len);//重新分配内存，释放掉多余的内存
						if(new_temp != RT_NULL)
						{
							out_buffer = new_temp;
						}
						result = RT_TRUE;
					}					
				}				
				else
				{
					rt_kprintf("PcdAuthState CARD_APP_CHECK_BLOCK error\r\n");
					goto _EXIT;
				}
			}
		}
	}
_EXIT:
	return result;
}


//以下定义用于写闸盒卡
//const unsigned char root_key_b[6] = {0xA5, 0xF2, 0x3D, 0x90, 0xF7, 0x43};
#define S70CardInfoSecCount 8
unsigned short S70CardInfBlockStart = 128;
unsigned char Write_Buffer[16];
	
extern card_msg card_arg;

void create_rnd_str(int length,char* out_str)
{
	const char* _rndstr = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ\0";
	int i;
	for(i=0;i<length;i++)
	{
		out_str[i] = _rndstr[RNG_Get_RandomRange(0,(rt_strlen(_rndstr)-1))];
	}
}

//查询卡
//outInf缓存最大45*16+1=721 加1字节结束符

extern rt_sem_t beep_sem;
/***********************************************************************************************************/

rt_uint8_t sys_key_a[6] = {0};
rt_uint8_t sys_key_b[6] = {0};    //系统密钥对，由密钥函数生成，并存入加密文件系统，这里暂时定义便于调试

void creat_sys_key(void)       //这里使用硬件随机数生成密钥对
{
	char *tmp;
	tmp = rt_malloc(12);
	rt_memset(tmp,0,12);
	create_rnd_str(12,tmp);
	rt_memcpy(sys_key_a,tmp,6);
	rt_memcpy(sys_key_b,&tmp[6],6);
	rt_free(tmp);
}
MSH_CMD_EXPORT(creat_sys_key,creat sys key);

void creat_md5_val(rt_uint8_t *keya, rt_uint8_t *keyb, char *str_json, rt_uint8_t *val)
{
	int len;
	
	len = rt_strlen(str_json);
	len+=12;
	rt_uint8_t *str = rt_malloc(len);
	rt_memcpy(str,keya,6);
	rt_memcpy(&str[6],str_json,rt_strlen(str_json));
	rt_memcpy(&str[len-6],keyb,6);
	tiny_md5(str,len,val);
	rt_free(str);
}



static rt_uint8_t money_bag_init(rt_uint8_t*key)  //调用该函数之前已经为该扇区控制块赋值。无需再次写控制块
{
	rt_uint8_t i, *money_block, ts;
	ts=0;
	money_block=rt_malloc(16);
	if(pcd_auth_state_ex(PICC_AUTHENT1B,35,key,card_id)!=MI_OK)  goto exit_m;  //用B验证控制块通过
	rt_memset(money_block,0xFF,16);
	money_block[2] = 0x80;   //启用钱包对应反码字段赋值
	money_block[3] = 0;
	for(i=0;i<5;i++)
	{
		money_block[6+i] = (rt_uint8_t)RNG_Get_RandomRange(0,255);
	}
	if(money_block[6]&0x80)
	{
		money_block[11] = ~money_block[11] | 0x80;
		money_block[11] = ~money_block[11];
		money_block[12] = ~money_block[12];
		money_block[13] = ~money_block[13];
		money_block[14] = ~money_block[14];
		money_block[15] = ~money_block[15];
	}
	if(pcd_write_ex(32,money_block)!=MI_OK) goto exit_m;
	rt_memset(money_block,0,4);
	rt_memset(&money_block[4],0xFF,4);
	rt_memset(&money_block[8],0,4);
	money_block[12] = 33; money_block[13] = ~33|0x80;money_block[14] = 33; money_block[15] = ~33|0x80;
	if(pcd_write_ex(33,money_block)!=MI_OK)  goto exit_m;
	if(pcd_write_ex(34,money_block)!=MI_OK)  goto exit_m;
	ts=1;
exit_m:	rt_free(money_block); 
	return ts;
}

rt_uint8_t money_bag_handle(rt_uint8_t act)
{
	rt_uint8_t tmpbuf[16],mbuf[16];
	int val;
	if(card_into_head()==0) return 0;
	if(pcd_auth_state_ex(PICC_AUTHENT1A,CARD_APP_CHECK_BLOCK,(unsigned char*)sys_key_a,card_id)!=MI_OK) return 0;
	if(pcd_read_ex(32,tmpbuf)!=MI_OK) return 0;
	if(pcd_read_ex(33,mbuf)!=MI_OK) return 0;
	if(tmpbuf[0]==0xFF && tmpbuf[1]==0xFF)     //判定钱包是否启用
	{
		val = mbuf[0]<<24 | mbuf[1]<<16 | mbuf[2]<<8 | mbuf[3] ;
		if(act) 
		{
			if(val)  val = 0;				
		}
		else
		{
			val=1;
		}
		mbuf[0] = (val>>24)&0xFF; mbuf[1] = (val>>16)&0xFF; mbuf[2] = (val>>8)&0xFF; mbuf[3] = val&0xFF;
		mbuf[8] = (val>>24)&0xFF; mbuf[9] = (val>>16)&0xFF; mbuf[10] = (val>>8)&0xFF; mbuf[11] = val&0xFF;
		val = !val |0x80000000;
		mbuf[4] = (val>>24)&0xFF; mbuf[5] = (val>>16)&0xFF; mbuf[6] = (val>>8)&0xFF; mbuf[7] = val&0xFF;
		if(pcd_auth_state_ex(PICC_AUTHENT1B,CARD_APP_CHECK_BLOCK,(unsigned char*)sys_key_b,card_id)!=MI_OK) return 0;
		if(pcd_write_ex(33,mbuf)!=MI_OK) return 0;
		if(pcd_write_ex(34,mbuf)!=MI_OK) return 0;
	}
}

/***********************************************************************************************************/

rt_uint8_t rfid_scan_handle(void)  //返回卡应用类型
{
	unsigned short inf_len;
	unsigned char *in_use_key;
	rt_uint8_t card_typ_join, *chk, index;
	char *prd_json;

	card_typ_join = CardAppType_FF;
	
	if(card_into_head()==0) {find_tag=0; return CardAppType_FF;}
	if(find_tag) return CardAppType_FF;
	find_tag = 1;
	if(card_type[0]!=0x04 || card_type[1]!=0x00)
	{
		if(card_type[0]==0x08 && card_type[1]==0x00)     //0x0800 = Mifare_Pro(X)
		return CardAppType_FF;
	}
	
	card_typ_join = card_auth_find();
	if(card_typ_join == CardAppType_FF) return CardAppType_FF;
	else if(card_typ_join == SPACE_CARD) {rt_kprintf("find a space card induction zone !\n"); goto exit;}

	if(pcd_read_ex(CARD_APP_CHECK_BLOCK, check_buf)!=MI_OK) return CardAppType_FF;   //读块1
	if(pcd_read_ex(CARD_APP_LEN_BLOCK, check_buf_2)!=MI_OK) return CardAppType_FF;   //读块2
	if(check_buf_2[0]==0xFF && check_buf_2[1]==0xFF)  inf_len = 0; //检查json信息长度
	else
	{
		inf_len = check_buf_2[0];inf_len <<= 8;inf_len |= check_buf_2[1];       //拼接json信息长度
		if(inf_len>CARD_INF_BLOCK_COUNT*16) inf_len = CARD_INF_BLOCK_COUNT*16;  //如果长度比容量大还大则长度值为应用容量
	}
	rt_kprintf("json length = %d\r\n",inf_len);
	
	switch(card_typ_join)   //对不同类型卡使用不同密钥
	{
		case ROOT_CARD :    in_use_key = (unsigned char*)FACTORY_KEY_A;  break;
		case WAIT_TYP_CARD: in_use_key = (unsigned char*)sys_key_a;      break;
	}
	
	prd_json = rt_malloc(inf_len);    //申请内存
	card_typ_join = 4;                //开始读json,此处变量的意义已经改变，成为计数工具（索引）  
	index = 0;     
	do{
		if(card_typ_join%4 == 0)      //验证控制块密钥
		{
			if(pcd_auth_state_ex(PICC_AUTHENT1A,card_typ_join+3,in_use_key,card_id)!=MI_OK)  {rt_free(prd_json); return 0;}  //用B验证控制块通过
			else card_typ_join++;
		}
		if(inf_len>16) inf_len-=16; else inf_len = 0;
		if(pcd_read_ex(card_typ_join-1,((rt_uint8_t*)prd_json+(index<<4)))!=MI_OK) {rt_free(prd_json); return 0;}    //从块4读起（第5块）
		index++; card_typ_join++;
	}while(inf_len>0);                                                //直到读完json信息
	
	card_typ_join = decode_json(prd_json); //解析json信息,成功: card_typ_join=卡类+1，失败: ts=0 这里恢复变量的意义
	if(card_typ_join==1) goto free_out;               //是密钥卡退出,无需验证MD5
	chk = rt_malloc(16);	//验证MD5
	creat_md5_val(card_arg.jskeya,card_arg.jskeyb,(char*)prd_json,chk);  //这里是否应该用json解析出来的密钥比对待分析，因为数据库没有记录MD5值
	if(rt_memcmp(check_buf,chk,16)!=0)  card_typ_join = 0;   //校验不成功
	rt_free(chk);
free_out:	rt_free(prd_json);
exit:
	rt_sem_release(beep_sem);
	return card_typ_join-1;
}//

//初始化卡
unsigned char card_init(rt_uint8_t typ)  
{
	unsigned char writeState = 0, *in_use_key;
	rt_uint8_t i, index;
	
	rt_memcpy(check_buf,card_default_ctrl,16); //将漠然的控制块数据进行拷贝
	switch(typ)                                //根据卡类型填充密钥
	{
		case ROOT_CARD :  rt_memcpy(check_buf,FACTORY_KEY_A,6); rt_memcpy(check_buf+10,FACTORY_KEY_B,6); break;
		case SPACE_CARD:  rt_memcpy(check_buf,default_key,6);  rt_memcpy(check_buf+10,default_key,6);    break;
		default :         rt_memcpy(check_buf,sys_key_a,6);  rt_memcpy(check_buf+10,sys_key_b,6);        break;
	}
	in_use_key = (unsigned char*)default_key;  //所有卡进入函数之前必定（必须）是空白卡
	if(card_into_head()==0) return 0;          //选卡失败退出
	
	for(i=0;i<APP_USE_SECTOR_SIZE;i++)      //只使用9个扇区：9个控制块，共26个数据块(减掉第0块)
	{
		index = (i<<2) + 3;
		if(pcd_auth_state_ex(PICC_AUTHENT1B,index,in_use_key,card_id)==MI_OK)   //一般使用密码B验证通过，这里是空白卡进入，用default_key
		{
			if(pcd_write_ex(index,check_buf)==MI_OK)  writeState++;  else break;   //写控制块，从块3写起，后面7,11,15...
		}
	}
	rt_kprintf("write block: %d \n",writeState);
	if(writeState==APP_USE_SECTOR_SIZE)   return 1;
	else return 0;
}

//重置卡
unsigned char card_reset(rt_uint8_t typ)
{
	rt_uint8_t i,j,index,writeState = 0;
	unsigned char *in_use_key;
	
	if(card_into_head()==0) return 0;   //选卡失败退出
	
	switch(typ)   //进入函数的卡都已经设置了相应的密钥对，依据不同卡类型，提供应用密钥
	{
		case ROOT_CARD :     in_use_key = (unsigned char*)FACTORY_KEY_B; break;
		case SPACE_CARD:     in_use_key = (unsigned char*)default_key;   break;
		default :            in_use_key = (unsigned char*)sys_key_b;     break;
	}
	
	for(i=0;i<APP_USE_SECTOR_SIZE;i++)
	{
		index = 3 + (i<<2);
		if(pcd_auth_state_ex(PICC_AUTHENT1B,index,in_use_key,card_id)!=MI_OK)  return 0;   //用B验证控制块通过
		if(pcd_write_ex(index,(rt_uint8_t*)card_default_ctrl)!=MI_OK)   return 0;             //将控制块更新为默认控制权限
		for(j=0;j<3;j++)
		{
			if(i==0 && j==0)  j++;   //第0块跳过
			if(pcd_write_ex((i<<2)+j,(rt_uint8_t*)default_data_block)==MI_OK)       //写默认数据到数据块
				writeState++;
		}
	}
	if(typ==NORMNA_CARD)   //如果是钥匙卡，则重新对定义的最后一个扇区的数据块赋初始值
	{
		if(money_bag_init(in_use_key)==0) return 0; 
	}
	rt_kprintf("write block: %d \n",writeState);
	if(writeState==(APP_USE_SECTOR_SIZE*3-1)) return 1;	//7个块，每块3个数据块，扇区0少一个，因此总数应该是7×3-1
	else return 0;
}

//写信息卡
unsigned char write_inf_card(rt_uint8_t typ, unsigned char* dat,unsigned short length)
{
	unsigned short len;
	unsigned char *in_use_key, *mas_key, *pdat;
	rt_uint8_t index;

	switch(typ)
	{
		case ROOT_CARD :    in_use_key = (unsigned char*)FACTORY_KEY_B; mas_key=(unsigned char*)FACTORY_KEY_A; break;
		case SPACE_CARD:    in_use_key = (unsigned char*)default_key;   mas_key=(unsigned char*)default_key;   break;
		default:            in_use_key = (unsigned char*)sys_key_b;     mas_key=(unsigned char*)sys_key_a;     break;
	}
	
	len = length;
	
	if(card_into_head()==0) return 0;    //选卡失败退出
	
	if(pcd_auth_state_ex(PICC_AUTHENT1B,CARD_APP_CHECK_BLOCK,(unsigned char*)in_use_key,card_id)!=MI_OK)  return 0; //使用密码B验证通过--MD5块
	
	creat_md5_val(mas_key,in_use_key,(char*)dat,check_buf);                              //将密钥对与json信息拼接后生成MD5码
	check_buf_2[0]=rt_strlen((char*)dat)>>8; check_buf_2[1]=rt_strlen((char*)dat)&0xFF;  //准备扇区0的块1和2的数据
	
	if(pcd_write_ex(CARD_APP_CHECK_BLOCK,check_buf)!=MI_OK || pcd_write_ex(CARD_APP_LEN_BLOCK,check_buf_2)!=MI_OK) return 0;   //写块1和2
	
	pdat = dat;
	index=4;
	do{
		if(index%4==0)
		{
			if(pcd_auth_state_ex(PICC_AUTHENT1B,index+3,in_use_key,card_id)!=MI_OK)  return 0;   //用B验证控制块通过
			else index++;
		}
		if(len>=16) len-=16; else len=0;
		if(pcd_write_ex(index-1,pdat)!=MI_OK) return 0;    //从4开始写
		pdat+=16; index++;
	}while(len>0);
	
	if(typ==NORMNA_CARD)  money_bag_init(in_use_key);
	
	return 1;
}

//读闸盒卡ID
unsigned char QueryBoxCardID(unsigned char *card_id)
{
	char returnStatus;
	returnStatus = pcd_request_ex(PICC_REQALL, card_id);
	if(returnStatus != MI_OK)//寻卡获得卡类型
		returnStatus = pcd_request_ex(PICC_REQALL, card_id);
	if(returnStatus == MI_OK && card_id[0]==0x02 && card_id[1]==0x00)//仅S70卡有效
	{
		if(pcd_anticoll_ex(card_id) == MI_OK)//读卡号
		{
			return TRUE;
		}
	}
	return FALSE;
}
//写闸盒信息卡
unsigned char WriteBoxInfToCard(unsigned char *card_id,unsigned char* dat,unsigned short length)
{
	unsigned short i;
	if(pcd_select_ex(card_id)!=MI_OK)  return FALSE;//选择卡
	for(i=S70CardInfBlockStart;i<S70CardInfBlockStart+(S70CardInfoSecCount*16);i++)
	{
		if(i%16==0)
		{
			if(pcd_auth_state_ex(PICC_AUTHENT1B,i,(unsigned char*)sys_key_b,card_id)!=MI_OK) break;  //使用密码B验证未通过
		}
		else if(i%16==15) continue;
		
		if(length>=16)
		{
			rt_memcpy(Write_Buffer,dat,16);
			dat+=16;
			length-=16;
		}
		else
		{
			rt_memset(Write_Buffer,0xFF,16);
			if(length>0)
			{
				rt_memcpy(Write_Buffer,dat,length);
				dat+=length;
				length = 0;
			}
		}
		if(pcd_write_ex(i,Write_Buffer)==MI_OK)
		{
			if(length==0) return TRUE;
		}
		else break;
	}
	return FALSE;
}


