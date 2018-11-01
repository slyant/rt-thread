#include "cloud_pay_intf.h"
#include <cJSON.h>

static rt_bool_t cJSON_HasObjectItem(const cJSON *object, const char *string)
{
    return cJSON_GetObjectItem((cJSON*)(cJSON*)object, string) ? 1 : 0;
}

typedef struct _Manager {
	Account account;
	Key key;
	Terminal terminal;
	CloudPayApiOps ops;
}Manager;

static Manager g_manager;
static char g_url[256] = "https://pay.qcloud.com";
static char g_error[4096];

typedef int (*pfRequestComputeAuthenInfo)(const char *in, cJSON **out);

typedef int(*pfResponseFailProcess)(const int status, const int internal_status, const char* description, const double log_id,void *api_response);
typedef int(*pfResponseSuccessProcess)(const int status, const int internal_status, const char* description, const double log_id, const cJSON *response_content, void *api_response);

enum WxpayOrderState
{
	kWxpayOrderStateInit = 1,
	kWxpayOrderStateMicropaySuccess = 2,
	kWxpayOrderStateUnifiedorderSuccess = 3,
	kWxpayOrderStateRefund = 4,
	kWxpayOrderStateMicropayNotpay = 5,
	kWxpayOrderStateUnifiedorderNotpay = 6,
	kWxpayOrderStateClosed = 7,
	kWxpayOrderStateRevoked = 8,
	kWxpayOrderStateUserpaying = 9,
	kWxpayOrderStatePayerror = 10,
	kWxpayOrderStateVoid = 11,
};

//微信退款
enum WxpayRefundOrderState
{
	kWxpayRefundOrderStateInit = 1,  // 退款单初始态
	kWxpayRefundOrderStateSuccess = 2,   // 退款成功
	kWxpayRefundOrderStateFail = 3,    // 退款失败 // 对应REFUNDCLOSE
	kWxpayRefundOrderStateProcessing = 4,    // 退款处理中
	kWxpayRefundOrderStateChange = 5,    // 转入代发，退款到银行发现用户的卡作废或者冻结了，导致原路退款银行卡失败，                                 // 资金回流到子商户的现金帐号，需要子商户人工干预，通过线下或者财付通转账的方式进行退款
	kWxpayRefundOrderStateVoid = 6,   // 作废状态，表示本地有，第三方支付平台没有的订单
};

enum AlipayOrderState
{
kAlipayOrderStateInit = 1,
kAlipayOrderStateSuccess = 2,
kAlipayOrderStateWaitBuyerPay = 4,
kAlipayOrderStateClosed = 5,
kAlipayOrderStateFinish = 6,
kAlipayOrderStateVoid = 7,
};

enum AlipayRefundOrderState
{
	kAlipayRefundStateInit = 1,    // 退款单初始态
	kAlipayRefundStateSuccess = 2,    // 退款单成功态
	kAlipayRefundStateFail = 3,    // 申请退款失败
};

