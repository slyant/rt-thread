#include <rtthread.h>
#include <qr_client.h>
#include <cloud_pay_intf.h>

#define QR_DEVICE_NAME	"uart1"
#define QR_BUFFER_SIZE	30
#define QR_MQ_MAX_MSGS	2

#define PRIORITY 7        
#define STACK_SIZE 20480
#define TIMESLICE 10

const char *g_private_key =
"-----BEGIN PRIVATE KEY-----\n"
"MIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSjAgEAAoIBAQDksI6notjNH6zk\n"
"awRweJSk71hSlut4gh1DKYrEUy5ncAK7Xl3dDIJYrtJTQyRZPq6ZdmPWFaChjjBJ\n"
"+3TGjL6gStzPoowFQenlmhxTI3LUIWLKnPPZVxb47NHtuMUDkwzgzVi8OiJ2D9LU\n"
"E8p7qSKFnKOGzL4GDrMTpxc3pFQGWppDBoSrK6t2ELdU4pWAgpTh/yB+0+2JiI76\n"
"OoSjthrhQxrhNiaoccjpw/fbtWvD8O6KsEN7Xh4uRugnE0C3PjdqcfiYZ8LtgE9d\n"
"EJc/703bm+yeppPLFoImBssZXkXqwbE5sNa/GxZWgp9SHrCrPVJ4plH1pz74uj2I\n"
"zvqurStzAgMBAAECggEAZnaS3BozICjOiqNGYGwuUX3pqUt0YlCN8t8kjRUl5/OA\n"
"TdoYnSdzKW/eza56FUBn81tfnPagu3HdQfCX4DQEXwZby+4s5Vgkh5qWkUuDMfdp\n"
"b5WGDj2ejdZxspzELPdQYFKxLNixQwuQ7zOU1/YugQXvIMEQbN9eLfKCDQykFxnd\n"
"R8HDQpinMkcF1y6UpyYxHYYlEQq9qYMufqKxepqYDsw5Etx/ZFKS1hvOJjIe16jf\n"
"SijPijg6p+zZAnh+W9saKcOBZcFSL2jCt3MspiGQZ4+m7jTPHMAmRo47VLv1wNaP\n"
"TTaCjzV7l7TwZ6jgBh9hdKz/yvUiBD5UwwcoJoszrQKBgQD1Tp8/LemyiRh+I1C6\n"
"s50TsfH60UBF7Qc5gy+gGnxiUOSf42JVfZ5cowDy+f08crPn8DSV/AhjjGmGuhdX\n"
"5Tv+OQosgajwDQ2JCfDSQrhD+8KsNdXhTMAFydFPMC1XhybCAVz6DhUGaE1SbZWN\n"
"VhB0ek8U1ZcPy2fEr3x32YCwjQKBgQDuqIBfDXavX9BgF8pXj8usoi0TwqIXKO2Q\n"
"FqfaDLf8nZnbiqqrNrWPtduiCbQhZ6NhEHeS8yw0bzT+6jAGTXgONoETFYgakvge\n"
"v5A3pBKjqynVwVj4ZpY+FoBrOo1c4MD3g1vl4g82vQcoYkpcOVshUxMLYQPnn2Oz\n"
"C3q8W9RL/wKBgAfotgeyckN4WhrPBq5OVjUpWpTspDbc8ouvKBzwXaxJ0WU6TpRV\n"
"DYMNGl27I6gMRrKmsvtW+epDv69BVrlVwb5rlmFi+NBsi1Asyt/smMZQWrF0tuWh\n"
"tNs6642zDvMwf1Zn9th1FcZ0HHIY6GZ7LOKkguI4FLvEoh7cTZYLpDONAoGBAOpf\n" 
"P/IY3r1Lcu9rbaKf7H0UpwAI2/Wyk9o05ZIVe3nxq+8WNpT9nUAMcCypKuTDSodA\n"
"DK2bcYXs0Dam9ZK68XPDwu5i7s5qFpDbv11lG4jvxp3MyvrmhBVlM6gaUEktz1ND\n"
"BTS01AwFhsn5/Nyk1wHa+5DpLVSCCv8B/v6Lj+kxAoGAKLrI9x30by2YJ0FHSbMR\n"
"befGgKLIpPTm7sku3Zeu4ocfdi072/hsk10wyZjzyUqXnsOQhnO+y89xSE5tDCDm\n"
"A1ftqnGX/VQSD55YhWEEZL26B3GxSlh899YSj2cejzzlHK0NTGSs0PIXZSwwugcY\n"
"RqJsXJrTJL98uAwI0CKbmlI=\n"
"-----END PRIVATE KEY-----";

