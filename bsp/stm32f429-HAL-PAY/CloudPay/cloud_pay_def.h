#ifndef _CLOUD_PAY_DEF_H_
#define _CLOUD_PAY_DEF_H_

/**
*  说明: C SDK 内部文件为 cloud_pay_def.h, cloud_pay_intf.h 和 cloud_pay.c; 实现中引用了cJSON, 文件为cJSON.h 和 cJSON.c; 
*		 使用者将这5个源文件直接拿去编译即可
*		 cloud_pay_def.h 数据结构定义  , cloud_pay_intf.h 接口声明
*        为了将C SDK运行以来, 调用者需要首先调用cloud_pay_api_init 初始化后才能调用micro_pay/ query_order/ refund等接口
*        为了兼容多平台, 使用者需要自己实现一些基本的功能, 如内存拷贝, HMAC_SHA256认证算法, RSASSA-PSS-2048-SHA256签名算法
*		 
*/

#include "stdint.h"
#include <stddef.h>
#include <rtthread.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
	/**
	* 公用数据接口
	*/
	#define CLOUD_PAY_ACCOUNT_LENGTH  33
	#define CLOUD_PAY_AUTHEN_KEY_LENGTH  33

	#define CLOUD_PAY_ORDER_LENGTH  64

	typedef struct _Account {
		char out_mch_id[CLOUD_PAY_ACCOUNT_LENGTH];
		char out_sub_mch_id[CLOUD_PAY_ACCOUNT_LENGTH];
		char out_shop_id[CLOUD_PAY_ACCOUNT_LENGTH];
		char device_id[CLOUD_PAY_ACCOUNT_LENGTH];
		char staff_id[CLOUD_PAY_ACCOUNT_LENGTH];
	}Account;

	typedef struct _Key {
		char authen_key[CLOUD_PAY_AUTHEN_KEY_LENGTH];
		char private_key[4096];
	}Key;

	typedef struct _Terminal {
		int terminal_type; // 1 windows 2 linux 3 android
		int sub_terminal_type; // 机具的类型. 找云支付分配， 可以统计某个机具的交易量
		char sdk_version[32];
		char machine_no[32];
		char spbill_create_ip[32];
	}Terminal;

	//本地状态
	enum CloudPaySdkLocalState
	{
		KCloudPaySdkLocalStateNone		 = 0,
		KCloudPaySdkLocalStateInit		 = 1,
		KCloudPaySdkLocalStateUserPaying = 2,
		KCloudPaySdkLocalStateSuccess	 = 3,
		KCloudPaySdkLocalStateFail		 = 4,
		KCloudPaySdkLocalStateReverse	 = 5,
		KCloudPaySdkLocalStateRefund	 = 6,
		KCloudPaySdkLocalStateClosed	 = 7,
	};

	enum CloudPaySdkRefundLocalState
	{
		kCloudPaySdkRefundLocalStateNone		= 0,
		kCloudPaySdkRefundLocalStateProcessing	= 1,
		kCloudPaySdkRefundLocalStateSuccess		= 2,
		kCloudPaySdkRefundLocalStateFail		= 3,
	};


	/**
	* 机具上使用的字段比较少 这里只是提供了几个关键字段
	* 如果需要更多的字段 可以自行修改SDK的代码
	*/
	typedef struct _Order {
		long long total_fee;   //订单金额 单位分
		char out_trade_no[CLOUD_PAY_ORDER_LENGTH]; //订单号
		char transaction_id[CLOUD_PAY_ORDER_LENGTH];
		int pay_platform;      //支付宝 or 微信支付
		enum CloudPaySdkLocalState state;             //订单状态
	}Order;

	typedef int(*pfHmacSha256)(const char *in, const char* key, char *out, size_t *out_size);
	typedef int(*pfRSA_SSA_PSS_2048_SHA256_2_Base64)(const char *in, const char* private_key, char *out, size_t *out_size);
	typedef void *(*pfMemCpy)(void *dst, const void *src, rt_ubase_t count);
	typedef rt_int32_t (*pfStrCmp)(const char *cs, const char *ct);
	typedef rt_size_t (*pfStrLen)(const char *s);
	typedef void *(*pfMalloc)(rt_size_t size);
	typedef void(*pfFree)(void *ptr);
	typedef rt_int32_t (*pfSnprintf)(char *buf, rt_size_t size, const char *fmt, ...);
	typedef int (*pfAnsi2Utf8)(const char *ansi, char *utf8);

	/**
	*	pfHttpPost 函数调用者自己负责返回下面两个错误码只能是这两个错误码
	*	CLOUD_PAY_API_ERROR_NETWORK_ERROR 
	*	CLOUD_PAY_API_ERROR_NETWORK_TIMEOUT
	*/
	typedef int(*pfHttpPost)(const char *url, const char *request, char *response, size_t *response_length);

	typedef int(*pfInit)();
	typedef int(*pfFini)();

	typedef struct _CloudPayApiOps {
		pfHmacSha256 pf_hmac_sha256;
		pfRSA_SSA_PSS_2048_SHA256_2_Base64 pf_sign_2_base64;
		pfMemCpy pf_mem_cpy;
		pfStrCmp pf_str_cmp;
		pfStrLen pf_str_len;
		pfMalloc pf_malloc;
		pfFree  pf_free;
		pfSnprintf pf_snprintf;
		pfAnsi2Utf8 pf_ansi_2_utf8;
		pfHttpPost pf_http_post;
		pfInit pf_init;
		pfFini pf_fini;
	}CloudPayApiOps;

	/**
	*	错误码定义
	*/
	#define CLOUD_PAY_API_SUCCESS								 0
	#define CLOUD_PAY_API_ERROR_CLOUDPAY_FAIL					 1
	#define CLOUD_PAY_API_ERROR_SYSTEM_ERROR					 2
	#define CLOUD_PAY_API_ERROR_ORDER_DONTEXIST					 3
	#define CLOUD_PAY_API_ERROR_NETWORK_ERROR					-1
	#define CLOUD_PAY_API_ERROR_NETWORK_TIMEOUT					-2
	#define CLOUD_PAY_API_ERROR_RESPONSE_INVALID				-3
	#define CLOUD_PAY_API_ERROR_AUTHEN_CODE_CHECK_FAIL			-4
	#define CLOUD_PAY_API_ERROR_COMPUTE_AUTHEN_CODE_FAIL		-5
	#define CLOUD_PAY_API_ERROR_COMPUTE_AUTHEN_CODE_RESP_FAIL   -6
	#define CLOUD_PAY_API_ERROR_COMPUTE_SIGN_2_BASE64_FAIL		-7
	#define CLOUD_PAY_API_ERROR_HTTP_POST_FAIL					-8

	typedef struct _MicroPayRequest {
		int pay_platform; //支付宝 or 微信支付
		char author_code[33]; // 付款码
		long long total_fee; //订单金额 单位分
		char out_trade_no[CLOUD_PAY_ORDER_LENGTH]; //订单号需要使用者自己生成, 这里是云支付订单前缀(20位) + 使用方自定义部分, 订单最好控制再20为左右
		char body[256]; //商品说明
		char nonce_str[64];//随机数
	}MicroPayRequest;

	typedef struct _MicroPayResponse {
		Order order;
	}MicroPayResponse;

	typedef struct _QueryOrderRequest {
		char out_trade_no[CLOUD_PAY_ORDER_LENGTH]; //订单号需要使用者自己生成, 这里是云支付订单前缀(20位) + 使用方自定义部分, 订单最好控制再20为左右
		char nonce_str[64];//随机数
	}QueryOrderRequest;


	typedef struct _QueryOrderResponse {
		Order order;
	}QueryOrderResponse;


	typedef struct _RefundRequest {
		int pay_platform; //支付宝 or 微信支付
		char out_trade_no[CLOUD_PAY_ORDER_LENGTH]; //订单号需要使用者自己生成, 这里是云支付订单前缀(20位) + 使用方自定义部分, 订单最好控制再20为左右
		char out_refund_no[CLOUD_PAY_ORDER_LENGTH]; //退款单号需要使用者自己生成, 这里是云支付订单前缀(20位) + 使用方自定义部分
		long long total_fee;   //订单金额 单位分
		long long refund_fee;   //退款金额 单位分
		char nonce_str[64];//随机数
	}RefundRequest;

	/**
	*	如果只按订单号查询退款单请将out_refund_no初始化为0
	*/
	typedef struct _QueryRefundRequest {
		int pay_platform; //支付宝 or 微信支付
		char out_trade_no[CLOUD_PAY_ORDER_LENGTH]; //订单号需要使用者自己生成, 这里是云支付订单前缀(20位) + 使用方自定义部分, 订单最好控制再20为左右
		char out_refund_no[CLOUD_PAY_ORDER_LENGTH]; //退款单号需要使用者自己生成, 这里是云支付订单前缀(20位) + 使用方自定义部分
		char nonce_str[64];//随机数
	}QueryRefundRequest;

	/**
	* 机具上使用的字段比较少 这里只是提供了几个关键字段
	* 如果需要更多的字段 可以自行修改SDK的代码
	*/
	typedef struct _RefundOrder {
		char out_trade_no[CLOUD_PAY_ORDER_LENGTH]; //订单号
		char out_refund_no[CLOUD_PAY_ORDER_LENGTH]; //退款订单号
		char refund_id[CLOUD_PAY_ORDER_LENGTH]; //退款订单号
		long long total_fee;   //订单金额 单位分
		long long refund_fee;   //退款金额 单位分
		int pay_platform;      //支付宝 or 微信支付
		enum CloudPaySdkRefundLocalState state;             //退款单状态
	}RefundOrder;

	typedef struct _QueryRefundResponse {
		RefundOrder *refund_order;
		int count;
	}QueryRefundResponse;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif 
