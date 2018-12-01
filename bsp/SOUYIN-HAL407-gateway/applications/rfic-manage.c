
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
#include "ic_card_protocol.h"

/*******************************************************************************************/
/*
 *  密钥卡的初始化，读写等程序
 *
 */

uint8_t user_data[16]={0}, use_bk_card_id[4];

void key_card_init(void)
{
	if(card_init(ROOT_CARD))
	{
		rt_kprintf("key card reset ok ! \n");
		creat_sys_key();
	}
	else rt_kprintf("key card reset faulted ! \n");
}

void key_card_make(void)
{
	if(write_inf_card(ROOT_CARD,user_data,16,use_bk_card_id))
		rt_kprintf("key card make ok ! \n");
	else rt_kprintf("key card make faulted ! \n");
}

void key_card_clear(void)
{
	if(card_reset(ROOT_CARD))
		rt_kprintf("key card clear done ! \n");
	else rt_kprintf("key card clear faulted ! \n");
}

/*******************************************************************************************/
/*
 *  配置卡的初始化，读写等程序
 *
 */

void manager_card_init(void)
{
	if(card_init(SYS_CARD))
		rt_kprintf("manage card init ok ! \n");
	else rt_kprintf("manage card init faulted ! \n");
}

void manager_card_make(void)
{
	if(write_inf_card(SYS_CARD,user_data,16,use_bk_card_id))
		rt_kprintf("manage card make ok ! \n");
	else rt_kprintf("manage card make faulted ! \n");
}

void manager_card_clear(void)
{
	if(card_reset(SYS_CARD))
		rt_kprintf("manage card clear done ! \n");
	else rt_kprintf("manage card clear faulted ! \n");
}

/*******************************************************************************************/
/*
 *  特权卡的初始化，读写等程序
 *
 */

void privilege_card_init(void)
{
	if(card_init(SYS_CARD))
		rt_kprintf("privilege card init ok ! \n");
	else rt_kprintf("privilege card init faulted ! \n");
}

void privilege_card_make(void)
{
	if(write_inf_card(SYS_CARD,user_data,16,use_bk_card_id))
		rt_kprintf("privilege card make ok ! \n");
	else rt_kprintf("privilege card make faulted ! \n");
}

void privilege_card_clear(void)
{
	if(card_reset(SYS_CARD))
		rt_kprintf("privilege card clear done ! \n");
	else rt_kprintf("privilege card clear faulted ! \n");
}

/*******************************************************************************************/
/*
 *  普通卡的初始化，读写等程序
 *
 */

void normaal_card_init(void)
{
	if(card_init(SYS_CARD))
		rt_kprintf("normaal card init ok ! \n");
	else rt_kprintf("normaal card init faulted ! \n");
}

void normaal_card_make(void)
{
	if(write_inf_card(SYS_CARD,user_data,16,use_bk_card_id))
		rt_kprintf("normaal card make ok ! \n");
	else rt_kprintf("normaal card make faulted ! \n");
}

void normaal_card_clear(void)
{
	if(card_reset(SYS_CARD))
		rt_kprintf("normaal card clear domne ! \n");
	else rt_kprintf("normaal card clear faulted ! \n");
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






