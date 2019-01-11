#include <rtthread.h>
#include <tinycrypt.h>
#include <drv_rfic.h>
#include <rng_helper.h>
#include <ic_card_protocol.h>

static const unsigned char factory_signature[SIGNATURE_LENGTH] = {0x61, 0x6D, 0x50, 0x75, 0x38, 0x39, 0x52, 0x33, 0x47, 0x44, 0x77, 0x78, 0x21, 0x7E, 0x6A, 0x23};
static const unsigned char factory_key_a[KEY_LENGTH] = {0x25, 0xFD, 0xC4, 0x96, 0xAA, 0x06};
static const unsigned char factory_key_b[KEY_LENGTH] = {0xA5, 0xF2, 0x3D, 0x90, 0xF7, 0x43};
static const unsigned char default_data_block[BLOCK_SIZE] = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
static const unsigned char default_data_ctrl[BLOCK_SIZE] = {0xff,0xff,0xff,0xff,0xff,0xff,0x08,0x77,0x8f,0x69,0xff,0xff,0xff,0xff,0xff,0xff};
static const unsigned char default_money_bag_enable[BLOCK_SIZE] = {0xFF,0xFF,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00};
static const unsigned char default_money_bag_disable[BLOCK_SIZE] = {0x00,0x00,0xFF,0xFF,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00};
static const unsigned char default_money_bag_value[BLOCK_SIZE] = {0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x21,0xDE,0x21,0xDE};
static const unsigned char default_key[KEY_LENGTH] = {0xff,0xff,0xff,0xff,0xff,0xff};
static const unsigned char card_inf_blocks[CARD_INF_BLOCK_COUNT] = {4,5,6,8,9,10,12,13,14,16,17,18,20,21,22,24,25,26};//,28,29,30,32,33,34,36,37,38,40,41,42,44,45,46,48,49,50,52,53,54,56,57,58,60,61,62

static rt_uint8_t find_tag;
    
//初始化电子钱包
static rt_bool_t rfic_money_init(rt_uint8_t card_id[4], rt_uint8_t in_key_b[KEY_LENGTH], rt_uint8_t *ctrl_buffer, rt_bool_t use_money_bag)
{
	rt_bool_t result = RT_FALSE;
	int i;
	rt_uint8_t temp_buf[BLOCK_SIZE];

	if(pcd_auth_state_ex(PICC_AUTHENT1B, MONEY_BAG_ENABLE, in_key_b, card_id) != MI_OK)
	{
		goto _EXIT;
	}
	if(use_money_bag)
	{
		rt_memcpy(temp_buf, default_money_bag_enable, BLOCK_SIZE);
		get_rnd_bytes(5, temp_buf + 6);
		for(i=0; i<5; i++)
		{
			temp_buf[11 + i] = ~temp_buf[6 + i];
		}
	}
	else
	{
		rt_memcpy(temp_buf, default_money_bag_disable, BLOCK_SIZE);
	}

	if(pcd_write_ex(MONEY_BAG_ENABLE, temp_buf) != MI_OK)
	{
		goto _EXIT;
	}
	if(pcd_write_ex(MONEY_BAG_VALUE, (unsigned char*)default_money_bag_value) != MI_OK)
	{
		goto _EXIT;
	}
	//备份钱包
	if(pcd_bak_value_ex(MONEY_BAG_VALUE, MONEY_BAG_BF) != MI_OK)
	{
		goto _EXIT;
	}
	if(pcd_write_ex(MONEY_BAG_CTRL, ctrl_buffer) != MI_OK)
	{
		goto _EXIT;
	}
	result = RT_TRUE;
_EXIT:
	return result;
}

void rfic_scan_reset(void)
{
    find_tag = 0;
}

