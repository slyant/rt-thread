#include <rtthread.h>
#include "rtdevice.h"
#include "rfic_oper.h"
#include "ic_card_protocol.h"
#include <rfic-manage.h>
#include <hmi_driver.h>
#include <uart_lcd_process.h>

#define  RFID_SIG_LOG  rt_kprintf("[RFIC_OPER]:");rt_kprintf

rt_uint8_t ic_card_read_buff[300];

void rfid_thread_entry(void* param)
{
	rt_uint8_t  card_typ, cof_num[2];
	
	while(1)
	{
	/***************************************************************************************************************/
		card_typ = rfid_scan_handle();
		switch(card_typ)
		{
			case ROOT_CARD:
				lcd_change_screen(KEY_CARD_COF);
				break;
			case SYS_COF_CARD:  
				UI = SYS_COF_CARD;
				SetScreen(CARD_MANAG); 
				show_string(CARD_MANAG,258,62,1,9,(uint8_t*)cof_card_str); 
				cof_num[0] = check_cof_card_cun(); 
				cof_num[1] = 0;
				if(cof_num[0]<2) //如果配置卡不符合要求
				{
					show_string(CARD_MANAG,20,440,1,6,(uint8_t*)mcardwarning);
					show_string(CARD_MANAG,20,410,1,6,(uint8_t*)mcardnum);
					show_string(CARD_MANAG,160,410,1,6,(uint8_t*)cof_num);
				}
				break;
			case MANAG_CARD:
				break;
			case NORMNA_CARD:
				break;
			case SPACE_CARD:
				break;
			case OTHER_CARD:
				break;
			default:
				break;
		}
		
		rt_thread_delay(100);
	/***************************************************************************************************************/
	}
}

int rfid_thread_startup(void)
{
	rt_thread_t
	rftid = rt_thread_create("rfic_tid",
							 rfid_thread_entry,
							 RT_NULL,40*1024,10,20);
	if(rftid != RT_NULL ) 
		rt_thread_startup(rftid);
	return 0;
}