#define CLOUD_PAY_ORDER(response_type_marco,api_response_marco, detail_macro)\
do{\
	response_type_marco *response = (response_type_marco *)api_response_marco;\
	if (!cJSON_HasObjectItem(response_content, detail_macro))\
	{\
		set_error_ansi("响应包无效");\
		return CLOUD_PAY_API_ERROR_RESPONSE_INVALID;\
	}\
	cJSON* detail = cJSON_GetObjectItem((cJSON*)response_content, detail_macro);\
	if (!cJSON_HasObjectItem(detail, "order_content"))\
	{\
		set_error_ansi("响应包无效");\
		return CLOUD_PAY_API_ERROR_RESPONSE_INVALID;\
	}\
	if (!cJSON_HasObjectItem(detail, "pay_mch_key"))\
	{\
		set_error_ansi("响应包无效");\
		return CLOUD_PAY_API_ERROR_RESPONSE_INVALID;\
	}\
	cJSON* order_content = cJSON_GetObjectItem((cJSON*)detail, "order_content");\
	cJSON* pay_mch_key = cJSON_GetObjectItem((cJSON*)detail, "pay_mch_key");\
	char* out_trade_no = cJSON_GetObjectItem((cJSON*)order_content, "out_trade_no")->valuestring;\
	g_manager.ops.pf_mem_cpy(response->order.out_trade_no, out_trade_no, g_manager.ops.pf_str_len(out_trade_no) + 1);\
	if (cJSON_HasObjectItem(order_content, "transaction_id")) {\
		char* transaction_id = cJSON_GetObjectItem((cJSON*)order_content, "transaction_id")->valuestring; \
		g_manager.ops.pf_mem_cpy(response->order.transaction_id, transaction_id, g_manager.ops.pf_str_len(transaction_id) + 1); \
	}\
	response->order.total_fee = cJSON_GetObjectItem((cJSON*)order_content, "total_fee")->valueint;\
	response->order.pay_platform = cJSON_GetObjectItem((cJSON*)pay_mch_key, "pay_platform")->valueint;\
	if (response->order.pay_platform == 1) { \
		if (!cJSON_HasObjectItem(order_content, "wxpay_order_content_ext"))\
		{\
			set_error_ansi("响应包无效");\
			return CLOUD_PAY_API_ERROR_RESPONSE_INVALID;\
		}\
		cJSON* wxpay_order_content_ext = cJSON_GetObjectItem((cJSON*)order_content, "wxpay_order_content_ext");\
		int current_trade_state = cJSON_GetObjectItem((cJSON*)wxpay_order_content_ext, "current_trade_state")->valueint;\
		response->order.state = get_local_state(response->order.pay_platform, current_trade_state);\
	}\
	if (response->order.pay_platform == 2) {\
		if (!cJSON_HasObjectItem(order_content, "alipay_order_content_ext"))\
		{\
			set_error_ansi("响应包无效");\
			return CLOUD_PAY_API_ERROR_RESPONSE_INVALID;\
		}\
		cJSON* alipay_order_content_ext = cJSON_GetObjectItem((cJSON*)order_content, "alipay_order_content_ext");\
		int current_trade_state = cJSON_GetObjectItem((cJSON*)alipay_order_content_ext, "current_trade_state")->valueint;\
		response->order.state = get_local_state(response->order.pay_platform, current_trade_state);\
	}\
	return 0;\
}while (0)

static void set_error_ansi(const char* ansi)
{
	g_manager.ops.pf_ansi_2_utf8(ansi, g_error);
}

static void set_error_utf8(int status, int internal_status, double log_id, const char *error)
{
	g_manager.ops.pf_snprintf(g_error, sizeof(g_error) - 1, 
		"status %d, internal_status %d, log_id %.0f, %s", status, internal_status, log_id, error);
}

// 返回的是authen info
static int request_compute_sign(const char *in, cJSON **out)
{
	char sign_str[1024];
	size_t length = sizeof(sign_str);
	//sign compute
	int ret = g_manager.ops.pf_sign_2_base64(in, g_manager.key.private_key, sign_str, &length);
	if (ret != 0) 
	{
		set_error_ansi("签名失败");
		return CLOUD_PAY_API_ERROR_COMPUTE_SIGN_2_BASE64_FAIL;
	}

	sign_str[length] = 0;
	
	//构造authen info
	cJSON *sign = cJSON_CreateObject();
	cJSON_AddStringToObject (sign, "sign", sign_str);
	cJSON_AddNumberToObject(sign, "sign_type", 1);

	cJSON *authen_info = cJSON_CreateObject();
	cJSON_AddItemToObject(authen_info, "s", sign);

	*out = authen_info;
	return 0;
}