/*
function: 扫描IC卡,并返回扫描结果
param:
in_key_a: in 应用卡A钥
in_key_b: in 应用卡B钥
out_result: out 输出卡信息，在外部释放
return: 卡基础类型 enum card_base_type
*/
enum card_base_type rfic_scan_handle(rt_uint8_t in_key_a[KEY_LENGTH], rt_uint8_t in_key_b[KEY_LENGTH], rfic_scan_info_t out_result)
{	
	rt_uint8_t auth_index = 0;
	enum card_base_type card_base_type = CARD_TYPE_NULL;
	rt_uint8_t status, block_count;
	rt_uint16_t i, inf_len;
	rt_uint8_t check_buf[BLOCK_SIZE*2];//用于存放CARD_CHECK_BLOCK和CARD_LEN_BLOCK或MD5结果
	
	RT_ASSERT(out_result != RT_NULL);
	RT_ASSERT(out_result->buffer != RT_NULL);
	
	out_result->buf_len = 0;
	
_ENTRY:
	if((status = pcd_request_ex(PICC_REQALL, out_result->card_id)) != MI_OK)
		status = pcd_request_ex(PICC_REQALL, out_result->card_id);
	if(status != MI_OK){//寻卡获得卡类型
		find_tag = 0;
	}
	if(status == MI_OK && find_tag == 0)
	{				
		if(pcd_anticoll_ex(out_result->card_id) == MI_OK)//读卡号
		{
			if(pcd_select_ex(out_result->card_id) == MI_OK)//选择卡
			{				
				if(auth_index==0)
				{
					if(pcd_auth_state_ex(PICC_AUTHENT1A, CARD_CHECK_BLOCK, (unsigned char*)default_key, out_result->card_id) == MI_OK)
					{//空白卡
						find_tag = 1;
						card_base_type = CARD_TYPE_BLANK;				
					}
					else
					{
						auth_index++;
						goto _ENTRY;
					}
				}
				else if(auth_index==1)
				{
					if(pcd_auth_state_ex(PICC_AUTHENT1A, CARD_CHECK_BLOCK, (unsigned char*)factory_key_a, out_result->card_id) == MI_OK)
					{//密钥卡
						find_tag = 1;
						card_base_type = CARD_TYPE_KEY;
						if(pcd_read_ex(CARD_CHECK_BLOCK, check_buf) != MI_OK)
						{
							goto _EXIT;
						}
						if(pcd_read_ex(CARD_LEN_BLOCK, check_buf + BLOCK_SIZE) != MI_OK)
						{
							goto _EXIT;
						}					
						inf_len = check_buf[BLOCK_SIZE]; inf_len <<= 8; inf_len |= check_buf[BLOCK_SIZE + 1];
						if(inf_len > CARD_INF_MAX_LEN)
						{//超出最大长度
							goto _EXIT;
						}
						block_count = inf_len / BLOCK_SIZE + (inf_len % BLOCK_SIZE > 0 ? 1:0);//实际在读取的数据块数量
					
						rt_uint8_t *tem_buffer = rt_calloc(1, inf_len + (KEY_LENGTH * 2) + SIGNATURE_LENGTH);
						RT_ASSERT(tem_buffer != RT_NULL);
						for(i=0; i<block_count; i++)
						{
							if(i % 3 == 0){
								if(pcd_auth_state_ex(PICC_AUTHENT1A, card_inf_blocks[i], (unsigned char*)factory_key_a, out_result->card_id) != MI_OK)
								{
									rt_free(tem_buffer);
									goto _EXIT;
								}
							}
							if(pcd_read_ex(card_inf_blocks[i], tem_buffer + (i * BLOCK_SIZE)) != MI_OK)
							{
								rt_free(tem_buffer);
								goto _EXIT;
							}
						}
						//校验卡
						rt_memcpy(tem_buffer + inf_len, factory_key_a, KEY_LENGTH);
						rt_memcpy(tem_buffer + inf_len + KEY_LENGTH, factory_key_b, KEY_LENGTH);
						rt_memcpy(tem_buffer + inf_len + KEY_LENGTH + KEY_LENGTH, factory_signature, SIGNATURE_LENGTH);
						tiny_md5(tem_buffer, inf_len + KEY_LENGTH + KEY_LENGTH + SIGNATURE_LENGTH, check_buf + BLOCK_SIZE);
						if(rt_memcmp(check_buf, check_buf + BLOCK_SIZE, BLOCK_SIZE) == 0)
						{
							out_result->buf_len = inf_len;
							*out_result->buffer = rt_calloc(1, inf_len + 1);
							rt_memcpy(*out_result->buffer, tem_buffer, inf_len);
							(*out_result->buffer)[inf_len] = '\0';
						}
						rt_free(tem_buffer);
					}
					else
					{
						auth_index++;
						goto _ENTRY;
					}
				}
				else if(auth_index==2)
				{
					if(in_key_a != RT_NULL && pcd_auth_state_ex(PICC_AUTHENT1A, CARD_CHECK_BLOCK, in_key_a, out_result->card_id) == MI_OK)
					{//应用卡
						find_tag = 1;
						card_base_type = CARD_TYPE_APP;
						if(pcd_read_ex(CARD_CHECK_BLOCK, check_buf) != MI_OK)
						{
							goto _EXIT;
						}
						if(pcd_read_ex(CARD_LEN_BLOCK, check_buf + BLOCK_SIZE) != MI_OK)
						{
							goto _EXIT;
						}					
						inf_len = check_buf[BLOCK_SIZE]; inf_len <<= 8; inf_len |= check_buf[BLOCK_SIZE + 1];
						if(inf_len > CARD_INF_MAX_LEN)
						{//超出最大长度
							goto _EXIT;
						}
						block_count = inf_len / BLOCK_SIZE + (inf_len % BLOCK_SIZE > 0 ? 1:0);//实际在读取的数据块数量

						rt_uint8_t *tem_buffer = rt_calloc(1, inf_len + (KEY_LENGTH * 2) + SIGNATURE_LENGTH);
						RT_ASSERT(tem_buffer != RT_NULL);
						for(i=0; i<block_count; i++)
						{
							if(i % 3 == 0){
								if(pcd_auth_state_ex(PICC_AUTHENT1A, card_inf_blocks[i], in_key_a, out_result->card_id) != MI_OK)
								{
									rt_free(tem_buffer);
									goto _EXIT;
								}
							}
							if(pcd_read_ex(card_inf_blocks[i], tem_buffer + (i * BLOCK_SIZE)) != MI_OK)
							{
								rt_free(tem_buffer);
								goto _EXIT;
							}
						}
						//校验卡
						rt_memcpy(tem_buffer + inf_len, in_key_a, KEY_LENGTH);
						rt_memcpy(tem_buffer + inf_len + KEY_LENGTH, in_key_b, KEY_LENGTH);
						rt_memcpy(tem_buffer + inf_len + KEY_LENGTH + KEY_LENGTH, factory_signature, SIGNATURE_LENGTH);
						tiny_md5(tem_buffer, inf_len + KEY_LENGTH + KEY_LENGTH + SIGNATURE_LENGTH, check_buf + BLOCK_SIZE);
						if(rt_memcmp(check_buf, check_buf + BLOCK_SIZE, BLOCK_SIZE)==0)
						{//校验通过
							out_result->buf_len = inf_len;
							*out_result->buffer = rt_calloc(1, inf_len + 1);
							rt_memcpy(*out_result->buffer, tem_buffer, inf_len);
							(*out_result->buffer)[inf_len] = '\0';
						}
						rt_free(tem_buffer);
					}			
					else
					{//未知卡
						find_tag = 1;
						card_base_type = CARD_TYPE_UNKNOW;
						rt_kprintf("PcdAuthState CARD_APP_CHECK_BLOCK error\r\n");
						goto _EXIT;
					}
				}
			}
		}
	}
_EXIT:		
	out_result->base_type = card_base_type;
	return card_base_type;
}

