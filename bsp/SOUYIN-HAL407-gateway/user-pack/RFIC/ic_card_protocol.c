
//
#include <rtthread.h>
#include "ic_card_protocol.h"
#include "dev_rfic.h"
#include "datacheck.h"

#define CARD_INF_BLOCK_COUNT 18	//最大值45(15*3=45)

//const unsigned char key_a[6] = {0xcc, 0x26, 0x42, 0xac, 0x8f, 0x91};
//const unsigned char key_b[6] = {0xdd, 0x3c, 0xf7, 0x30, 0x45, 0xa8};

const unsigned char FACTORY_KEY_A[] = {0x25, 0xFD, 0xC4, 0x96, 0xAA, 0x06};
const unsigned char FACTORY_KEY_B[] = {0xA5, 0xF2, 0x3D, 0x90, 0xF7, 0x43};

//const unsigned char root_key_a[6] = {0x25, 0xFD, 0xC4, 0x96, 0xAA, 0x06};
unsigned char check_buf[16];
unsigned char check_buf_2[16];
unsigned char find_tag = 0;

unsigned char card_type[2];
unsigned char card_id[4];

const unsigned char card_default_ctrl[16] = {0xff,0xff,0xff,0xff,0xff,0xff,0x08,0x77,0x8f,0x69,0xff,0xff,0xff,0xff,0xff,0xff};
const unsigned char default_key[6] = {0xff,0xff,0xff,0xff,0xff,0xff};
const unsigned char default_data_block[16] = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};

const unsigned char card_inf_blocks[CARD_INF_BLOCK_COUNT] = {4,5,6,8,9,10,12,13,14,16,17,18,20,21,22,24,25,26};//,28,29,30,32,33,34,36,37,38,40,41,42,44,45,46,48,49,50,52,53,54,56,57,58,60,61,62

//以下定义用于写闸盒卡
//const unsigned char root_key_b[6] = {0xA5, 0xF2, 0x3D, 0x90, 0xF7, 0x43};
#define S70CardInfoSecCount 8
unsigned short S70CardInfBlockStart = 128;
unsigned char Write_Buffer[16];

//卡检测
unsigned char card_check(void)
{
	return (pcd_request_ex(PICC_REQALL, card_type)==MI_OK);
}

//查询卡
//outInf缓存最大45*16+1=721 加1字节结束符

extern rt_sem_t beep_sem;
/***********************************************************************************************************/
#include "drv_rng.h"

uint8_t sys_key_a[6] = {0};
uint8_t sys_key_b[6] = {0};    //系统密钥对，由密钥函数生成，并存入加密文件系统，这里暂时定义便于调试

void creat_sys_key(void)       //这里使用硬件随机数生成密钥对
{
	uint32_t tmp[3],i;
	for(i=0;i<3;i++)
	{
		tmp[i] = RNG_Get_RandomNum();
	}
	for(i=0;i<4;i++)
	{
		sys_key_a[i] = (tmp[0]&0xff)>>(8*i);
		sys_key_b[i] = (tmp[2]&0xff)>>(8*i);
	}
	sys_key_a[4] = tmp[1]&0xff;  sys_key_a[5] = (tmp[1]>>8)&0xff; 
	sys_key_b[4] = (tmp[1]>>16)&0xff; sys_key_b[5] = (tmp[1]>>24)&0xff;
	
	
}

static uint8_t card_into_head(void)
{
	char returnStatus;
	
	returnStatus = pcd_request_ex(PICC_REQALL, card_type);
	if(returnStatus != MI_OK)//寻卡获得卡类型
		returnStatus = pcd_request_ex(PICC_REQALL, card_type);
	if(returnStatus != MI_OK) return 0;	
	if(pcd_anticoll_ex(card_id) != MI_OK)  return 0;//读卡号
	if(pcd_select_ex(card_id)!=MI_OK) return 0;//选择卡
	else return 1;
}

/***********************************************************************************************************/