static int self_http_post(const char *url, const char *request, char *response, size_t *length)
{
	return 0;
}
static int self_compute_sign_2_base64(const char *in, const char* private_key, char *out, size_t *out_size)
{
	return  0;
}
static int self_compute_authen_code(const char *in, const char* key, char *out, size_t *out_size)
{
	return 0;
}
static int self_ansi_to_utf8(const char *ansi, char *utf8)
{
	return 0;
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
	
	rt_memcpy(account.out_mch_id,		 "sz013NzuonO6CMJd0rCB", rt_strlen("sz013NzuonO6CMJd0rCB") + 1);
	rt_memcpy(account.out_sub_mch_id,  "sz01ELTR281OFpmdAp6J", rt_strlen("sz01ELTR281OFpmdAp6J") + 1);
	rt_memcpy(account.out_shop_id,     "sz019xAikh2E0VfLwLfi", rt_strlen("sz019xAikh2E0VfLwLfi") + 1);
	rt_memcpy(account.device_id,       "5973", rt_strlen("5973") + 1);
	rt_memcpy(account.staff_id,        "17352", rt_strlen("17352") + 1);

	rt_memcpy(key.authen_key, "lSCp1M5grGWFD7rJzaZaqixsvOhORp2P", rt_strlen("lSCp1M5grGWFD7rJzaZaqixsvOhORp2P") + 1);
	rt_memcpy(key.private_key, g_private_key, rt_strlen(g_private_key ) + 1);

	rt_memcpy(terminal.sdk_version,      "1.1", rt_strlen("1.1")+1);
	rt_memcpy(terminal.machine_no,       "01-01-01-01-01-01", rt_strlen("01-01-01-01-01-01") + 1);
	rt_memcpy(terminal.spbill_create_ip, "192.168.1.1", rt_strlen("192.168.1.1") + 1);
	terminal.terminal_type=     2;    // 1 windows 2 linux 3 android
	terminal.sub_terminal_type= 900;  // 机具的类型. 找云支付分配， 可以统计某个机具的交易量

	return cloud_pay_api_init(&account, &key, &terminal, &ops);
}
static int cloud_pay_request(char* pay_code)
{
	rt_err_t result = RT_EOK;
	
	return result;	
}
static void thread_entry(void *parameter)
{
	char qr_code[30];
	char last_code[18] = {0};
	rt_err_t result;
	result = cloud_pay_init();
	
	while(1)
	{
		if(qr_mq_recv(qr_code, 30)==RT_EOK)
		{
			char *head, *head0, *head1, *end;
			rt_uint8_t len;
			head0 = rt_strstr(qr_code, "AAj]C0");
			head1 = rt_strstr(qr_code, "AAQ]Q1");
			head = head0?head0:head1;
			if(head)
			{	//AAQ]Q1135124350877540729BB
				//AAj]C0135098347648826005BB
				end = rt_strstr(qr_code, "BB\r");
				if(end)
				{		
					len = (rt_uint8_t)(end - head - 6);
					rt_kprintf("len:%d",len);
					if(len==18)
					{
						rt_memcpy(qr_code, head+6, 18);
						qr_code[18] = '\0';
						if(rt_memcmp(qr_code, last_code, 18)==0)
						{
							rt_kprintf("The QR Code repetition!mq:%s", qr_code);
						}
						else
						{
							if(cloud_pay_request(qr_code)==RT_EOK)
							{
								rt_kprintf("cloud pay ok!%s", qr_code);
							}
							else
							{
								rt_kprintf("cloud pay failed!%s", qr_code);
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

static int qr_code_scan_start(void)
{
	rt_err_t result;	
	result = qr_device_init(QR_DEVICE_NAME, QR_BUFFER_SIZE, QR_MQ_MAX_MSGS);
	if(result==RT_EOK)
	{
		rt_thread_t rtt = RT_NULL;
		rtt = rt_thread_create("qr_scan",         //线程名称。
								thread_entry,    //线程入口函数。
								RT_NULL,         //线程入口参数。
								STACK_SIZE,      //线程栈大小。
								PRIORITY,        //线程优先级。
								TIMESLICE);      //时间片Tick。
		if(rtt != RT_NULL)                       //判断线程是否创建成功。
		{
			rt_thread_startup(rtt);             //线程创建成功，启动线程。
		}
		else
		{
			result = RT_ERROR;
		}
	}
	return result;
}
INIT_APP_EXPORT(qr_code_scan_start);