//初始化卡
rt_bool_t rfic_card_init(enum card_base_type type, rt_bool_t use_money_bag, rt_uint8_t in_key_a[KEY_LENGTH], rt_uint8_t in_key_b[KEY_LENGTH])
{
	rt_uint8_t status, card_id[4], result = RT_FALSE;
	rt_uint16_t i;
	rt_uint8_t temp_buf[BLOCK_SIZE];

	if((status = pcd_request_ex(PICC_REQALL, card_id)) != MI_OK)
		status = pcd_request_ex(PICC_REQALL, card_id);
	if(status == MI_OK)
	{
		if(pcd_anticoll_ex(card_id) == MI_OK)
		{
			if(pcd_select_ex(card_id) == MI_OK)
			{
				rt_memcpy(temp_buf, default_data_ctrl, BLOCK_SIZE);
				if(type == CARD_TYPE_KEY)
				{//密钥卡
					rt_memcpy(temp_buf, factory_key_a, KEY_LENGTH);
					rt_memcpy(temp_buf + KEY_LENGTH + CTRL_LENGTH, factory_key_b, KEY_LENGTH);
				}
				else if(type == CARD_TYPE_APP)
				{//应用卡
					RT_ASSERT(in_key_a != RT_NULL);
					RT_ASSERT(in_key_b != RT_NULL);
					rt_memcpy(temp_buf, in_key_a, KEY_LENGTH);
					rt_memcpy(temp_buf + KEY_LENGTH + CTRL_LENGTH, in_key_b, KEY_LENGTH);
				}
				else
				{
					goto _EXIT;
				}
				//处理第1个扇区
				if(pcd_auth_state_ex(PICC_AUTHENT1B, CARD_CHECK_BLOCK, (unsigned char*)default_key, card_id) != MI_OK)
				{
					goto _EXIT;
				}
				if(pcd_write_ex(CARD_CHECK_BLOCK, (unsigned char*)default_data_block) != MI_OK)
				{
					goto _EXIT;
				}
				if(pcd_write_ex(CARD_LEN_BLOCK, (unsigned char*)default_data_block) != MI_OK)
				{
					goto _EXIT;
				}
				if(pcd_write_ex(CARD_CTRL_BLOCK, temp_buf) != MI_OK)
				{
					goto _EXIT;
				}
				//处理其它扇区
				for(i=0; i<CARD_INF_BLOCK_COUNT; i+=3)
				{
					if(pcd_auth_state_ex(PICC_AUTHENT1B, card_inf_blocks[i], (unsigned char*)default_key, card_id) != MI_OK)
					{
						goto _EXIT;
					}
					if(pcd_write_ex(card_inf_blocks[i] + 0, (unsigned char*)default_data_block) != MI_OK)
					{
						goto _EXIT;
					}
					if(pcd_write_ex(card_inf_blocks[i] + 1, (unsigned char*)default_data_block) != MI_OK)
					{
						goto _EXIT;
					}
					if(pcd_write_ex(card_inf_blocks[i] + 2, (unsigned char*)default_data_block) != MI_OK)
					{
						goto _EXIT;
					}
					if(pcd_write_ex(card_inf_blocks[i] + 3, temp_buf) != MI_OK)
					{
						goto _EXIT;
					}
				}
				if(type == CARD_TYPE_APP)
				{//应用卡
					//处理电子钱包
					if(! rfic_money_init(card_id, (rt_uint8_t*)default_key, temp_buf, use_money_bag))
					{
						goto _EXIT;
					}
				}
				result = RT_TRUE;
			}
		}
	}
_EXIT:
	return result;
}