// 返回的是authen info
static int request_compute_authen_code(const char *in, cJSON **out)
{
	char authen_code[65];
	size_t length = sizeof(authen_code);
	//hmac compute
	int ret = g_manager.ops.pf_hmac_sha256(in, g_manager.key.authen_key, authen_code, &length);
	if (ret != 0) 
	{
		set_error_ansi("计算认证码失败");
		return CLOUD_PAY_API_ERROR_COMPUTE_AUTHEN_CODE_FAIL;
	}

	//构造authen info
	cJSON *authen = cJSON_CreateObject();
	cJSON_AddStringToObject(authen, "authen_code", authen_code);
	cJSON_AddNumberToObject(authen, "authen_type", 1);

	cJSON *authen_info = cJSON_CreateObject();
	cJSON_AddItemToObject(authen_info, "a", authen);

	*out = authen_info;
	return 0;
}


static int hit_fail_status(int status)
{
	if (status == 101 ||    // kStatusFailNoRetryNoAuthenCode
		status == 102 ||    // kStatusFailCreateNewReceiptNo
		status == 104)      // kStatusFailNoRetryAuthenCode
	{
		return 1;
	}

	return 0;
}

static int core_process(
	const char *url,
	const char *request_content,
	pfRequestComputeAuthenInfo pf_request_compute_authen_info,
	pfResponseSuccessProcess pf_response_success_process,
	pfResponseFailProcess pf_response_no_success_process,
	void *api_response)
{
	cJSON* authen_info = NULL;
	int ret = pf_request_compute_authen_info(request_content, &authen_info);
	if (ret != 0) 
	{
		RT_ASSERT(ret == CLOUD_PAY_API_ERROR_COMPUTE_AUTHEN_CODE_FAIL || ret == CLOUD_PAY_API_ERROR_COMPUTE_SIGN_2_BASE64_FAIL);
		return ret;
	}

	//组包
	cJSON *request = cJSON_CreateObject();
	cJSON_AddStringToObject(request, "request_content", request_content);
	cJSON_AddItemToObject  (request, "authen_info",     authen_info);

	char* request_str = cJSON_PrintUnformatted(request);
	cJSON_Delete(request);

	//https
	char response_str[4096];
	size_t length = sizeof(response_str);
	ret = g_manager.ops.pf_http_post(url, request_str, response_str, &length);
	if (ret != 0) 
	{
		RT_ASSERT(ret == CLOUD_PAY_API_ERROR_NETWORK_ERROR || ret == CLOUD_PAY_API_ERROR_NETWORK_TIMEOUT);
		return ret;
	}

	//解包
	cJSON* response = cJSON_Parse(response_str);

	if (!cJSON_HasObjectItem(response, "response_content")){
		return CLOUD_PAY_API_ERROR_RESPONSE_INVALID;
	}

	char* response_content_str = cJSON_GetObjectItem((cJSON*)response, "response_content")->valuestring;

	//hmac check
	if (cJSON_HasObjectItem(response, "authen_info")) 
	{
		cJSON* authen_info = cJSON_GetObjectItem((cJSON*)response, "authen_info");
		cJSON* a = cJSON_GetObjectItem((cJSON*)authen_info, "a");

		char *authen_code = cJSON_GetObjectItem((cJSON*)a,"authen_code")->valuestring;

		char authen_code_resp[65];
		size_t length = sizeof(authen_code_resp);
		ret = g_manager.ops.pf_hmac_sha256(response_content_str, g_manager.key.authen_key, authen_code_resp, &length);
		if (ret != 0) 
		{
			return CLOUD_PAY_API_ERROR_COMPUTE_AUTHEN_CODE_RESP_FAIL;
		}
		if (g_manager.ops.pf_str_cmp(authen_code, authen_code_resp) != 0)
		{
			return CLOUD_PAY_API_ERROR_AUTHEN_CODE_CHECK_FAIL;
		}
	}

	//解包
	cJSON* response_content = cJSON_Parse(response_content_str);

	int status = cJSON_GetObjectItem((cJSON*)response_content, "status")->valueint;
	int internal_status = cJSON_GetObjectItem((cJSON*)response_content, "internal_status")->valueint;
	char* description = cJSON_GetObjectItem((cJSON*)response_content, "description")->valuestring;
	double log_id = cJSON_GetObjectItem((cJSON*)response_content, "log_id")->valuedouble;

	set_error_utf8(status, internal_status, log_id, description);

	if (status == 0) 
	{
		return pf_response_success_process(status, internal_status, description, log_id, response_content, api_response);
	}

	if (hit_fail_status(status)) 
	{
		if (pf_response_no_success_process) 
		{
			return pf_response_no_success_process(status, internal_status, description, log_id, api_response);
		}

		return CLOUD_PAY_API_ERROR_CLOUDPAY_FAIL; //云支付的失败
	}

	return CLOUD_PAY_API_ERROR_SYSTEM_ERROR; //云支付的系统内部错误 需要重试
}

