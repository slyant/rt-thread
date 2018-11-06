#include <rtthread.h>
#include <webclient.h>
#include <qr_client.h>
#include <cloud_pay_intf.h>
#include <drv_rng.h>
#include <tinycrypt.h>

#define DBG_ENABLE
#define DBG_COLOR
#define DBG_SECTION_NAME	"cloud_pay"
#define DBG_LEVEL           DBG_INFO
#include <rtdbg.h>

#define QR_DEVICE_NAME	"uart1"
#define QR_BUFFER_SIZE	30
#define QR_MQ_MAX_MSGS	5

#define PRIORITY	7
#define STACK_SIZE	20480
#define TIMESLICE	10

#define SHA256_DIGEST_LENGTH	32
const char* TRADE_NO_HEAD = "sz0100mpp3";
const char* MCH_ID = "sz01Kb5mGP6pdxtj7C53";
const char* SUB_MCH_ID = "sz01mYMssPJx5r5U4K8K";
const char* SHOP_ID = "sz011MYZ6vv7mZGfyEFP";
const char* DEVICE_ID = "1001";
const char* STAFF_ID = "1000";
const char* AUTHEN_KEY = "vGs4apQ3Iw4h2JwnuH2jaxp57rfPjsdD";
const char* PRIVATE_KEY = 
"-----BEGIN PRIVATE KEY-----\n"
"MIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSjAgEAAoIBAQDKv/ktN9dsscql\n"
"30XBnAipTVw3eSK8eXLSyU6HvKBe7Yioto/dOCf2Cj8/gMKx0YIkndcbfLnP5rMM\n"
"12T165z/docJO2wpvytqHO304Ro1e8IBMBShpMEdeK7wXnhaYwnVI28Hvhi2Y7r9\n"
"/MjqlAK0u9255Sw1MuF+1mUdcJ/HiF+QwNXLUDTNap+0m96U8UOpvtgwG1CsSk5l\n"
"BH0TGCCQU3ZOqt+a2WJbPVDWMqz1kqzM8H0txPkHSgH+h2fjwAd/pVc3AbGcqHOo\n"
"RoVP3LSqzXO9Uz/rx/XlrDYs1IftjqNiVBlZveBsWhs7g3C8tPv5LvHKl20Op6dy\n"
"eyjc4T87AgMBAAECggEBAJPAKIbiTdQBEIumVMBNxCYUoSIv3bd5zcp8NwVF2Zyz\n"
"/0XrnJV6yexL9WGCpN9OrUef2tVGJGa0WUD/aqyn4gZsyuVYb29QyAORR3uP2zik\n"
"knGpJy/Qz4Z5nwq93XkI+W5gXxZGWt33eZqD76q65+nrih5hOsEd7iQvewpkO5v8\n"
"oNMYrsPIEO26AM6fuNHIyHpeFiUXqsZPrCfIsNyo73iXjSLfjKSTmLo3KsoGT4iN\n"
"0Bucv49HQaSlglqBcxY0Jdg5iHDbHBW2xHoWUW7U115ZTkNGuzAknWjOvOvDN7A9\n"
"EwTv9t5uo5t2SqLpqZysm5Nh1mcyI89x3CSPh2D4DzkCgYEA/G9YYdssAGCog69J\n"
"1KTE3mt48wpYrGIbFYKu1JXmPxJ1gpYJH236K+FpE838AW4e2KtUa0TLbm0xgJtB\n"
"3zg2KQAu9mH0++Vd1pqaaZvCI0ePjHBiauKC9wxXUocSe0TC0kCFxxnqAjh4G96J\n"
"BK1geCdNnh7cYWpxdE4qblWLvk0CgYEAzZz/GnT8PYcT0XTWab/3KrVkCHRZD8uE\n"
"+nCun8K8AiCb/CUvEiCd+3oHJ8W2u+qGJtqLVZf/ns+Cf7ncsetFR3cx5GW6/6Ed\n"
"7fC3pAOHyubvIEWAMeBYV9Do7LQgyhUpAOAuN5XYmzGVc+IVDeqDyRyfCa7j6XYd\n"
"iLzEkxiUB6cCgYBEoZkr8cTCzYTDWLMAB1QnDJ0zpsp5A6tNa9xQm/ifiUs9l8Ef\n"
"NG1h5MUGx2gOk3v7eIlZKLkOas2co5mDMOs4tn4OKo/KtI+QuzPkJp7frKMQkbWQ\n"
"TclBla9gFq0lqoo1U6mLejYvKF8vZI9oICMsR7NInCdarO/vqRjMKqH1uQKBgG1a\n"
"TRaeFzUF7RVd5VbywJqXRC926ZML85paaZUQNXuQKkWM5R7n9/2yJbISZiBFF7n8\n"
"S1RndYzZ4hbaG7UIYp0imIdXT3/7CXbtqk1xjzvrXEw2inp8TZ/OvMJ+/l1bqaYM\n"
"lKf9aukHEuxB1l7DVfmU3kSBHjehGQ4LDyUIUlpLAoGAJ0f6MbP9TxxJ5rNyGxwZ\n"
"S4Dz2IduaUFzaqmpXSNuavY9RS8YKiojb8MGJdKbh5rOTYLbnod2x7g0vDkjXT18\n"
"eiSYKVZXDEx4nN9W6/VLwzu4Da4eyfQiy1WljQPXeomXZPK19/0F5hxz1u3w4yRR\n"
"5T5LYsRmlsgVUgQlt17qBa4=\n"
"-----END PRIVATE KEY-----\n";