//重置卡
rt_bool_t rfic_card_reset(enum card_base_type type, rt_uint8_t in_key_b[KEY_LENGTH])
{
	rt_uint8_t status, card_id[4], result = RT_FALSE;
	rt_uint16_t i;
	rt_uint8_t auth_key[KEY_LENGTH];

	if((status = pcd_request_ex(PICC_REQALL, card_id)) != MI_OK)
		status = pcd_request_ex(PICC_REQALL, card_id);
	if(status == MI_OK)
	{
		if(pcd_anticoll_ex(card_id) == MI_OK)
		{
			if(pcd_select_ex(card_id)==MI_OK)
			{
				if(type == CARD_TYPE_KEY)
				{//密钥卡
					rt_memcpy(auth_key, factory_key_b, KEY_LENGTH);
				}
				else if(type == CARD_TYPE_APP)
				{//应用卡
					RT_ASSERT(in_key_b != RT_NULL);
					rt_memcpy(auth_key, in_key_b, KEY_LENGTH);
				}
				else
				{
					goto _EXIT;
				}
				//处理第1个扇区
				if(pcd_auth_state_ex(PICC_AUTHENT1B, CARD_CHECK_BLOCK, auth_key, card_id) != MI_OK)
				{
					goto _EXIT;
				}
				if(pcd_write_ex(CARD_CHECK_BLOCK, (unsigned char*)default_data_block) != MI_OK)
				{
					goto _EXIT;
				}
				if(pcd_write_ex(CARD_LEN_BLOCK, (unsigned char*)default_data_block) != MI_OK)
				{
					goto _EXIT;
				}
				if(pcd_write_ex(CARD_CTRL_BLOCK, (unsigned char*)default_data_ctrl) != MI_OK)
				{
					goto _EXIT;
				}
				//处理其它扇区
				for(i=0; i<CARD_INF_BLOCK_COUNT; i+=3)
				{
					if(pcd_auth_state_ex(PICC_AUTHENT1B, card_inf_blocks[i], auth_key, card_id) != MI_OK)
					{
						goto _EXIT;
					}
					if(pcd_write_ex(card_inf_blocks[i] + 0, (unsigned char*)default_data_block) != MI_OK)
					{
						goto _EXIT;
					}
					if(pcd_write_ex(card_inf_blocks[i] + 1, (unsigned char*)default_data_block) != MI_OK)
					{
						goto _EXIT;
					}
					if(pcd_write_ex(card_inf_blocks[i] + 2, (unsigned char*)default_data_block) != MI_OK)
					{
						goto _EXIT;
					}
					if(pcd_write_ex(card_inf_blocks[i] + 3, (unsigned char*)default_data_ctrl) != MI_OK)
					{
						goto _EXIT;
					}
				}
				//处理电子钱包扇区
				if(pcd_auth_state_ex(PICC_AUTHENT1B, MONEY_BAG_ENABLE, auth_key, card_id) != MI_OK)
				{//认证未通过，则忽略此扇区
					result = RT_TRUE;
					goto _EXIT;
				}
				if(pcd_write_ex(MONEY_BAG_ENABLE, (unsigned char*)default_data_block) != MI_OK)
				{
					goto _EXIT;
				}
				if(pcd_write_ex(MONEY_BAG_VALUE, (unsigned char*)default_data_block) != MI_OK)
				{
					goto _EXIT;
				}
				if(pcd_write_ex(MONEY_BAG_BF, (unsigned char*)default_data_block) != MI_OK)
				{
					goto _EXIT;
				}
				if(pcd_write_ex(MONEY_BAG_CTRL, (unsigned char*)default_data_ctrl) != MI_OK)
				{
					goto _EXIT;
				}
				result = RT_TRUE;
			}
		}
	}
_EXIT:
	return result;
}