int cloud_pay_api_init(
	Account *account,
	Key *key,
	Terminal *terminal,
	CloudPayApiOps *ops)
{
	ops->pf_mem_cpy(&g_manager.account, account, sizeof(Account));
	ops->pf_mem_cpy(&g_manager.key, key, sizeof(Key));
	ops->pf_mem_cpy(&g_manager.terminal, terminal, sizeof(Terminal));
	ops->pf_mem_cpy(&g_manager.ops, ops, sizeof(CloudPayApiOps));

	if (ops->pf_malloc == NULL || ops->pf_free == NULL)
	{
		return -1;
	}

	if (ops->pf_init) 
	{
		return ops->pf_init();
	}
	return 0;
}

int cloud_pay_api_fini()
{
	if (g_manager.ops.pf_fini) 
	{
		return g_manager.ops.pf_fini();
	}

	return 0;
}

static inline cJSON*  set_pay_mch_key(int pay_platform)
{
	cJSON *pay_mch_key = cJSON_CreateObject(); 
	cJSON_AddStringToObject (pay_mch_key, "out_mch_id",     g_manager.account.out_mch_id); 
	cJSON_AddStringToObject (pay_mch_key, "out_sub_mch_id", g_manager.account.out_sub_mch_id); 
	cJSON_AddStringToObject (pay_mch_key, "out_shop_id",    g_manager.account.out_shop_id); 
	if (pay_platform > 0)
	{
		cJSON_AddNumberToObject(pay_mch_key, "pay_platform", pay_platform);
	}

	return pay_mch_key;
}

static inline cJSON*  set_order_client()
{
	cJSON *order_client = cJSON_CreateObject();
	cJSON_AddStringToObject (order_client, "device_id",         g_manager.account.device_id);
	cJSON_AddStringToObject (order_client, "staff_id",          g_manager.account.staff_id);
	cJSON_AddStringToObject (order_client, "sdk_version",       g_manager.terminal.sdk_version);
	cJSON_AddStringToObject (order_client, "machine_no",        g_manager.terminal.machine_no);
	cJSON_AddStringToObject (order_client, "spbill_create_ip",  g_manager.terminal.spbill_create_ip);
	cJSON_AddNumberToObject (order_client, "terminal_type",     g_manager.terminal.terminal_type);
	cJSON_AddNumberToObject (order_client, "sub_terminal_type", g_manager.terminal.sub_terminal_type);
	return order_client;
}

static inline cJSON* set_pay_content(MicroPayRequest *request)
{
	cJSON *pay_content = cJSON_CreateObject();
	cJSON_AddStringToObject (pay_content, "out_trade_no",  request->out_trade_no);
	cJSON_AddStringToObject (pay_content, "author_code",   request->author_code);
	cJSON_AddStringToObject (pay_content, "body",          request->body);
	cJSON_AddNumberToObject (pay_content, "total_fee",	   request->total_fee); // 64位
	cJSON_AddStringToObject (pay_content, "fee_type",      "CNY");
	return pay_content;
}

