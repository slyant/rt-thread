
//
#include <rtthread.h>
#include "ic_card_protocol.h"
#include "dev_rfic.h"
#include "datacheck.h"

#define CARD_APP_CHECK_BLOCK			1	//卡签名数据块
#define CARD_APP_LEN_BLOCK				2	//卡信息长度数据块
#define CARD_APP_CHECK_CTRL_BLOCK	    3	//卡签名控制数据块

const unsigned char card_default_ctrl[16] = {0xff,0xff,0xff,0xff,0xff,0xff,0x08,0x77,0x8f,0x69,0xff,0xff,0xff,0xff,0xff,0xff};
const unsigned char default_key[6] = {0xff,0xff,0xff,0xff,0xff,0xff};
const unsigned char default_data_block[16] = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
const unsigned char key_a[6] = {0xcc, 0x26, 0x42, 0xac, 0x8f, 0x91};
const unsigned char key_b[6] = {0xdd, 0x3c, 0xf7, 0x30, 0x45, 0xa8};
unsigned char check_buf[16];
unsigned char check_buf_2[16];
unsigned char card_type[2];
unsigned char card_id[4];
unsigned char find_tag = 0;
#define CARD_INF_BLOCK_COUNT 18	//最大值45(15*3=45)
const unsigned char card_inf_blocks[CARD_INF_BLOCK_COUNT] = {4,5,6,8,9,10,12,13,14,16,17,18,20,21,22,24,25,26};//,28,29,30,32,33,34,36,37,38,40,41,42,44,45,46,48,49,50,52,53,54,56,57,58,60,61,62

//卡检测
unsigned char card_check(void)
{
	return (pcd_request_ex(PICC_REQALL, card_type)==MI_OK);
}
//查询卡
//outInf缓存最大45*16+1=721 加1字节结束符

extern rt_sem_t beep_sem;