//写卡
rt_bool_t rfic_card_write(enum card_base_type type, rt_uint8_t in_key_a[KEY_LENGTH], rt_uint8_t in_key_b[KEY_LENGTH], rt_uint8_t *buffer, rt_uint16_t buf_length)
{
	rt_uint8_t status, card_id[4], result = RT_FALSE;
	rt_uint16_t i, block_count;
	rt_uint8_t auth_akey[KEY_LENGTH], auth_bkey[KEY_LENGTH], check_md5[BLOCK_SIZE];
	rt_uint8_t *check_all = RT_NULL;

	if(buf_length>CARD_INF_MAX_LEN)
		goto _EXIT;

	if((status = pcd_request_ex(PICC_REQALL, card_id)) != MI_OK)
		status = pcd_request_ex(PICC_REQALL, card_id);
	if(status == MI_OK)
	{
		if(pcd_anticoll_ex(card_id) == MI_OK)
		{
			if(pcd_select_ex(card_id)==MI_OK)
			{				
				if(type == CARD_TYPE_KEY)
				{//密钥卡
					rt_memcpy(auth_akey, factory_key_a, KEY_LENGTH);
					rt_memcpy(auth_bkey, factory_key_b, KEY_LENGTH);
				}
				else if(type == CARD_TYPE_APP)
				{//应用卡
					RT_ASSERT(in_key_a != RT_NULL && in_key_b != RT_NULL);
					rt_memcpy(auth_akey, in_key_a, KEY_LENGTH);
					rt_memcpy(auth_bkey, in_key_b, KEY_LENGTH);
				}
				else
				{
					goto _EXIT;
				}			
				block_count = buf_length / BLOCK_SIZE + (buf_length % BLOCK_SIZE ? 1:0);
				//处理其它扇区
				for(i=0; i<block_count; i++)
				{
					if(i % 3 == 0)
					{
						if(pcd_auth_state_ex(PICC_AUTHENT1B, card_inf_blocks[i], auth_bkey, card_id) != MI_OK)
						{
							goto _EXIT;
						}
					}					
					if(pcd_write_ex(card_inf_blocks[i], buffer + (i * BLOCK_SIZE)) != MI_OK)
					{
						goto _EXIT;
					}
				}
				//处理第1个扇区
				if(pcd_auth_state_ex(PICC_AUTHENT1B, CARD_CHECK_BLOCK, auth_bkey, card_id) != MI_OK)
				{
					goto _EXIT;
				}
				//
				check_all = rt_malloc(buf_length + KEY_LENGTH + KEY_LENGTH + SIGNATURE_LENGTH);
				if(check_all == RT_NULL)
				{
					goto _EXIT;
				}

				rt_memcpy(check_all, buffer, buf_length);
				rt_memcpy(check_all + buf_length, auth_akey, KEY_LENGTH);
				rt_memcpy(check_all + buf_length + KEY_LENGTH, auth_bkey, KEY_LENGTH);
				rt_memcpy(check_all + buf_length + KEY_LENGTH + KEY_LENGTH, factory_signature, SIGNATURE_LENGTH);

				tiny_md5(check_all, buf_length + KEY_LENGTH + KEY_LENGTH + SIGNATURE_LENGTH, check_md5);
				if(pcd_write_ex(CARD_CHECK_BLOCK, check_md5) != MI_OK)
				{
					goto _EXIT;
				}
				check_md5[0] = (rt_uint8_t)(buf_length>>8);		//保存信息长度高位
				check_md5[1] = (rt_uint8_t)(buf_length&0xff);	//保存信息长度低位
				if(pcd_write_ex(CARD_LEN_BLOCK, check_md5) != MI_OK)
				{
					goto _EXIT;
				}			
				result = RT_TRUE;
			}
		}
	}
_EXIT:
	if(check_all != RT_NULL) rt_free(check_all);		
	return result;
}