static inline char* set_micro_pay_request_content(cJSON *pay_mch_key, cJSON *order_client, cJSON *pay_content, char* nonce_str)
{
	cJSON *request_content = cJSON_CreateObject();
	cJSON_AddItemToObject (request_content,  "pay_mch_key",  pay_mch_key);
	cJSON_AddItemToObject (request_content,  "order_client", order_client);
	cJSON_AddItemToObject (request_content,  "pay_content",  pay_content);
	cJSON_AddStringToObject(request_content, "nonce_str",    nonce_str);

	char *s = cJSON_PrintUnformatted(request_content);
	cJSON_Delete(request_content);

	return s;
}

static enum CloudPaySdkLocalState get_local_state(int pay_platform, int state)
{
	enum CloudPaySdkLocalState local_state = KCloudPaySdkLocalStateInit;
	if (pay_platform == 1)
	{
		enum WxpayOrderState current_trade_state = (enum WxpayOrderState)state;
		if (current_trade_state == kWxpayOrderStateMicropaySuccess ||
			current_trade_state == kWxpayOrderStateUnifiedorderSuccess)
		{
			local_state = KCloudPaySdkLocalStateSuccess;
		}

		if (current_trade_state == kWxpayOrderStatePayerror ||
			current_trade_state == kWxpayOrderStateVoid ||
			current_trade_state == kWxpayOrderStateMicropayNotpay ||
			current_trade_state == kWxpayOrderStateRevoked ||
			current_trade_state == kWxpayOrderStateClosed ||
			current_trade_state == kWxpayOrderStateUnifiedorderNotpay)
		{
			local_state = KCloudPaySdkLocalStateFail;
		}

		if (current_trade_state == kWxpayOrderStateRefund)
		{
			local_state = KCloudPaySdkLocalStateRefund;
		}

		if (current_trade_state == kWxpayOrderStateUserpaying)
		{
			local_state = KCloudPaySdkLocalStateUserPaying;
		}
	}

	if (pay_platform == 2)
	{
		enum AlipayOrderState current_trade_state = (enum AlipayOrderState)state;
		if (current_trade_state == kAlipayOrderStateSuccess)
		{
			local_state = KCloudPaySdkLocalStateSuccess;
		}

		if (current_trade_state == kAlipayOrderStateVoid)
		{
			local_state = KCloudPaySdkLocalStateFail;
		}

		if (current_trade_state == kAlipayOrderStateClosed)
		{
			local_state = KCloudPaySdkLocalStateClosed;
		}

		if (current_trade_state == kAlipayOrderStateWaitBuyerPay)
		{
			local_state = KCloudPaySdkLocalStateUserPaying;
		}

	}
	return local_state;
}

static enum CloudPaySdkRefundLocalState get_refund_local_state(int pay_platform, int state)
{
	enum CloudPaySdkRefundLocalState local_state = kCloudPaySdkRefundLocalStateNone;
	if (pay_platform == 1)
	{
		if (state == kWxpayRefundOrderStateInit ||
			state == kWxpayRefundOrderStateProcessing)
		{
			local_state = (enum CloudPaySdkRefundLocalState)1;
		}

		if (state == kWxpayRefundOrderStateSuccess)
		{
			local_state = (enum CloudPaySdkRefundLocalState)2;
		}

		if (state == kWxpayRefundOrderStateFail ||
			state == kWxpayRefundOrderStateChange ||
			state == kWxpayRefundOrderStateVoid)
		{
			local_state = (enum CloudPaySdkRefundLocalState)3;
		}
	}

	if (pay_platform == 2)
	{
		if (state == kAlipayRefundStateInit)
		{
			local_state = (enum CloudPaySdkRefundLocalState)1;
		}

		if (state == kAlipayRefundStateSuccess)
		{
			local_state = (enum CloudPaySdkRefundLocalState)2;
		}

		if (state == kAlipayRefundStateFail)
		{
			local_state = (enum CloudPaySdkRefundLocalState)3;
		}
	}
	return local_state;
}

