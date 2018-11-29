
#include <rtthread.h>
#include "rtdevice.h"
#include "rfic_oper.h"
#include "ic_card_protocol.h"


#define  RFID_SIG_LOG  rt_kprintf("[RFID_SIG]:");rt_kprintf


unsigned char ic_card_read_buff[300];


sys_status_t    sys_status;
sys_config_t    sys_config;



static unsigned char card_id_check(unsigned char* card_s,unsigned char* card_o)
{
    return (card_s[0]==card_o[0] && card_s[1]==card_o[1] && card_s[2]==card_o[2] && card_s[3]==card_o[3]);
}

static unsigned char write_log_handle(unsigned char* card_id,unsigned char card_number,unsigned char card_type,unsigned char door_number,char* desc)
{
    unsigned char result = 0;
    char  tem_tim[300];                 // malloc(300);
	char * log  =  tem_tim;
//    update_sys_datetime();
    if(card_id!=NULL)
    {
        rt_kprintf(log,"%d-%02d-%02d %02d:%02d:%02d,%02X%02X%02X%02X,%d,%s,[%d-%d-%d],%s\r\n",   //sys_datetime.year,sys_datetime.month,sys_datetime.mday,sys_datetime.hour,sys_datetime.min,sys_datetime.sec,
        card_id[0],card_id[1],card_id[2],card_id[3],
        ((card_type_t)card_type==power_card||(card_type_t)card_type==user_card)?(card_number+1):0,
        ((card_type_t)card_type==power_card?"��Ȩ��":((card_type_t)card_type==user_card?"��ͨ��":"��")),
        sys_config.device_addr0,
        sys_config.device_addr1,
        door_number+1,
        desc);
    }
    else
    {
        rt_kprintf(log,"%d-%02d-%02d %02d:%02d:%02d,%02X%02X%02X%02X,%d,%s,[%d-%d-%d],%s\r\n",  //sys_datetime.year,sys_datetime.month,sys_datetime.mday,sys_datetime.hour,sys_datetime.min,sys_datetime.sec,
        0,0,0,0,
        ((card_type_t)card_type==power_card||(card_type_t)card_type==user_card)?(card_number+1):0,
        ((card_type_t)card_type==power_card?"��Ȩ��":((card_type_t)card_type==user_card?"��ͨ��":"��")),
        sys_config.device_addr0,
        sys_config.device_addr1,
        door_number+1,
        desc);
    }
//    if(sys_status.u_disk_ready==TRUE)
//	{
//        result = write_log_to_udisk(log);
//    }
//    else
//    {
////        printf("%s",log);
//    }
//    free(log);
    return result;
}