rt_bool_t rfic_money_read(rt_uint8_t in_key_a[KEY_LENGTH], rt_bool_t *out_stat, rt_uint32_t *out_value)
{
	rt_uint8_t status, card_id[4], result = RT_FALSE;
	rt_uint16_t i;
	rt_uint8_t temp_buf[BLOCK_SIZE];

	if((status = pcd_request_ex(PICC_REQALL, card_id)) != MI_OK)
		status = pcd_request_ex(PICC_REQALL, card_id);
	if(status == MI_OK)
	{
		if(pcd_anticoll_ex(card_id) == MI_OK)
		{
			if(pcd_select_ex(card_id)==MI_OK)
			{				
				//处理电子钱包
				if(pcd_auth_state_ex(PICC_AUTHENT1A, MONEY_BAG_ENABLE, (unsigned char*)in_key_a, card_id) != MI_OK)
				{
					goto _EXIT;
				}	
				if(pcd_read_ex(MONEY_BAG_ENABLE, temp_buf) != MI_OK)
				{
					goto _EXIT;
				}
				//校验
				for(i=0; i<5; i++)
				{
					temp_buf[6 + i] = ~temp_buf[6 + i];
				}
				rt_uint8_t check_tag = ! rt_memcmp(temp_buf + 6, temp_buf + 11, 5);
				if(rt_memcmp(temp_buf, default_money_bag_disable, 6) == 0 && check_tag)
				{//电子钱包禁用
					*out_stat = RT_FALSE;
					*out_value = 0;
				}
				else
				{//电子钱包启用
					*out_stat = RT_TRUE;
					if(! check_tag)
					{
						*out_value = 0;
					}
					else
					{
						if(pcd_read_ex(MONEY_BAG_VALUE, temp_buf) != MI_OK)
						{
							goto _EXIT;
						}
						//校验
						for(i=0; i<4; i++)
						{
							temp_buf[4 + i] = ~temp_buf[4 + i];
						}
                        temp_buf[13] = ~temp_buf[13];
                        temp_buf[15] = ~temp_buf[15];
						if(rt_memcmp(temp_buf, temp_buf + 4, 4) == 0 && rt_memcmp(temp_buf, temp_buf + 8, 4) == 0 
                            && temp_buf[12] == temp_buf[13] && temp_buf[12] == temp_buf[14] && temp_buf[12] == temp_buf[15])
						{//通过校验
							*out_value = temp_buf[3]; (*out_value) <<= 8; (*out_value) |= temp_buf[2]; 
							(*out_value) <<= 8; (*out_value) |= temp_buf[1]; (*out_value) <<= 8; (*out_value) |= temp_buf[0];							
						}
						else
						{//未通过校验
							*out_value = 0;
						}
					}
				}
				result = RT_TRUE;	
			}
		}
	}
_EXIT:
	return result;	
}
//value>0:充值 value<0:扣款
rt_bool_t rfic_money_write(rt_uint8_t in_key_b[KEY_LENGTH], rt_int32_t value)
{
	rt_uint8_t status, card_id[4], result = RT_FALSE;
	rt_uint16_t i;
	rt_uint8_t temp_buf[BLOCK_SIZE];

	if((status = pcd_request_ex(PICC_REQALL, card_id)) != MI_OK)
		status = pcd_request_ex(PICC_REQALL, card_id);
	if(status == MI_OK)
	{
		if(pcd_anticoll_ex(card_id) == MI_OK)
		{
			if(pcd_select_ex(card_id)==MI_OK)
			{				
				//处理电子钱包
				if(pcd_auth_state_ex(PICC_AUTHENT1B, MONEY_BAG_ENABLE, (unsigned char*)in_key_b, card_id) != MI_OK)
				{
					goto _EXIT;
				}
				if(pcd_read_ex(MONEY_BAG_ENABLE, temp_buf) != MI_OK)
				{
					goto _EXIT;
				}
				//校验
				for(i=0; i<5; i++)
				{
					temp_buf[6 + i] = ~temp_buf[6 + i];
				}
				rt_uint8_t check_tag = ! rt_memcmp(temp_buf + 6, temp_buf + 11, 5);
				if(rt_memcmp(temp_buf, default_money_bag_disable, 6) == 0 && check_tag)
				{//电子钱包禁用
					goto _EXIT;
				}
				else
				{//电子钱包启用
					rt_int32_t v;
					if(value>0)
					{//充值
						v = value;
						if(pcd_value_ex(PICC_INCREMENT, MONEY_BAG_VALUE, (rt_uint8_t*)&v) != MI_OK)
						{//充值失败
							goto _EXIT;
						}
						result = RT_TRUE;
					}
					else if(value<0)
					{//扣款
						v = -value;
						if(pcd_value_ex(PICC_DECREMENT, MONEY_BAG_VALUE, (rt_uint8_t*)&v) != MI_OK)
						{//充值失败
							goto _EXIT;
						}
						result = RT_TRUE;
					}
				}
			}
		}
	}
_EXIT:
	return result;	
}