static int micro_pay_response_success_process(
	const int status,
	const int internal_status,
	const char* description,
	const double log_id,
	const cJSON *response_content,
	void *api_response)
{
	CLOUD_PAY_ORDER(MicroPayResponse, api_response, "micro_pay");
}

int micro_pay(MicroPayRequest *request, MicroPayResponse *response)
{
	char* request_content_str  = 
		set_micro_pay_request_content(set_pay_mch_key(request->pay_platform), 
			                          set_order_client(), 
									  set_pay_content(request),
									   request->nonce_str);
	char url[256];
	g_manager.ops.pf_snprintf(url, sizeof(url) - 1, "%s/cpay/micro_pay", g_url);

	return  core_process(url,
						 request_content_str,
						 request_compute_authen_code,
						 micro_pay_response_success_process,
						 NULL,
						 response);
}

static inline char* set_query_order_request_content(cJSON *pay_mch_key, cJSON *order_client, QueryOrderRequest *request)
{
	cJSON *request_content = cJSON_CreateObject();
	cJSON_AddItemToObject  (request_content, "pay_mch_key",  pay_mch_key);
	cJSON_AddItemToObject  (request_content, "order_client", order_client);
	cJSON_AddStringToObject(request_content, "out_trade_no", request->out_trade_no);
	cJSON_AddStringToObject(request_content, "nonce_str",    request->nonce_str);

	char *s = cJSON_PrintUnformatted(request_content);
	cJSON_Delete(request_content);

	return s;
}

static int query_order_response_fail_process(
	const int status,
	const int internal_status,
	const char* description,
	const double log_id,
	void *api_response)
{
	if (internal_status == 400) 
	{
		return CLOUD_PAY_API_ERROR_ORDER_DONTEXIST; //订单不存在
	}

	return CLOUD_PAY_API_ERROR_CLOUDPAY_FAIL;  //查单失败
}

static int query_order_response_success_process(
	const int status,
	const int internal_status,
	const char* description,
	const double log_id,
	const cJSON *response_content,
	void *api_response)
{
	CLOUD_PAY_ORDER(QueryOrderResponse, api_response, "query_order");
}

int query_order(QueryOrderRequest *request, QueryOrderResponse *response)
{
	char* request_content_str =
		set_query_order_request_content(set_pay_mch_key(0), set_order_client(), request);

	char url[256];
	g_manager.ops.pf_snprintf(url, sizeof(url) - 1, "%s/cpay/query_order", g_url);

	return  core_process(url,
		request_content_str,
		request_compute_authen_code,
		query_order_response_success_process,
		query_order_response_fail_process,
		response);
}

static int refund_response_success_process(
	const int status,
	const int internal_status,
	const char* description,
	const double log_id,
	const cJSON *response_content,
	void *api_response)
{
	return status; //status == 0 退款受理成功
}

static inline cJSON* set_refund_content(RefundRequest *request)
{
	cJSON *refund_content = cJSON_CreateObject();
	cJSON_AddStringToObject(refund_content, "out_trade_no",  request->out_trade_no);
	cJSON_AddStringToObject(refund_content, "out_refund_no", request->out_refund_no);
	cJSON_AddNumberToObject(refund_content, "total_fee",     request->total_fee); // 64位
	cJSON_AddNumberToObject(refund_content, "refund_fee",    request->refund_fee); // 64位
	cJSON_AddStringToObject(refund_content, "refund_fee_type", "CNY");
	return refund_content;
}

static inline char* set_refund_request_content(cJSON *pay_mch_key, cJSON *order_client, cJSON *refund_content, char *nonce_str)
{
	cJSON *request_content = cJSON_CreateObject();
	cJSON_AddItemToObject  (request_content, "pay_mch_key",    pay_mch_key);
	cJSON_AddItemToObject  (request_content, "order_client",   order_client);
	cJSON_AddItemToObject  (request_content, "refund_content", refund_content);
	cJSON_AddStringToObject(request_content, "nonce_str",      nonce_str);

	char *s = cJSON_PrintUnformatted(request_content);
	cJSON_Delete(request_content);

	return s;
}

