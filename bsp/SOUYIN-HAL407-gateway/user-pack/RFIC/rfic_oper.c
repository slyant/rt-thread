
#include <rtthread.h>
#include "rtdevice.h"
#include "rfic_oper.h"
#include "ic_card_protocol.h"
#include "rfic-manage.h"
#include "lcd_oper.h"
#include "cmd_queue.h"


#define  RFID_SIG_LOG  rt_kprintf("[RFID_SIG]:");rt_kprintf


unsigned char ic_card_read_buff[300];

extern volatile uint8_t UI;

//sys_status_t    sys_status;
//sys_config_t    sys_config;


//static unsigned char write_log_handle(unsigned char* card_id,unsigned char card_number,unsigned char card_type,unsigned char door_number,char* desc)
//{
//    unsigned char result = 0;
//    char  tem_tim[300];                 // malloc(300);
//	char * log  =  tem_tim;
////    update_sys_datetime();
//    if(card_id!=NULL)
//    {
//        rt_kprintf(log,"%d-%02d-%02d %02d:%02d:%02d,%02X%02X%02X%02X,%d,%s,[%d-%d-%d],%s\r\n",   //sys_datetime.year,sys_datetime.month,sys_datetime.mday,sys_datetime.hour,sys_datetime.min,sys_datetime.sec,
//        card_id[0],card_id[1],card_id[2],card_id[3],
//        ((card_type_t)card_type==power_card||(card_type_t)card_type==user_card)?(card_number+1):0,
//        ((card_type_t)card_type==power_card?"特权卡":((card_type_t)card_type==user_card?"普通卡":"无")),
//        sys_config.device_addr0,
//        sys_config.device_addr1,
//        door_number+1,
//        desc);
//    }
//    else
//    {
//        rt_kprintf(log,"%d-%02d-%02d %02d:%02d:%02d,%02X%02X%02X%02X,%d,%s,[%d-%d-%d],%s\r\n",  //sys_datetime.year,sys_datetime.month,sys_datetime.mday,sys_datetime.hour,sys_datetime.min,sys_datetime.sec,
//        0,0,0,0,
//        ((card_type_t)card_type==power_card||(card_type_t)card_type==user_card)?(card_number+1):0,
//        ((card_type_t)card_type==power_card?"特权卡":((card_type_t)card_type==user_card?"普通卡":"无")),
//        sys_config.device_addr0,
//        sys_config.device_addr1,
//        door_number+1,
//        desc);
//    }
//    if(sys_status.u_disk_ready==TRUE)
//	{
//        result = write_log_to_udisk(log);
//    }
//    else
//    {
////        printf("%s",log);
//    }
//    free(log);
//    return result;
//}


extern const uint8_t  cof_card_str[];
extern const uint8_t  mcardwarning[];
extern const uint8_t  mcardnum[];

void rfid_thread_entry(void* param)
{
	uint8_t  card_typ, cof_num[2];
	
	while(1)
	{
	/***************************************************************************************************************/
		card_typ = rfid_scan_handle();
		switch(card_typ)
		{
			case ROOT_CARD:    
				UI = KEY_CARD_COF; 
				SetScreen(KEY_CARD_COF); 
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

static int rfid_handle(void)
{
	rt_thread_t
	rftid = rt_thread_create("rfic_tid",
							 rfid_thread_entry,
							 RT_NULL,40*1024,10,20);
	if(rftid != RT_NULL ) 
		rt_thread_startup(rftid);
	return 0;
}
//INIT_APP_EXPORT(rfid_handle);