unsigned char rfid_scan_handle(unsigned char *card_id, unsigned char *outInf, unsigned short* outLength)  //返回卡应用类型
{
	char returnStatus;
	unsigned short i, inf_len, read_len=0;
	unsigned char checkTag = 1;

	*outLength = 0;
	if((returnStatus = pcd_request_ex(PICC_REQALL, card_type)) != MI_OK)
		returnStatus = pcd_request_ex(PICC_REQALL, card_type);
	if(returnStatus != MI_OK) find_tag = 0;  //寻卡获得卡类型
	
	if(returnStatus != MI_OK || find_tag)    return CardAppType_FF;		
	if(pcd_anticoll_ex(card_id) != MI_OK)    return CardAppType_FF;//读卡号
	if(pcd_select_ex(card_id)!=MI_OK)        return CardAppType_FF;//选择卡
	
	if(card_type[0]==0x04 && card_type[1]==0x00)
	{
		if(pcd_auth_state_ex(PICC_AUTHENT1A,CARD_APP_CHECK_BLOCK,(unsigned char*)sys_key_a,card_id)==MI_OK)
		{
			if(pcd_read_ex(CARD_APP_CHECK_BLOCK, check_buf)!=MI_OK) return CardAppType_FF;
			if(pcd_read_ex(CARD_APP_LEN_BLOCK, check_buf_2)!=MI_OK) return CardAppType_FF;
			
			inf_len = check_buf_2[0];inf_len <<= 8;inf_len |= check_buf_2[1];
			if(inf_len>CARD_INF_BLOCK_COUNT*16) inf_len = CARD_INF_BLOCK_COUNT*16;
//					rt_kprintf("inf_len=%d\r\n",inf_len);
			for(i=0;i<CARD_INF_BLOCK_COUNT;i++)
			{
				if(i%3==0){
					if(pcd_auth_state_ex(PICC_AUTHENT1A,card_inf_blocks[i],(unsigned char*)sys_key_a,card_id)!=MI_OK){
//								rt_kprintf("pcd_auth_state_ex error\r\n");
						return CardAppType_FF;
					}
				}
				if(pcd_read_ex(card_inf_blocks[i], outInf+(i*16))!=MI_OK){
					rt_kprintf("pcd_read_ex error\r\n"); return CardAppType_FF;
				}
				read_len += 16; if(read_len>=inf_len)break;
			}
		}
		else{  //	rt_kprintf("PcdAuthState CARD_APP_CHECK_BLOCK error\r\n");
			return CardAppType_FF; 
		}
		check_buf_2[0] = get_crc8(outInf, inf_len);
		check_buf_2[1] = get_sum(outInf, inf_len);
		check_buf_2[2] = get_xor(outInf, inf_len);
		for(i=0;i<3;i++){
			if(check_buf_2[i]!=check_buf[i]) {checkTag = 0;break;}
		}
		if(checkTag==1){//校验正确
			*outLength = inf_len;  outInf[inf_len] = 0x00;//给字符串添加结束符
			find_tag = 1;  rt_kprintf("card check OK ! \r\n");
			rt_sem_release(beep_sem);
			return CardAppType_01;
		}
	}
	else if(card_type[0]==0x08 && card_type[1]==0x00) { find_tag = 1;  return CardAppType_FD; }  //0x0800 = Mifare_Pro(X)
	return CardAppType_FF;
}//
//初始化卡
unsigned char card_init(uint8_t typ)
{
	unsigned char writeState = 0, *in_use_key;
	unsigned short i;
	
	rt_memcpy(check_buf,card_default_ctrl,16); 
	
	if(typ==ROOT_CARD)
	{
		rt_memcpy(check_buf,FACTORY_KEY_A,6);
		rt_memcpy(check_buf+10,FACTORY_KEY_B,6);
		in_use_key = (unsigned char*)FACTORY_KEY_B;
	}		
	else
	{
		rt_memcpy(check_buf,sys_key_a,6);
		rt_memcpy(check_buf+10,sys_key_b,6);
		in_use_key = (unsigned char*)sys_key_b;
	}		

	if(card_into_head()==0) return 0;      //选卡失败退出

	for(i=0;i<CARD_INF_BLOCK_COUNT;i++)
	{
		if(i%3==0)
		{
			if(pcd_auth_state_ex(PICC_AUTHENT1B,card_inf_blocks[i],in_use_key,card_id)==MI_OK)   //使用密码B验证通过
			{
				if(pcd_write_ex(card_inf_blocks[i]+3,check_buf)==MI_OK)  writeState++;  
				else break;
			}
			else break;
		}					
	}
	if(pcd_auth_state_ex(PICC_AUTHENT1B,CARD_APP_CHECK_BLOCK,in_use_key,card_id)==MI_OK)   //使用密码B验证通过
	{
		if(pcd_write_ex(CARD_APP_CHECK_CTRL_BLOCK,check_buf)==MI_OK)  writeState++;
	}
	if(writeState==CARD_INF_BLOCK_COUNT/3+1)   return 1;
	else return 0;
}
//重置卡
unsigned char card_reset(uint8_t typ)
{
	unsigned short i;
	unsigned char writeState = 0, *in_use_key;
	
	if(card_into_head()==0) return 0;   //选卡失败退出
	
	if(typ==ROOT_CARD) in_use_key = (unsigned char*)FACTORY_KEY_B;
	else in_use_key = (unsigned char*)sys_key_b;
	
	if(pcd_auth_state_ex(PICC_AUTHENT1B,CARD_APP_CHECK_BLOCK,in_use_key,card_id)==MI_OK)   //使用默认密码B验证通过
	{
		if(pcd_write_ex(CARD_APP_CHECK_BLOCK,(unsigned char*)default_data_block)==MI_OK)      writeState++;
		if(pcd_write_ex(CARD_APP_CHECK_BLOCK+1,(unsigned char*)default_data_block)==MI_OK)    writeState++;
		if(pcd_write_ex(CARD_APP_CHECK_CTRL_BLOCK,(unsigned char*)card_default_ctrl)==MI_OK)  writeState++;
	}
	for(i=0;i<CARD_INF_BLOCK_COUNT;i++)
	{
		if(i%3==0)
		{
			if(i>0)
			{
				if(pcd_write_ex(card_inf_blocks[i]-1,(unsigned char*)card_default_ctrl)==MI_OK) writeState++;
				else  break;
			}
			if(pcd_auth_state_ex(PICC_AUTHENT1B,card_inf_blocks[i],in_use_key,card_id)==MI_OK)  //使用密码B验证通过
			{
				if(pcd_write_ex(card_inf_blocks[i],(unsigned char*)default_data_block)==MI_OK) writeState++;
				else break;
			}
			else  break;
		}
		else
		{
			if(pcd_write_ex(card_inf_blocks[i],(unsigned char*)default_data_block)==MI_OK)  writeState++;
			else  break;
		}
	}
	if(pcd_write_ex(card_inf_blocks[CARD_INF_BLOCK_COUNT-1]+1,(unsigned char*)card_default_ctrl)==MI_OK) writeState++;
	if(writeState==((CARD_INF_BLOCK_COUNT/3)*4)+3) return 1;				
	else return 0;
}
//写信息卡
unsigned char write_inf_card(uint8_t typ, unsigned char* dat,unsigned short length,unsigned char* out_card_id)
{
	unsigned short i, len;
	unsigned char *in_use_key;

	if(typ==ROOT_CARD) in_use_key = (unsigned char*)FACTORY_KEY_B;
	else in_use_key = (unsigned char*)sys_key_b;
	
	len = length;
	
	if(card_into_head()==0) return 0;    //选卡失败退出
	
	if(pcd_auth_state_ex(PICC_AUTHENT1B,CARD_APP_CHECK_BLOCK,(unsigned char*)in_use_key,card_id)!=MI_OK)  return 0; //使用密码B验证通过

	rt_memset(check_buf,0xFF,16);
	check_buf[0] = get_crc8(dat, len);
	check_buf[1] = get_sum(dat, len);
	check_buf[2] = get_xor(dat, len);
	rt_memset(check_buf_2,0xFF,16);
	check_buf_2[0] = len>>8;  check_buf_2[1] = len&0xff;
	if(pcd_write_ex(CARD_APP_CHECK_BLOCK,check_buf)!=MI_OK || pcd_write_ex(CARD_APP_LEN_BLOCK,check_buf_2)!=MI_OK) return 0;
	
	for(i=0;i<CARD_INF_BLOCK_COUNT;i++)
	{
		if(i%3==0)
		{
			if(pcd_auth_state_ex(PICC_AUTHENT1B,card_inf_blocks[i],(unsigned char*)in_use_key,card_id)!=MI_OK) return 0; //使用密码B验证未通过
		}
		if(len>=16)
		{ rt_memcpy(check_buf,dat+(16*i),16); len-=16; }   //将传入数据拷贝至check_buf
		else
		{
			rt_memset(check_buf,0xFF,16);
			if(len>0) { rt_memcpy(check_buf,dat+(16*i),len); len = 0; }
		}
		if(pcd_write_ex(card_inf_blocks[i],check_buf)!=MI_OK) return 0;    //将数据写入卡中
		if(len==0) 
		{ rt_memcpy(out_card_id,card_id,4);  return 1; }
	}
	return 0;
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


