
/*
 *  IC��������������˻��������ܶ�����������
 *  �������������RFIC������
 *
 *
 *
 *
 */


#include <rtthread.h>
#include "rfic-manage.h"

/*******************************************************************************************/
/*
 *  ���ÿ��ĳ�ʼ������д�ȳ���
 *
 */

void manager_card_init(void)
{
	rt_kprintf("manage card init ok ! \n");
}

void manager_card_make(void)
{
	rt_kprintf("manage card make ok ! \n");
}

void manager_card_clear(void)
{
	rt_kprintf("manage card clear done ! \n");
}

/*******************************************************************************************/
/*
 *  ��Ȩ���ĳ�ʼ������д�ȳ���
 *
 */

void privilege_card_init(void)
{
	rt_kprintf("privilege card init ok ! \n");
}

void privilege_card_make(void)
{
	rt_kprintf("privilege card make ok ! \n");
}

void privilege_card_clear(void)
{
	rt_kprintf("privilege card clear done ! \n");
}

/*******************************************************************************************/
/*
 *  ��ͨ���ĳ�ʼ������д�ȳ���
 *
 */

void normaal_card_init(void)
{
	rt_kprintf("normaal card init ok ! \n");
}

void normaal_card_make(void)
{
	rt_kprintf("normaal card make ok ! \n");
}

void normaal_card_clear(void)
{
	rt_kprintf("normaal card clear domne ! \n");
}

/*******************************************************************************************/
/*
 *  ����Ϣ������ϵͳ
 *
 */
void complete_card_work(void)
{
	rt_kprintf("rfic card operition ok ! \n");
}






