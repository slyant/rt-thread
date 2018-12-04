#ifndef _CLOUD_PAY_INTF_H_
#define _CLOUD_PAY_INTF_H_

#include "cloud_pay_def.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


	/**
	*	调用交易接口失败后，可以调用这个接口获取具体的错误描述信息
	*/
	const char* error_utf8(void);

	/**
	*    初始化接口 再其他接口之前调用
	**/
	int cloud_pay_api_init(
		Account *account,
		Key *key,
		Terminal *terminal,
		CloudPayApiOps *ops);

	/**
	*    结束接口
	**/
	int cloud_pay_api_fini(void);


	/**
	* 刷卡支付接口
	* 返回值说明
	*   1. ret = 0 获取order的state判断支付结果
	*   2. ret > 0 云支付返回的错误码status  描述信息里给internal status和description
	*		ret = 1 失败
	*		ret = 2 系统内部错误,需要重试
	*   3. ret < 0 本地错误 如参数错误，计算认证码失败， 网络请求失败 解包失败 返回包认证码校验失败
	* 	   ret = -1 网络请求失败
	*      ret = -2 网络超时, 可以重试; 需要确认支付结果
	*      其他错误
	*	使用error_utf8获取错误描述
	*/
	int micro_pay(MicroPayRequest *request, MicroPayResponse *response);


	/**
	* 查询订单接口
	* 返回值说明
	*   1. ret = 0 获取到了云支付业务包 以获取order的内容 state判断支付结果
	*   2. ret > 0 云支付返回的错误码status  描述信息里给internal status和description
	*		ret = 1 失败
	*		ret = 2 系统内部错误,需要重试
	*		ret = 3 订单不存在
	*   3. ret < 0 本地错误 如参数错误，计算认证码失败， 网络请求失败 解包失败 返回包认证码校验失败
	* 	   3.1 ret = -1 网络请求失败
	*      3.2 ret = -2 网络超时, 可以重试
	*      3.3 其他错误
	*	使用error_utf8获取错误描述
	*/
	int query_order(QueryOrderRequest *request, QueryOrderResponse *response);


	/**
	* 申请退款接口
	* 返回值说明
	*   1. ret = 0 退款受理成功
	*   2. ret > 0 云支付返回的错误码status  描述信息里给internal status和description
	*		ret = 1 失败
	*		ret = 2 系统内部错误,需要重试
	*   3. ret < 0 本地错误 如参数错误，计算认证码失败， 网络请求失败 解包失败 返回包认证码校验失败
	* 	   3.1 ret = -1 网络请求失败
	*      3.2 ret = -2 网络超时, 可以重试
	*      3.3 其他错误
	*	使用error_utf8获取错误描述
	*/
	int refund(RefundRequest *request);


	/**
	* 退款查询接口
	* 请求参数说明: count为RefundOrder的分配空间的个数
	* 返回值说明
	*   1. ret = 0 查询退款成功
				total_count = 0 表示没有订单退过款
				total_count > 0 表示这笔订单的退款单个数; 使用refund_order返回, 使用方自己负责释放内存空间
	*   2. ret > 0 云支付返回的错误码status  描述信息里给internal status和description
	*		ret = 1 失败
	*		ret = 2 系统内部错误,需要重试
	*   3. ret < 0 本地错误 如参数错误，计算认证码失败， 网络请求失败 解包失败 返回包认证码校验失败
	* 	   3.1 ret = -1 网络请求失败
	*      3.2 ret = -2 网络超时, 可以重试
	*      3.3 其他错误
	*	使用error_utf8获取错误描述
	*/
	int query_refund(QueryRefundRequest *request, QueryRefundResponse *response);


	/**
	*   探测域名是否可用接口(网络是否可用)
	*   url的参数为下面4个
	*	主域名		pay.qcloud.com
	*	上海子域名  sh.pay.qcloud.com
	*	天津子域名  tj.pay.qcloud.com
	*	深圳子域名  sz.pay.qcloud.com
	*   用法: 网络故障时，可以先ping选择一个可用的域名，再通过set_url设置新的域名支付即可
	*/
	int pingp(const char *url);

	/**
	*   设置域名接口 如果不调用这个接口默认使用主域名
	*   url的参数为下面4个
	*	主域名		pay.qcloud.com
	*	上海子域名  sh.pay.qcloud.com
	*	天津子域名  tj.pay.qcloud.com
	*	深圳子域名  sz.pay.qcloud.com
	*/
	void set_url(const char *url);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif 