int refund(RefundRequest *request)
{
	char* request_content_str =
		set_refund_request_content(set_pay_mch_key(request->pay_platform),       
			                       set_order_client(), 
			                       set_refund_content(request),
			                       request->nonce_str);
	char url[256];
	g_manager.ops.pf_snprintf(url, sizeof(url) - 1, "%s/cpay/refund", g_url);

	return  core_process(url,
		request_content_str,
		request_compute_sign,
		refund_response_success_process,
		NULL,
		NULL);
}

static inline char* set_query_refund_request_content(cJSON *pay_mch_key, cJSON *order_client, QueryRefundRequest *request)
{
	cJSON *request_content = cJSON_CreateObject();
	cJSON_AddItemToObject(request_content, "pay_mch_key", pay_mch_key);
	cJSON_AddItemToObject(request_content, "order_client", order_client);
	cJSON_AddStringToObject(request_content, "out_trade_no", request->out_trade_no);
	if (g_manager.ops.pf_str_len(request->out_refund_no) > 0) 
	{
		cJSON_AddStringToObject(request_content, "out_refund_no", request->out_refund_no);
	}
	cJSON_AddStringToObject(request_content, "nonce_str", request->nonce_str);

	char *s = cJSON_PrintUnformatted(request_content);
	cJSON_Delete(request_content);

	return s;
}

static void set_refund_order_zero(RefundOrder *refund_orders, int count)
{
	int idx;
	for (idx = 0; idx < count; idx++) 
	{
		RefundOrder *refund_order = &(refund_orders[idx]);
		refund_order->out_trade_no[0] = 0;
		refund_order->out_refund_no[0] = 0;
		refund_order->refund_id[0] = 0;
		refund_order->total_fee = 0;
		refund_order->refund_fee = 0;
		refund_order->pay_platform = 0;
		refund_order->state = kCloudPaySdkRefundLocalStateNone;
	}
}

static int query_refund_response_fail_process(
	const int status,
	const int internal_status,
	const char* description,
	const double log_id,
	void *api_response)
{
	QueryRefundResponse *response = (QueryRefundResponse *)api_response;

	if (internal_status == 426)
	{
		response->refund_order = NULL;
		response->count = 0;
		return 0;
	}

	return CLOUD_PAY_API_ERROR_CLOUDPAY_FAIL;
}

