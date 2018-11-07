#ifndef _CLOUD_PAY_DEF_H_
#define _CLOUD_PAY_DEF_H_

/**
*  ˵��: C SDK �ڲ��ļ�Ϊ cloud_pay_def.h, cloud_pay_intf.h �� cloud_pay.c; ʵ����������cJSON, �ļ�ΪcJSON.h �� cJSON.c; 
*		 ʹ���߽���5��Դ�ļ�ֱ����ȥ���뼴��
*		 cloud_pay_def.h ���ݽṹ����  , cloud_pay_intf.h �ӿ�����
*        Ϊ�˽�C SDK��������, ��������Ҫ���ȵ���cloud_pay_api_init ��ʼ������ܵ���micro_pay/ query_order/ refund�Ƚӿ�
*        Ϊ�˼��ݶ�ƽ̨, ʹ������Ҫ�Լ�ʵ��һЩ�����Ĺ���, ���ڴ濽��, HMAC_SHA256��֤�㷨, RSASSA-PSS-2048-SHA256ǩ���㷨
*		 
*/

#include "stdint.h"
#include <stddef.h>
#include <rtthread.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
	/**
	* �������ݽӿ�
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
		int sub_terminal_type; // ���ߵ�����. ����֧�����䣬 ����ͳ��ĳ�����ߵĽ�����
		char sdk_version[32];
		char machine_no[32];
		char spbill_create_ip[32];
	}Terminal;

	//����״̬
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
	* ������ʹ�õ��ֶαȽ��� ����ֻ���ṩ�˼����ؼ��ֶ�
	* �����Ҫ������ֶ� ���������޸�SDK�Ĵ���
	*/
	typedef struct _Order {
		long long total_fee;   //������� ��λ��
		char out_trade_no[CLOUD_PAY_ORDER_LENGTH]; //������
		char transaction_id[CLOUD_PAY_ORDER_LENGTH];
		int pay_platform;      //֧���� or ΢��֧��
		enum CloudPaySdkLocalState state;             //����״̬
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
	*	pfHttpPost �����������Լ����𷵻���������������ֻ����������������
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
	*	�����붨��
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
		int pay_platform; //֧���� or ΢��֧��
		char author_code[33]; // ������
		long long total_fee; //������� ��λ��
		char out_trade_no[CLOUD_PAY_ORDER_LENGTH]; //��������Ҫʹ�����Լ�����, ��������֧������ǰ׺(20λ) + ʹ�÷��Զ��岿��, ������ÿ�����20Ϊ����
		char body[256]; //��Ʒ˵��
		char nonce_str[64];//�����
	}MicroPayRequest;

	typedef struct _MicroPayResponse {
		Order order;
	}MicroPayResponse;

	typedef struct _QueryOrderRequest {
		char out_trade_no[CLOUD_PAY_ORDER_LENGTH]; //��������Ҫʹ�����Լ�����, ��������֧������ǰ׺(20λ) + ʹ�÷��Զ��岿��, ������ÿ�����20Ϊ����
		char nonce_str[64];//�����
	}QueryOrderRequest;


	typedef struct _QueryOrderResponse {
		Order order;
	}QueryOrderResponse;


	typedef struct _RefundRequest {
		int pay_platform; //֧���� or ΢��֧��
		char out_trade_no[CLOUD_PAY_ORDER_LENGTH]; //��������Ҫʹ�����Լ�����, ��������֧������ǰ׺(20λ) + ʹ�÷��Զ��岿��, ������ÿ�����20Ϊ����
		char out_refund_no[CLOUD_PAY_ORDER_LENGTH]; //�˿����Ҫʹ�����Լ�����, ��������֧������ǰ׺(20λ) + ʹ�÷��Զ��岿��
		long long total_fee;   //������� ��λ��
		long long refund_fee;   //�˿��� ��λ��
		char nonce_str[64];//�����
	}RefundRequest;

	/**
	*	���ֻ�������Ų�ѯ�˿�뽫out_refund_no��ʼ��Ϊ0
	*/
	typedef struct _QueryRefundRequest {
		int pay_platform; //֧���� or ΢��֧��
		char out_trade_no[CLOUD_PAY_ORDER_LENGTH]; //��������Ҫʹ�����Լ�����, ��������֧������ǰ׺(20λ) + ʹ�÷��Զ��岿��, ������ÿ�����20Ϊ����
		char out_refund_no[CLOUD_PAY_ORDER_LENGTH]; //�˿����Ҫʹ�����Լ�����, ��������֧������ǰ׺(20λ) + ʹ�÷��Զ��岿��
		char nonce_str[64];//�����
	}QueryRefundRequest;

	/**
	* ������ʹ�õ��ֶαȽ��� ����ֻ���ṩ�˼����ؼ��ֶ�
	* �����Ҫ������ֶ� ���������޸�SDK�Ĵ���
	*/
	typedef struct _RefundOrder {
		char out_trade_no[CLOUD_PAY_ORDER_LENGTH]; //������
		char out_refund_no[CLOUD_PAY_ORDER_LENGTH]; //�˿����
		char refund_id[CLOUD_PAY_ORDER_LENGTH]; //�˿����
		long long total_fee;   //������� ��λ��
		long long refund_fee;   //�˿��� ��λ��
		int pay_platform;      //֧���� or ΢��֧��
		enum CloudPaySdkRefundLocalState state;             //�˿״̬
	}RefundOrder;

	typedef struct _QueryRefundResponse {
		RefundOrder *refund_order;
		int count;
	}QueryRefundResponse;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif 