unsigned char rfid_scan_handle(unsigned char *out_card_id, unsigned char *outInf, unsigned short* outLength)
{
	char returnStatus;
	unsigned short i;
	unsigned char checkTag = 1;
	unsigned short inf_len;
	unsigned short read_len = 0;
	*outLength = 0;
	if((returnStatus = pcd_request_ex(PICC_REQALL, card_id)) != MI_OK)
		returnStatus = pcd_request_ex(PICC_REQALL, card_id);
	if(returnStatus != MI_OK){//寻卡获得卡类型
		find_tag = 0;
	}
	if(returnStatus == MI_OK && find_tag==0)
	{		
		if(pcd_anticoll_ex(card_id) == MI_OK)//读卡号
		{
			if(pcd_select_ex(card_id)==MI_OK)//选择卡
			{
				if(pcd_auth_state_ex(PICC_AUTHENT1A,CARD_APP_CHECK_BLOCK,(unsigned char*)default_key,card_id)==MI_OK)  //======key_a
				{
					if(pcd_read_ex(CARD_APP_CHECK_BLOCK, check_buf)!=MI_OK){
						return 0;
					}
					if(pcd_read_ex(CARD_APP_LEN_BLOCK, check_buf_2)!=MI_OK){
						return 0;
					}
					
					inf_len = check_buf_2[0];inf_len <<= 8;inf_len |= check_buf_2[1];
					if(inf_len>CARD_INF_BLOCK_COUNT*16) inf_len = CARD_INF_BLOCK_COUNT*16;
//					rt_kprintf("inf_len=%d\r\n",inf_len);
					for(i=0;i<CARD_INF_BLOCK_COUNT;i++){
						if(i%3==0){
							if(pcd_auth_state_ex(PICC_AUTHENT1A,card_inf_blocks[i],(unsigned char*)default_key,card_id)!=MI_OK){  /////key_a
//								rt_kprintf("pcd_auth_state_ex error\r\n");
								return 0;
							}
						}
						if(pcd_read_ex(card_inf_blocks[i], outInf+(i*16))!=MI_OK){
							rt_kprintf("pcd_read_ex error\r\n");
							return 0;
						}
						read_len += 16;
						if(read_len>=inf_len)break;
					}
				}
				else{
					rt_kprintf("PcdAuthState CARD_APP_CHECK_BLOCK error\r\n");
					return 0;
				}
				check_buf_2[0] = get_crc8(outInf, inf_len);
				check_buf_2[1] = get_sum(outInf, inf_len);
				check_buf_2[2] = get_xor(outInf, inf_len);
				for(i=0;i<3;i++){
					if(check_buf_2[i]!=check_buf[i]){
						checkTag = 0;break;
					}
				}
				if(checkTag==1){//校验正确
                    rt_memcpy(out_card_id,card_id,4);
					*outLength = inf_len;
					outInf[inf_len] = 0x00;//给字符串添加结束符
					find_tag = 1;
                    rt_kprintf("card check OK ! \r\n");
					rt_sem_release(beep_sem);
					return 1;
				}
                else
                {
//                    printf("校验错误\r\n");
                }
			}
		}
	}	
	return 0;
}//
//初始化卡
unsigned char card_init(void)
{
	unsigned char writeState = 0;
	unsigned short i;
	char returnStatus;
	rt_memcpy(check_buf,card_default_ctrl,16);    /////////////key_a
	rt_memcpy(check_buf,default_key,6);
	rt_memcpy(check_buf+10,key_b,6);
	returnStatus = pcd_request_ex(PICC_REQALL, card_type);
	if(returnStatus != MI_OK)//寻卡获得卡类型
		returnStatus = pcd_request_ex(PICC_REQALL, card_type);
	if(returnStatus == MI_OK)
	{		
		if(pcd_anticoll_ex(card_id) == MI_OK)//读卡号
		{
			if(pcd_select_ex(card_id)==MI_OK)//选择卡
			{
				for(i=0;i<CARD_INF_BLOCK_COUNT;i++)
				{
					if(i%3==0)
					{
						if(pcd_auth_state_ex(PICC_AUTHENT1B,card_inf_blocks[i],(unsigned char*)default_key,card_id)==MI_OK)   //使用默认密码B验证通过
						{
							if(pcd_write_ex(card_inf_blocks[i]+3,check_buf)==MI_OK)  writeState++;  else break;
						}
						else break;
					}					
				}
				if(pcd_auth_state_ex(PICC_AUTHENT1B,CARD_APP_CHECK_BLOCK,(unsigned char*)default_key,card_id)==MI_OK)   //使用默认密码B验证通过
				{
					if(pcd_write_ex(CARD_APP_CHECK_CTRL_BLOCK,check_buf)==MI_OK)  writeState++;
				}
				if(writeState==CARD_INF_BLOCK_COUNT/3+1)   return 1;
			}
		}
	}
	return 0;
}
//重置卡
unsigned char card_reset(void)
{
	unsigned short i;
	unsigned char writeState = 0;
	char returnStatus;
	returnStatus = pcd_request_ex(PICC_REQALL, card_type);
	if(returnStatus != MI_OK)//寻卡获得卡类型
		returnStatus = pcd_request_ex(PICC_REQALL, card_type);
	if(returnStatus == MI_OK)
	{		
		if(pcd_anticoll_ex(card_id) == MI_OK)//读卡号
		{
			if(pcd_select_ex(card_id)==MI_OK)//选择卡
			{
				if(pcd_auth_state_ex(PICC_AUTHENT1B,CARD_APP_CHECK_BLOCK,(unsigned char*)key_b,card_id)==MI_OK){//使用默认密码B验证通过
					if(pcd_write_ex(CARD_APP_CHECK_BLOCK,(unsigned char*)default_data_block)==MI_OK){
						writeState++;
					}
					if(pcd_write_ex(CARD_APP_CHECK_BLOCK+1,(unsigned char*)default_data_block)==MI_OK){
						writeState++;
					}
					if(pcd_write_ex(CARD_APP_CHECK_CTRL_BLOCK,(unsigned char*)card_default_ctrl)==MI_OK){
						writeState++;
					}
				}
				for(i=0;i<CARD_INF_BLOCK_COUNT;i++)
				{
					if(i%3==0)
					{
						if(i>0){
							if(pcd_write_ex(card_inf_blocks[i]-1,(unsigned char*)card_default_ctrl)==MI_OK){
								writeState++;
							}
							else
							{
								break;
							}
						}
						if(pcd_auth_state_ex(PICC_AUTHENT1B,card_inf_blocks[i],(unsigned char*)key_b,card_id)==MI_OK){//使用密码B验证通过
							if(pcd_write_ex(card_inf_blocks[i],(unsigned char*)default_data_block)==MI_OK){
								writeState++;
							}
							else
							{
								break;
							}
						}
						else
						{
							break;
						}
					}
					else{
						if(pcd_write_ex(card_inf_blocks[i],(unsigned char*)default_data_block)==MI_OK){
							writeState++;
						}
						else
						{
							break;
						}
					}
				}
				if(pcd_write_ex(card_inf_blocks[CARD_INF_BLOCK_COUNT-1]+1,(unsigned char*)card_default_ctrl)==MI_OK){
					writeState++;
				}
				if(writeState==((CARD_INF_BLOCK_COUNT/3)*4)+3)
				{
					return 1;
				}				
			}
		}
	}
	return 0;
}
//写信息卡
unsigned char write_inf_card(unsigned char* dat,unsigned short length,unsigned char* out_card_id)
{
	unsigned short i;
	unsigned short len;
	char returnStatus;
	len = length;
	returnStatus = pcd_request_ex(PICC_REQALL, card_type);
	if(returnStatus != MI_OK)//寻卡获得卡类型
		returnStatus = pcd_request_ex(PICC_REQALL, card_type);
	if(returnStatus == MI_OK)
	{		
		if(pcd_anticoll_ex(card_id) == MI_OK)//读卡号
		{
			if(pcd_select_ex(card_id)==MI_OK)//选择卡
			{
				if(pcd_auth_state_ex(PICC_AUTHENT1B,CARD_APP_CHECK_BLOCK,(unsigned char*)key_b,card_id)==MI_OK){//使用密码B验证通过
					rt_memset(check_buf,0xFF,16);
					check_buf[0] = get_crc8(dat, len);
					check_buf[1] = get_sum(dat, len);
					check_buf[2] = get_xor(dat, len);
					rt_memset(check_buf_2,0xFF,16);
					check_buf_2[0] = len>>8;check_buf_2[1] = len&0xff;
					if(pcd_write_ex(CARD_APP_CHECK_BLOCK,check_buf)!=MI_OK || pcd_write_ex(CARD_APP_LEN_BLOCK,check_buf_2)!=MI_OK)
					{
						return 0;
					}
                    for(i=0;i<CARD_INF_BLOCK_COUNT;i++)
                    {
                        if(i%3==0)
                        {
                            if(pcd_auth_state_ex(PICC_AUTHENT1B,card_inf_blocks[i],(unsigned char*)key_b,card_id)!=MI_OK){//使用密码B验证未通过
                                return 0;
                            }
                        }
                        if(len>=16)
                        {
                            rt_memcpy(check_buf,dat+(16*i),16);
                            len-=16;
                        }
                        else
                        {
                            rt_memset(check_buf,0xFF,16);
                            if(len>0)
                            {
                                rt_memcpy(check_buf,dat+(16*i),len);
                                len = 0;
                            }
                        }
                        if(pcd_write_ex(card_inf_blocks[i],check_buf)!=MI_OK)
                        {
                            return 0;
                        }
                        if(len==0)
                        {
                            rt_memcpy(out_card_id,card_id,4);
                            return 1;
                        }
                    }
				}
			}
		}
	}
	return 0;
}
