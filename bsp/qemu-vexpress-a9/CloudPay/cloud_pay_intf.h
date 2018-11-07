#ifndef _CLOUD_PAY_INTF_H_
#define _CLOUD_PAY_INTF_H_

#include "cloud_pay_def.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


	/**
	*	���ý��׽ӿ�ʧ�ܺ󣬿��Ե�������ӿڻ�ȡ����Ĵ���������Ϣ
	*/
	const char* error_utf8(void);

	/**
	*    ��ʼ���ӿ� �������ӿ�֮ǰ����
	**/
	int cloud_pay_api_init(
		Account *account,
		Key *key,
		Terminal *terminal,
		CloudPayApiOps *ops);

	/**
	*    �����ӿ�
	**/
	int cloud_pay_api_fini(void);


	/**
	* ˢ��֧���ӿ�
	* ����ֵ˵��
	*   1. ret = 0 ��ȡorder��state�ж�֧�����
	*   2. ret > 0 ��֧�����صĴ�����status  ������Ϣ���internal status��description
	*		ret = 1 ʧ��
	*		ret = 2 ϵͳ�ڲ�����,��Ҫ����
	*   3. ret < 0 ���ش��� ��������󣬼�����֤��ʧ�ܣ� ��������ʧ�� ���ʧ�� ���ذ���֤��У��ʧ��
	* 	   ret = -1 ��������ʧ��
	*      ret = -2 ���糬ʱ, ��������; ��Ҫȷ��֧�����
	*      ��������
	*	ʹ��error_utf8��ȡ��������
	*/
	int micro_pay(MicroPayRequest *request, MicroPayResponse *response);


	/**
	* ��ѯ�����ӿ�
	* ����ֵ˵��
	*   1. ret = 0 ��ȡ������֧��ҵ��� �Ի�ȡorder������ state�ж�֧�����
	*   2. ret > 0 ��֧�����صĴ�����status  ������Ϣ���internal status��description
	*		ret = 1 ʧ��
	*		ret = 2 ϵͳ�ڲ�����,��Ҫ����
	*		ret = 3 ����������
	*   3. ret < 0 ���ش��� ��������󣬼�����֤��ʧ�ܣ� ��������ʧ�� ���ʧ�� ���ذ���֤��У��ʧ��
	* 	   3.1 ret = -1 ��������ʧ��
	*      3.2 ret = -2 ���糬ʱ, ��������
	*      3.3 ��������
	*	ʹ��error_utf8��ȡ��������
	*/
	int query_order(QueryOrderRequest *request, QueryOrderResponse *response);


	/**
	* �����˿�ӿ�
	* ����ֵ˵��
	*   1. ret = 0 �˿�����ɹ�
	*   2. ret > 0 ��֧�����صĴ�����status  ������Ϣ���internal status��description
	*		ret = 1 ʧ��
	*		ret = 2 ϵͳ�ڲ�����,��Ҫ����
	*   3. ret < 0 ���ش��� ��������󣬼�����֤��ʧ�ܣ� ��������ʧ�� ���ʧ�� ���ذ���֤��У��ʧ��
	* 	   3.1 ret = -1 ��������ʧ��
	*      3.2 ret = -2 ���糬ʱ, ��������
	*      3.3 ��������
	*	ʹ��error_utf8��ȡ��������
	*/
	int refund(RefundRequest *request);


	/**
	* �˿��ѯ�ӿ�
	* �������˵��: countΪRefundOrder�ķ���ռ�ĸ���
	* ����ֵ˵��
	*   1. ret = 0 ��ѯ�˿�ɹ�
				total_count = 0 ��ʾû�ж����˹���
				total_count > 0 ��ʾ��ʶ������˿����; ʹ��refund_order����, ʹ�÷��Լ������ͷ��ڴ�ռ�
	*   2. ret > 0 ��֧�����صĴ�����status  ������Ϣ���internal status��description
	*		ret = 1 ʧ��
	*		ret = 2 ϵͳ�ڲ�����,��Ҫ����
	*   3. ret < 0 ���ش��� ��������󣬼�����֤��ʧ�ܣ� ��������ʧ�� ���ʧ�� ���ذ���֤��У��ʧ��
	* 	   3.1 ret = -1 ��������ʧ��
	*      3.2 ret = -2 ���糬ʱ, ��������
	*      3.3 ��������
	*	ʹ��error_utf8��ȡ��������
	*/
	int query_refund(QueryRefundRequest *request, QueryRefundResponse *response);


	/**
	*   ̽�������Ƿ���ýӿ�(�����Ƿ����)
	*   url�Ĳ���Ϊ����4��
	*	������		pay.qcloud.com
	*	�Ϻ�������  sh.pay.qcloud.com
	*	���������  tj.pay.qcloud.com
	*	����������  sz.pay.qcloud.com
	*   �÷�: �������ʱ��������pingѡ��һ�����õ���������ͨ��set_url�����µ�����֧������
	*/
	int pingp(const char *url);

	/**
	*   ���������ӿ� �������������ӿ�Ĭ��ʹ��������
	*   url�Ĳ���Ϊ����4��
	*	������		pay.qcloud.com
	*	�Ϻ�������  sh.pay.qcloud.com
	*	���������  tj.pay.qcloud.com
	*	����������  sz.pay.qcloud.com
	*/
	void set_url(const char *url);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif 