static int query_refund_response_success_process(
	const int status,
	const int internal_status,
	const char* description,
	const double log_id,
	const cJSON *response_content,
	void *api_response)
{
	QueryRefundResponse *response = (QueryRefundResponse *)api_response; 
	
	if (!cJSON_HasObjectItem(response_content, "query_refund_order"))
	{
		return CLOUD_PAY_API_ERROR_RESPONSE_INVALID; 
	}

	cJSON* query_refund_order = cJSON_GetObjectItem((cJSON*)(cJSON*)response_content, "query_refund_order");
	if (!cJSON_HasObjectItem(query_refund_order, "refund_order_content"))
	{
		//return CLOUD_PAY_API_ERROR_RESPONSE_INVALID;
		response->refund_order = NULL;
		response->count = 0;
		return 0;
	}

	cJSON* refund_order_content = cJSON_GetObjectItem((cJSON*)query_refund_order, "refund_order_content");

	int count = cJSON_GetArraySize(refund_order_content);

	if (count == 0)
	{
		response->refund_order = NULL;
		response->count = 0;
		return 0;
	}

	RefundOrder *refund_orders = g_manager.ops.pf_malloc(sizeof(RefundOrder) * count);
	set_refund_order_zero(refund_orders, count);

	int idx = 0; 
	for (idx = 0; idx < count; idx++) 
	{
		cJSON* refund_order_json = cJSON_GetArrayItem(refund_order_content, idx);
		RefundOrder * refund_order = &(refund_orders[idx]);

		char* out_trade_no = cJSON_GetObjectItem((cJSON*)refund_order_json, "out_trade_no")->valuestring;
		g_manager.ops.pf_mem_cpy(refund_order->out_trade_no, out_trade_no, g_manager.ops.pf_str_len(out_trade_no) + 1);

		char* out_refund_no = cJSON_GetObjectItem((cJSON*)refund_order_json, "out_refund_no")->valuestring;
		g_manager.ops.pf_mem_cpy(refund_order->out_refund_no, out_refund_no, g_manager.ops.pf_str_len(out_refund_no) + 1);

		if (cJSON_HasObjectItem(refund_order_json, "refund_id")) 
		{
			char* refund_id = cJSON_GetObjectItem((cJSON*)refund_order_json, "refund_id")->valuestring;
			g_manager.ops.pf_mem_cpy(refund_order->refund_id, refund_id, g_manager.ops.pf_str_len(refund_id) + 1);
		}

		refund_order->total_fee = cJSON_GetObjectItem((cJSON*)refund_order_json, "total_fee")->valueint;
		refund_order->refund_fee = cJSON_GetObjectItem((cJSON*)refund_order_json, "refund_fee")->valueint;

		cJSON* pay_mch_key = cJSON_GetObjectItem((cJSON*)query_refund_order, "pay_mch_key");
		refund_order->pay_platform = cJSON_GetObjectItem((cJSON*)pay_mch_key, "pay_platform")->valueint;

		if (refund_order->pay_platform == 1) 
		{
			if (!cJSON_HasObjectItem(refund_order_json, "wxpay_refund_order_content_ext"))
			{
				return CLOUD_PAY_API_ERROR_RESPONSE_INVALID;
			}
			cJSON* wxpay_refund_order_content_ext = cJSON_GetObjectItem((cJSON*)refund_order_json, "wxpay_refund_order_content_ext");
			int state = cJSON_GetObjectItem((cJSON*)wxpay_refund_order_content_ext, "state")->valueint;
			refund_order->state = get_refund_local_state(refund_order->pay_platform, state);
		}

		if (refund_order->pay_platform == 2) 
		{
			if (!cJSON_HasObjectItem(refund_order_json, "alipay_refund_order_content_ext"))
			{
				return CLOUD_PAY_API_ERROR_RESPONSE_INVALID;
			}
			cJSON* alipay_refund_order_content_ext = cJSON_GetObjectItem((cJSON*)refund_order_json, "alipay_refund_order_content_ext");
			int state = cJSON_GetObjectItem((cJSON*)alipay_refund_order_content_ext, "refund_status")->valueint;
			refund_order->state = get_refund_local_state(refund_order->pay_platform, state);
		}
	}

	response->count = count;
	response->refund_order = refund_orders;
	return 0; 
}

int query_refund(QueryRefundRequest *request, QueryRefundResponse *response)
{
	char* request_content_str =
		set_query_refund_request_content(set_pay_mch_key(request->pay_platform), set_order_client(), request);

	char url[256];
	g_manager.ops.pf_snprintf(url, sizeof(url) - 1, "%s/cpay/query_refund_order", g_url);

	return  core_process(url,
		request_content_str,
		request_compute_authen_code,
		query_refund_response_success_process,
		query_refund_response_fail_process,
		response);
}

int ping(const char *url) 
{
	char url_inf[256];
	g_manager.ops.pf_snprintf(url_inf, sizeof(url_inf) - 1, "%s/cpay/ping", url);

	char response_str[256];
	size_t length = sizeof(response_str);
	return g_manager.ops.pf_http_post(url_inf, "CPing", response_str, &length);
}

void set_url(const char *url)
{
	g_manager.ops.pf_snprintf(g_url, sizeof(g_url) - 1, "%s", url);
}


const char* error_utf8()
{
	return g_error;
}