void rfid_thread_entry(void* param)
{
	unsigned char i;
    unsigned short length;
    unsigned char card_id[4];
    unsigned char card_type;
    unsigned char card_number;
	
	while(1)
	{
	/***************************************************************************************************************/
		if(rfid_scan_handle(card_id,ic_card_read_buff,&length)==1)
		{
			if(length>=2)
			{
				card_type = ic_card_read_buff[0];
				card_number = ic_card_read_buff[1];
				switch(card_type)
				{
					case config_card:
	//                    stop_operate_timeout_timer();
						for(i=0;i<sys_status.config_card_count;i++)
						{
							if(sys_config.cards[i].card_number==card_number && card_id_check(sys_config.cards[i].card_id,card_id))
							{
								sys_status.sys_last_state = sys_status.sys_state = config_card_setting_state;
								RFID_SIG_LOG("Card setting state ! \n");
	//                            SetScreen(SYS_CFG_INDEX);    //������
	//                            SetBuzzer(100);              //����������ʱ��
	//                            fmq_bell_L();                //����������
								break;
							}
						}
						break;
					case power_card:                    
						for(i=0;i<sys_status.power_card_count;i++)
						{
							if(sys_config.cards[CONFIG_CARD_MAX_COUNT+i].card_number==card_number && card_id_check(sys_config.cards[CONFIG_CARD_MAX_COUNT+i].card_id,card_id))
							{
								unsigned char open_en = 0;
								char* str;
	//							if(time_is_between_times(&sys_datetime,sys_config.cards[CONFIG_CARD_MAX_COUNT+i].open_door_start_h,
	//                            sys_config.cards[CONFIG_CARD_MAX_COUNT+i].open_door_start_m,
	//                            sys_config.cards[CONFIG_CARD_MAX_COUNT+i].open_door_end_h,
	//                            sys_config.cards[CONFIG_CARD_MAX_COUNT+i].open_door_end_m)==1)
	//                            {//�ڿ���ʱ�����
	//                               open_en = 1;
	//                            }
	//                            else
	//                            {//���ڿ���ʱ�����
	//                                str = "����:���ںϷ���ʱ����ڿ��ţ�";
	//                                open_en = 0;
	//                            }							
								if(open_en==1)
								{
									sys_status.sys_last_state = sys_status.sys_state = power_card_open_door_state;
									sys_status.power_card_number = card_number;
	//								update_door_button_status();
	//								SetScreen(OPEN_CARD_INDEX);
	//                                sys_status.last_screen_index = OPEN_CARD_INDEX;
	//								SetBuzzer(100);
	//								fmq_bell_L(); 
									write_log_handle(card_id,card_number,card_type,0,"ˢ��ȨIC��");
								}
								else
								{
	//                                SetTextValue(MESSAGE_INDEX,100,(unsigned char*)str);
	//                                SetScreen(MESSAGE_INDEX);
	//								sys_status.last_screen_index = MAIN_INDEX;
	//								start_message_delay_timer();
	//                                fmq_bell(2);
									write_log_handle(card_id,card_number,card_type,0,str);
								}                                                      
								break;
							}
						}                    
						break;
					case user_card:
						for(i=0;i<sys_status.user_card_count;i++)
						{
							if(sys_config.cards[CONFIG_CARD_MAX_COUNT+POWER_CARD_MAX_COUNT+i].card_number==card_number && card_id_check(sys_config.cards[CONFIG_CARD_MAX_COUNT+POWER_CARD_MAX_COUNT+i].card_id,card_id))
							{
								unsigned char open_en = 0;
								char* str;
	//                            if(time_is_between_times(&sys_datetime,sys_config.cards[CONFIG_CARD_MAX_COUNT+POWER_CARD_MAX_COUNT+i].open_door_start_h,
	//                            sys_config.cards[CONFIG_CARD_MAX_COUNT+POWER_CARD_MAX_COUNT+i].open_door_start_m,
	//                            sys_config.cards[CONFIG_CARD_MAX_COUNT+POWER_CARD_MAX_COUNT+i].open_door_end_h,
	//                            sys_config.cards[CONFIG_CARD_MAX_COUNT+POWER_CARD_MAX_COUNT+i].open_door_end_m)==TRUE)
	//                            {//�ڿ���ʱ�����
	//                                if(card_use_data_list.card_use_data[i].open_door_year==sys_datetime.year
	//                                && card_use_data_list.card_use_data[i].open_door_month==sys_datetime.month
	//                                && card_use_data_list.card_use_data[i].open_door_day==sys_datetime.mday)
	//                                {//�����ѿ���
	//                                    if(sys_config.cards[CONFIG_CARD_MAX_COUNT+POWER_CARD_MAX_COUNT+i].open_count_per_day==0xff || 
	//										card_use_data_list.card_use_data[i].open_count_current<sys_config.cards[CONFIG_CARD_MAX_COUNT+POWER_CARD_MAX_COUNT+i].open_count_per_day)
	//                                    {//���Ŵ���С���趨ֵ
	//                                        card_use_data_list.card_use_data[i].open_count_current++;
	//                                        save_card_use_data_list();
	//										open_en = TRUE;
	//                                    }
	//                                    else
	//                                    {
	//                                        str = "����:���Ŵ����Ѵ����ޣ�";
	//                                        open_en = FALSE;
	//                                    }
	//                                }
	//                                else
	//                                {//����û����
	//                                    card_use_data_list.card_use_data[i].open_count_current = 1;
	//                                    card_use_data_list.card_use_data[i].open_door_year = sys_datetime.year;
	//                                    card_use_data_list.card_use_data[i].open_door_month = sys_datetime.month;
	//                                    card_use_data_list.card_use_data[i].open_door_day = sys_datetime.mday;                                                                    
	//                                    save_card_use_data_list();
	//									open_en = TRUE;
	//                                }
	//                            }
	//                            else
	//                            {//���ڿ���ʱ�����
	//                                str = "����:���ںϷ���ʱ����ڿ��ţ�";
	//                                open_en = FALSE;
	//                            }
								if(open_en==1)
								{
									sys_status.sys_last_state = sys_status.sys_state = user_card_open_door_state;
	//                                update_door_button_status();
	//                                SetScreen(OPEN_CARD_INDEX);
	//                                sys_status.last_screen_index = OPEN_CARD_INDEX;
	//                                SetBuzzer(100);
	//                                fmq_bell_L();
	//                                execute_open_door(card_number);
									write_log_handle(card_id,card_number,card_type,card_number,"ˢ��ͨIC������");
								}
								else
								{
	//                                SetTextValue(MESSAGE_INDEX,100,(unsigned char*)str);
	//                                SetScreen(MESSAGE_INDEX);
	//								sys_status.last_screen_index = MAIN_INDEX;
	//								start_message_delay_timer();
	//                                fmq_bell(2);
									write_log_handle(card_id,card_number,card_type,card_number,str);
								}
								break;
							}
						}                    
						break;
				}            
			}
		}
		rt_thread_delay(20);
	/***************************************************************************************************************/
	}
}

static int rfid_handle(void)
{
	rt_thread_t
	rftid = rt_thread_create("rfic_tid",
							 rfid_thread_entry,
							 RT_NULL,1024,10,20);
	if(rftid != RT_NULL ) 
		rt_thread_startup(rftid);
	return 0;
}
INIT_APP_EXPORT(rfid_handle);