static char* errmsg_utf8_to_ansi(const char* in)
{
	return (char*)in;
//	int outputSize = 0; //记录转换后的Unicode字符串的字节数
//	char* pInput = (char*)in;
//	char* pOutput;
//	while (*pInput)
//	{
//		if (*pInput > 0x00 && *pInput <= 0x7F) //处理单字节UTF8字符（英文字母、数字）
//		{
//			*pOutput = *pInput;
//			 pOutput++;
//			*pOutput = 0; //小端法表示，在高地址填补0
//		}
//		else if (((*pInput) & 0xE0) == 0xC0) //处理双字节UTF8字符
//		{
//			char high = *pInput;
//			pInput++;
//			char low = *pInput;
//			if ((low & 0xC0) != 0x80)  //检查是否为合法的UTF8字符表示
//			{
//				return pOutput; //如果不是则报错
//			}
// 
//			*pOutput = (high << 6) + (low & 0x3F);
//			pOutput++;
//			*pOutput = (high >> 2) & 0x07;
//		}
//		else if (((*pInput) & 0xF0) == 0xE0) //处理三字节UTF8字符
//		{
//			char high = *pInput;
//			pInput++;
//			char middle = *pInput;
//			pInput++;
//			char low = *pInput;
//			if (((middle & 0xC0) != 0x80) || ((low & 0xC0) != 0x80))
//			{
//				return pOutput;
//			}
//			*pOutput = (middle << 6) + (low & 0x3F);//取出middle的低两位与low的低6位，组合成unicode字符的低8位
//			pOutput++;
//			*pOutput = (high << 4) + ((middle >> 2) & 0x0F); //取出high的低四位与middle的中间四位，组合成unicode字符的高8位
//		}
//		else //对于其他字节数的UTF8字符不进行处理
//		{
//			return pOutput;
//		}
//		pInput ++;//处理下一个utf8字符
//		pOutput ++;
//		outputSize += 2;
//	}
//	//unicode字符串后面，有两个\0
//	*pOutput = 0;
//	 pOutput++;
//	*pOutput = 0;
//	return pOutput;	
}
static int self_http_post(const char *url, const char *request, char *response, size_t *length)
{
#define POST_HEADER_BUFSZ	1024
#define POST_RESP_BUFSZ		1024
	
    struct webclient_session* session = RT_NULL;
    unsigned char *buffer = (unsigned char*)response;
    char *URI = (char*)url;
    int response_pos = 0, ret = 0;
    int bytes_read, resp_status;

    /* create webclient session and set header response size */
    session = webclient_session_create(POST_HEADER_BUFSZ);
    if (session == RT_NULL)
    {
        ret = -RT_ENOMEM;
        goto __exit;
    }
	
    /* build header for upload */
    webclient_header_fields_add(session, "Content-Length: %d\r\n", rt_strlen(request));
    webclient_header_fields_add(session, "Content-Type: application/json\r\n");
    /* send GET request by default header */
	rt_kprintf("send POST request to %s\n", URI);
    if ((resp_status = webclient_post(session, URI, request)) != 200)
    {
        rt_kprintf("webclient POST request failed, response(%d) error.\n", resp_status);
        ret = -RT_ERROR;
        goto __exit;
    }
	
    do
    {
        bytes_read = webclient_read(session, buffer + response_pos, POST_RESP_BUFSZ);
        if (bytes_read <= 0)
        {
            break;
        }
		response_pos += bytes_read;
    } while (1);

__exit:
    if (session)
    {
        webclient_close(session);
    }
    return ret;	
}
static int self_compute_sign_2_base64(const char *in, const char* private_key, char *out, size_t *out_size)
{
	return  0;
}
static int self_compute_authen_code(const char *in, const char* key, char *out, size_t *out_size)
{
	unsigned char md[SHA256_DIGEST_LENGTH]; //32 bytes
	unsigned int md_len = sizeof(md);
	
	tiny_sha2_hmac((unsigned char*)key, rt_strlen(key), (unsigned char*)in, rt_strlen(in), md, 0);
	
	if (*out_size < SHA256_DIGEST_LENGTH * 2 + 1) {
		return -1;
	}

	for (unsigned int i = 0; i < md_len; i++)
	{
		rt_snprintf(out + i * 2, *out_size - i*2, "%02X", md[i]);
	}
	*out_size = SHA256_DIGEST_LENGTH * 2;
	
	return 0;
}
static int self_ansi_to_utf8(const char *ansi, char *utf8)
{
	rt_strncpy((char*)utf8, ansi, rt_strlen(ansi));
	return 0;
//	int len = 0; //记录转换后的Utf8字符串的字节数
//	char* pInput = (char*)ansi;
//	char* pOutput = utf8;
//	while (*pInput)
//	{
//		//处理一个unicode字符
//		char low = *pInput;//取出unicode字符的低8位
//		pInput++;
//		char high = *pInput;//取出unicode字符的高8位
//		int w=high<<8;
//		unsigned  wchar = (high<<8)+low;//高8位和低8位组成一个unicode字符,加法运算级别高
// 
//		if (wchar <= 0x7F ) //英文字符
//		{   
//			pOutput[len] = (char)wchar;  //取wchar的低8位
//			len++;
//		}  
//		else if (wchar >=0x80 && wchar <= 0x7FF)  //可以转换成双字节pOutput字符
//		{  
//			pOutput[len] = 0xc0 |((wchar >> 6)&0x1f);  //取出unicode编码低6位后的5位，填充到110yyyyy 10zzzzzz 的yyyyy中
//			len++;
//			pOutput[len] = 0x80 | (wchar & 0x3f);  //取出unicode编码的低6位，填充到110yyyyy 10zzzzzz 的zzzzzz中
//			len++;
//		}  
//		else if (wchar >=0x800 && wchar < 0xFFFF)  //可以转换成3个字节的pOutput字符
//		{  
//			pOutput[len] = 0xe0 | ((wchar >> 12)&0x0f);  //高四位填入1110xxxx 10yyyyyy 10zzzzzz中的xxxx
//			len++;
//			pOutput[len] = 0x80 | ((wchar >> 6) & 0x3f);  //中间6位填入1110xxxx 10yyyyyy 10zzzzzz中的yyyyyy
//			len++;
//			pOutput[len] = 0x80 | (wchar & 0x3f);  //低6位填入1110xxxx 10yyyyyy 10zzzzzz中的zzzzzz
//			len++;
//		}
//		else //对于其他字节数的unicode字符不进行处理
//		{
//			return -1;
//		}
//		pInput ++;//处理下一个unicode字符
//	}
//	//utf8字符串后面，有个\0
//	pOutput [len]= 0;
//	return len;
}

