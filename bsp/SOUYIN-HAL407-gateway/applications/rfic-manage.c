
/*
 *  IC卡操作程序，完成人机交互功能而不是驱动。
 *  相关驱动程序在RFIC分组内
 *
 *
 *
 *
 */


#include <rtthread.h>
#include "rfic-manage.h"

/*******************************************************************************************/
/*
 *  配置卡的初始化，读写等程序
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
 *  特权卡的初始化，读写等程序
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
 *  普通卡的初始化，读写等程序
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
 *  卡信息保存至系统
 *
 */
void complete_card_work(void)
{
	rt_kprintf("rfic card operition ok ! \n");
}