static int self_pf_fini()
{
//	EVP_cleanup();
//	CRYPTO_cleanup_all_ex_data();
//	ERR_remove_thread_state(NULL);
//	ERR_free_strings();
	return 0;
}
static int self_pf_init()
{
	//openssl初始化
	//OpenSSL_add_all_algorithms();
	return 0;
}
static int cloud_pay_init(void)
{
	Account account;
	Key key;
	Terminal terminal;
	CloudPayApiOps ops;

	rt_memset(&account, 0, sizeof(Account));
	rt_memset(&key, 0, sizeof(Key));
	rt_memset(&terminal, 0, sizeof(Terminal));
	rt_memset(&ops, 0, sizeof(CloudPayApiOps));

	ops.pf_init = self_pf_init;
	ops.pf_fini = self_pf_fini;

	ops.pf_mem_cpy = rt_memcpy;
	ops.pf_str_cmp = rt_strcmp;
	ops.pf_str_len = rt_strlen;
	ops.pf_malloc  = rt_malloc;
	ops.pf_free    = rt_free;
	ops.pf_snprintf = rt_snprintf;
	ops.pf_ansi_2_utf8 = self_ansi_to_utf8;
	ops.pf_hmac_sha256 = self_compute_authen_code;
	ops.pf_sign_2_base64 = self_compute_sign_2_base64;
	ops.pf_http_post = self_http_post;
	
	rt_memcpy(account.out_mch_id,		MCH_ID, rt_strlen(MCH_ID) + 1);
	rt_memcpy(account.out_sub_mch_id,	SUB_MCH_ID, rt_strlen(SUB_MCH_ID) + 1);
	rt_memcpy(account.out_shop_id,		SHOP_ID, rt_strlen(SHOP_ID) + 1);
	rt_memcpy(account.device_id,		DEVICE_ID, rt_strlen(DEVICE_ID) + 1);
	rt_memcpy(account.staff_id,			STAFF_ID, rt_strlen(STAFF_ID) + 1);

	rt_memcpy(key.authen_key, AUTHEN_KEY, rt_strlen(AUTHEN_KEY) + 1);
	rt_memcpy(key.private_key, PRIVATE_KEY, rt_strlen(PRIVATE_KEY ) + 1);

	rt_memcpy(terminal.sdk_version,      "1.1", rt_strlen("1.1")+1);
	rt_memcpy(terminal.machine_no,       "01-01-01-01-01-01", rt_strlen("01-01-01-01-01-01") + 1);
	rt_memcpy(terminal.spbill_create_ip, "192.168.1.1", rt_strlen("192.168.1.1") + 1);
	terminal.terminal_type=2;		// 1 windows 2 linux 3 android
	terminal.sub_terminal_type=900;	// 机具的类型. 找云支付分配， 可以统计某个机具的交易量

	if(cloud_pay_api_init(&account, &key, &terminal, &ops)==RT_EOK)
	{
		LOG_I("cloud pay init success!");
		return RT_EOK;
	}
	else
	{
		LOG_E("cloud pay init failed!");
		return -RT_ERROR;
	}
}
static int cloud_pay_request(char* pay_code, long long fee)
{
	rt_err_t ret = RT_EOK;
	char nonce_str[17];
	rt_sprintf(nonce_str, "%04d%04d%04d%04d", RNG_Get_RandomRange(0,9999), RNG_Get_RandomRange(0,9999), RNG_Get_RandomRange(0,9999), RNG_Get_RandomRange(0,9999)); 
	struct tm datetime;
	time_t now = time(RT_NULL);
	struct tm *tmnow = localtime(&now);
	rt_memcpy(&datetime, tmnow, sizeof(struct tm));
	datetime.tm_year += (1900-2);
	datetime.tm_mon += 1;
	char* author_code = pay_code;
	char out_trade_no[64]; //云支付订单前缀
	rt_sprintf(out_trade_no,"%s04d%02d%02d%02d%02d%02d", TRADE_NO_HEAD, datetime.tm_year, datetime.tm_mon, datetime.tm_mday,
	datetime.tm_hour, datetime.tm_min, datetime.tm_sec);

	char out_refund_no_1[64];
	rt_snprintf(out_refund_no_1, sizeof(out_refund_no_1) - 1, "%s%s", out_trade_no, "01");

	char out_refund_no_2[64];
	rt_snprintf(out_refund_no_2, sizeof(out_refund_no_2) - 1, "%s%s", out_trade_no, "02");

	int state = 0;
	
	if(ret == 0)
	{
		MicroPayRequest request;
		MicroPayResponse response;
		
		rt_memset(&request, 0, sizeof(request));
		rt_memcpy(request.out_trade_no, out_trade_no, rt_strlen(out_trade_no) + 1);
		rt_memcpy(request.author_code, author_code, rt_strlen(author_code) +1);
		rt_memcpy(request.nonce_str, nonce_str, rt_strlen(nonce_str) + 1);
		rt_memcpy(request.body, "Bus fare", rt_strlen("Bus fare") + 1);
		request.total_fee=fee;
		request.pay_platform=1;
		
		ret = micro_pay(&request, &response);

		rt_kprintf("ret = %d, %s\n", ret, errmsg_utf8_to_ansi(error_utf8()));
		
		if (ret == 0) 
		{
			state = response.order.state;
			RT_ASSERT(state == KCloudPaySdkLocalStateUserPaying || 
				   state == KCloudPaySdkLocalStateSuccess    || 
				   state == KCloudPaySdkLocalStateRefund); //已经退过款
			if (ret == 0) 
			{
				RT_ASSERT(state == KCloudPaySdkLocalStateSuccess || state == KCloudPaySdkLocalStateUserPaying || state == KCloudPaySdkLocalStateRefund);
				RT_ASSERT(response.order.pay_platform == 1);
				RT_ASSERT(response.order.total_fee == fee);
				RT_ASSERT(0 == rt_strcmp(response.order.out_trade_no,out_trade_no));
				RT_ASSERT(rt_strlen(response.order.transaction_id) > 0);
			}
		}
		else if (ret == CLOUD_PAY_API_ERROR_SYSTEM_ERROR || ret == CLOUD_PAY_API_ERROR_NETWORK_TIMEOUT)
		{
			//结果未知 需要重试
		}

		else
		{
			//支付失败
		}		
	}
	return ret;
/*
	while (ret == 0 && state == KCloudPaySdkLocalStateUserPaying) //用户支付中状态 需要继续查单 可以查询1分钟没有结果就提示去手机端管理系统查询订单支付结果
	{
		QueryOrderRequest request;
		QueryOrderResponse response;
		
		rt_memset(&request, 0, sizeof(request));
		rt_memcpy(request.out_trade_no, out_trade_no, rt_strlen(out_trade_no) + 1);
		rt_memcpy(request.nonce_str, "nonce_str", rt_strlen("nonce_str") + 1);
		ret = query_order(&request, &response);
		rt_kprintf("query order ret = %d\n", ret);

		RT_ASSERT(ret == 0);
		
		if (ret == 0) 
		{
			state = response.order.state;
			RT_ASSERT(state == KCloudPaySdkLocalStateUserPaying || state == KCloudPaySdkLocalStateSuccess);

			if (state == KCloudPaySdkLocalStateSuccess)
			{
				RT_ASSERT(state == KCloudPaySdkLocalStateSuccess);
				RT_ASSERT(response.order.pay_platform == 1);
				RT_ASSERT(response.order.total_fee == 2);
				RT_ASSERT(0 == rt_strcmp(response.order.out_trade_no, out_trade_no));
				RT_ASSERT(rt_strlen(response.order.transaction_id) > 0);
			}
		}
		else if (ret == CLOUD_PAY_API_ERROR_SYSTEM_ERROR || ret == CLOUD_PAY_API_ERROR_NETWORK_TIMEOUT)
		{			
			//结果未知 需要重试
		}
		else if (ret == CLOUD_PAY_API_ERROR_ORDER_DONTEXIST)
		{
			//订单不存在
		}
		else
		{
			//查询失败
		}
		rt_thread_mdelay(2000);
	}

	if (ret == 0)
	{
		RefundRequest request;
		rt_memset(&request, 0, sizeof(request));
		rt_memcpy(request.out_trade_no, out_trade_no, rt_strlen(out_trade_no) + 1);
		rt_memcpy(request.out_refund_no, out_refund_no_1, rt_strlen(out_refund_no_1) + 1);
		rt_memcpy(request.nonce_str, "nonce_str", rt_strlen("nonce_str") + 1);
		request.total_fee = 2;
		request.refund_fee = 1;
		request.pay_platform = 1;
	
		ret = refund(&request);
		rt_kprintf("refund ret1 = %d, %s\n", ret, errmsg_utf8_to_ansi(error_utf8()));
		RT_ASSERT(ret == 0);

		if (ret == 0) 
		{
		
		}
		else if (ret == CLOUD_PAY_API_ERROR_SYSTEM_ERROR || ret == CLOUD_PAY_API_ERROR_NETWORK_TIMEOUT)
		{
			//结果未知 需要重试
		}
		else
		{
			//申请退款失败
		}

		rt_memcpy(request.out_refund_no, out_refund_no_2, rt_strlen(out_refund_no_2) + 1);
	
		ret = refund(&request);
		rt_kprintf("refund ret2 = %d, %s\n", ret, errmsg_utf8_to_ansi(error_utf8()));
		RT_ASSERT(ret == 0);
	}

	if (ret == 0)
	{
		QueryRefundRequest request;
		QueryRefundResponse response;
		rt_memset(&request, 0, sizeof(request));
		rt_memcpy(request.out_trade_no, out_trade_no, rt_strlen(out_trade_no) + 1);
		//memcpy(request.out_refund_no, out_refund_no_2, strlen(out_refund_no_2) + 1);
		rt_memcpy(request.nonce_str, "nonce_str", rt_strlen("nonce_str") + 1);
		request.pay_platform = 1;
	
		ret = query_refund(&request, &response);
		rt_kprintf("query refund ret = %d, %s\n", ret, errmsg_utf8_to_ansi(error_utf8()));
		RT_ASSERT(ret == 0);
		RT_ASSERT(response.count == 2);

		{
			RT_ASSERT(response.refund_order[0].state == kCloudPaySdkRefundLocalStateSuccess || response.refund_order[0].state == kCloudPaySdkRefundLocalStateProcessing);
			RT_ASSERT(response.refund_order[0].pay_platform == 1);
			RT_ASSERT(response.refund_order[0].total_fee == 2);
			RT_ASSERT(response.refund_order[0].refund_fee == 1);
			RT_ASSERT(0 == rt_strcmp(response.refund_order[0].out_trade_no, out_trade_no));
			RT_ASSERT(0 == rt_strcmp(response.refund_order[0].out_refund_no, out_refund_no_1));
			if (response.refund_order[0].state == kCloudPaySdkRefundLocalStateSuccess)
			{
				RT_ASSERT(rt_strlen(response.refund_order[0].refund_id) > 0);
			}
		}

		{
			RT_ASSERT(response.refund_order[1].state == kCloudPaySdkRefundLocalStateSuccess || response.refund_order[0].state == kCloudPaySdkRefundLocalStateProcessing);
			RT_ASSERT(response.refund_order[1].pay_platform == 1);
			RT_ASSERT(response.refund_order[1].total_fee == 2);
			RT_ASSERT(response.refund_order[1].refund_fee == 1);
			RT_ASSERT(0 == rt_strcmp(response.refund_order[1].out_trade_no, out_trade_no));
			RT_ASSERT(0 == rt_strcmp(response.refund_order[1].out_refund_no, out_refund_no_2));
			if (response.refund_order[1].state == kCloudPaySdkRefundLocalStateSuccess)
			{
				RT_ASSERT(rt_strlen(response.refund_order[1].refund_id) > 0);
			}
		}
	}	
	return ret;	
*/
}
static void thread_entry(void *parameter)
{
	char qr_code[QR_BUFFER_SIZE];
	char last_code[18] = "\0";
	while(1)
	{
		if(qr_mq_recv(qr_code, QR_BUFFER_SIZE)==RT_EOK)
		{
			char *head, *head0, *head1, *end;
			rt_uint8_t len;
			head0 = rt_strstr(qr_code, "j]C0");
			head1 = rt_strstr(qr_code, "Q]Q1");
			head = head0?head0:head1;
			if(head)
			{	//Q]Q1135124350877540729
				//j]C0135098347648826005
				end = rt_strstr(qr_code, "\r");
				if(end)
				{		
					len = (rt_uint8_t)(end - head - 4);
					if(len==18)
					{
						rt_memcpy(qr_code, head + 4, 18);
						qr_code[18] = '\0';
						if(rt_memcmp(qr_code, last_code, 18)==0)
						{
							rt_kprintf("The QR Code repetition!mq:%s", qr_code);
						}
						else
						{
							rt_kprintf("OK_QR:%s\n", qr_code);
							if(cloud_pay_request(qr_code, 1)==RT_EOK)
							{
								rt_kprintf("cloud pay ok!");
							}
							else
							{
								rt_kprintf("cloud pay failed!");
							}							
							rt_memcpy(last_code, qr_code, 18);
						}
						continue;
					}
				}				
			}
			rt_kprintf("ERR_QR:%s\n", qr_code);
		}
	}
}

static int cloud_pay_thread_startup(void)
{
	rt_err_t result0, result1, result2;	
	result0 = qr_device_init(QR_DEVICE_NAME, QR_BUFFER_SIZE, QR_MQ_MAX_MSGS);
	result1 = cloud_pay_init();
	if(result0==RT_EOK && result1==RT_EOK)
	{
		rt_thread_t rtt = RT_NULL;
		rtt = rt_thread_create("cl_pay",      //线程名称。
								thread_entry,    //线程入口函数。
								RT_NULL,         //线程入口参数。
								STACK_SIZE,      //线程栈大小。
								PRIORITY,        //线程优先级。
								TIMESLICE);      //时间片Tick。
		if(rtt != RT_NULL)                       //判断线程是否创建成功。
		{
			rt_thread_startup(rtt);             //线程创建成功，启动线程。
			result2 = RT_EOK;
		}
		else
		{
			result2 = -RT_ERROR;
		}
	}
	return (result0 | result1 | result2);
}
INIT_APP_EXPORT(cloud_pay_thread_startup);
