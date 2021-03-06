
#include <stm32f4xx_hal.h>
#include <rtthread.h>
#include "lcd_oper.h"
#include "cmd_queue.h"
#include "gpio_oper.h"
#include "rfic_oper.h"
#include "rfic-manage.h"
#include "nrf_gateway.h"
//#include <stdlib.h>
//#include <stdio.h>
#include <string.h>
//#include "spi_flash_gd.h"
#include "pcf8563.h"


#define tag_value				0x4923

//static uint8_t update_en = 0;                          //更新标记
//volatile uint32_t  timer_tick_count = 0;               //定时器节拍

extern unsigned char lcd_update_en;

extern uint8_t  cmd_buffer[CMD_MAX_SIZE];              //LCD指令缓存

volatile uint8_t UI=MAIN_INDEX;       //界面标识符，禁止编译器优化该变量
static unsigned char lcd_update_time_count = 0;

sys_status_t    sys_status_lcd;
sys_config_t    sys_config_lcd;
Calendar_Def    sys_datetime;
card_t temp_card;

/***********************************************************************************************************************/
const uint16_t utex[3] = {84,312,540};
const uint16_t utey[2] = {230,372};

const uint8_t  key_card_str[] = {0xC3, 0xDC, 0xD4, 0xBF, 0xBF, 0xA8, 0xB9, 0xDC, 0xC0, 0xED,0};    //密钥卡管理
const uint8_t  cof_card_str[] = {0xC5, 0xE4, 0xD6, 0xC3, 0xBF, 0xA8, 0xB9, 0xDC, 0xC0, 0xED,0};    //配置卡
const uint8_t  pri_card_str[] = {0xCC, 0xD8, 0xC8, 0xA8, 0xBF, 0xA8, 0xB9, 0xDC, 0xC0, 0xED,0};    //特权卡
const uint8_t  nom_card_str[] = {0xC6, 0xD5, 0xCD, 0xA8, 0xBF, 0xA8, 0xB9, 0xDC, 0xC0, 0xED,0};    //普通卡  218，62 64点阵

static uint8_t test_sta_char[] = {0xD7, 0xB4, 0xCC, 0xAC, 0xA3, 0xBA,0};     //"状态"
static uint8_t test_add_char[] = {0xB5, 0xD8, 0xD6, 0xB7, 0xA3, 0xBA,0};     //"地址"

static uint8_t test_dor_open[] = {0xBF,0xAA,0};       //开
static uint8_t test_dor_close[] = {0xB9,0xD8,0};      //关

typedef struct
{
	uint8_t sqet;
	uint8_t addr;
	uint8_t id;
	uint8_t stat;
}test_btn;

test_btn t_button[5];

uint8_t cur_btn=0;

static void show_parment(uint8_t addr,uint8_t id)  
{
	uint8_t tmp[6],i;

	for(i=0;i<5;i++)
	{
		if(t_button[i].addr==addr && t_button[i].id==id) break;
	}
	tmp[0] = (addr/10)+'0'; tmp[1] = (addr%10)+'0'; tmp[2]=' '; 
	tmp[3]=(t_button[i].id/10)+'0'; tmp[4]=(t_button[i].id%10)+'0'; tmp[5]=0;
	
	if(t_button[i].stat)
	{
		show_string(TEST_UI,utex[i%3]+68,utey[i/3],1,6,(uint8_t*)test_dor_open);
	}
	else show_string(TEST_UI,utex[i%3]+68,utey[i/3],1,6,(uint8_t*)test_dor_close);
	show_string(TEST_UI,utex[i%3]+68,utey[i/3]+26,1,6,(uint8_t*)tmp);
	
}

void rev_parment(uint8_t src, uint8_t dor_id, uint8_t dor_s)
{
	uint8_t i;
	for(i=0;i<5;i++)
	{
		if(t_button[i].addr==src && t_button[i].id==dor_id)
		{
			t_button[i].stat = dor_s; goto exit;
		}
	}
	t_button[cur_btn].sqet=cur_btn+1;
	t_button[cur_btn].addr=src; 
	t_button[cur_btn].id=dor_id; 
	t_button[cur_btn].stat=dor_s; 
	if(cur_btn<5)cur_btn++;
	exit: show_parment(src,dor_id);
}

static void test_screen(void)
{
	uint8_t i;
	SetScreen(TEST_UI);
	SetFcolor(0xFFFF); //前景白色
	for(i=0;i<3;i++)
	{
		show_string(TEST_UI,utex[i],utey[0],0,6,test_sta_char);
		show_string(TEST_UI,utex[i],utey[1],0,6,test_sta_char);
		show_string(TEST_UI,utex[i],utey[0]+26,0,6,test_add_char);
		show_string(TEST_UI,utex[i],utey[1]+26,0,6,test_add_char);
	}
}

static void test_send_node(uint8_t dat)   
{
	#ifdef NRF24L01_USING_NODE
		uint16_t tmp;
		tmp = 0xFFFF^(1<<(addr-1));
		write_h595(tmp);
		rt_thread_delay(500);
		write_h595(0xFFFF);
	#else
		nrf_send_data(t_button[dat-1].addr,&t_button[dat-1].id);      //向节点发送命令
	#endif
}

/*************************************************************************************************************************/

/*! 
 *  \brief  读取RTC时间，注意返回的是BCD码
 *  \param year 年（BCD）
 *  \param month 月（BCD）
 *  \param week 星期（BCD）
 *  \param day 日（BCD）
 *  \param hour 时（BCD）
 *  \param minute 分（BCD）
 *  \param second 秒（BCD）
 */
static void NotifyReadRTC(unsigned char year,unsigned char month,unsigned char week,unsigned char day,unsigned char hour,unsigned char minute,unsigned char second)
{
//    if(datetime_is_updated==FALSE)
//    {
        sys_datetime.year = 2000;
        sys_datetime.year += ((year>>4)*10);
        sys_datetime.year += (year&0x0F);
        sys_datetime.month = ((month>>4)*10);
        sys_datetime.month += (month&0x0F);
        sys_datetime.mday = ((day>>4)*10);
        sys_datetime.mday += (day&0x0F);
        sys_datetime.hour = ((hour>>4)*10);
        sys_datetime.hour += (hour&0x0F);
        sys_datetime.min = ((minute>>4)*10);
        sys_datetime.min += (minute&0x0F); 
        sys_datetime.sec = ((second>>4)*10);
        sys_datetime.sec += (second&0x0F);
//        RTC_SetTime(&sys_datetime);
//    }
}


/*! 
 *  \brief  画面切换通知
 *  \details  当前画面改变时(或调用GetScreen)，执行此函数
 *  \param screen_id 当前画面ID
 */
static void NotifyScreen(unsigned short screen_id)
{
    lcd_update_en = 1;
}


/*! 
*  \brief  消息处理流程
*  \param msg 待处理消息
*  \param size 消息长度
*/
void ProcessMessage( PCTRL_MSG msg, uint16_t size )
{
    uint8_t cmd_type = msg->cmd_type;                                                  //指令类型
    uint8_t ctrl_msg = msg->ctrl_msg;                                                  //消息的类型
    uint8_t control_type = msg->control_type;                                          //控件类型
    uint16_t screen_id = PTR2U16(&msg->screen_id_high);                                //画面ID
    uint16_t control_id = PTR2U16(&msg->control_id_high);                              //控件ID
    uint32_t value = PTR2U32(msg->param);                                              //数值


    switch(cmd_type)
    {  
    case NOTIFY_TOUCH_PRESS:                                                        //触摸屏按下      0x01
    case NOTIFY_TOUCH_RELEASE:                                                      //触摸屏松开      0x03
        NotifyTouchXY(cmd_buffer[1],PTR2U16(cmd_buffer+2),PTR2U16(cmd_buffer+4)); 
        break;                                                                    
    case NOTIFY_WRITE_FLASH_OK:                                                     //写FLASH成功     0x0C 
        NotifyWriteFlash(1);                                                      
        break;                                                                    
    case NOTIFY_WRITE_FLASH_FAILD:                                                  //写FLASH失败     0x0D
        NotifyWriteFlash(0);                                                      
        break;                                                                    
    case NOTIFY_READ_FLASH_OK:                                                      //读取FLASH成功  0x0B
        NotifyReadFlash(1,cmd_buffer+2,size-6);                                     //去除帧头帧尾
        break;                                                                    
    case NOTIFY_READ_FLASH_FAILD:                                                   //读取FLASH失败   0x0F
        NotifyReadFlash(0,0,0);                                                   
        break;                                                                    
    case NOTIFY_READ_RTC:                                                           //读取RTC时间 0xF7
        NotifyReadRTC(cmd_buffer[2],cmd_buffer[3],cmd_buffer[4],cmd_buffer[5],cmd_buffer[6],cmd_buffer[7],cmd_buffer[8]);
        break;
    case NOTIFY_CONTROL:                 //0xB1
        {
            if(ctrl_msg==MSG_GET_CURRENT_SCREEN)                                    //画面ID变化通知   0X01
            {
                NotifyScreen(screen_id);                                            //画面切换调动的函数
            }
            else
            {
                switch(control_type)
                {
                case kCtrlButton:                                                   //按钮控件
                    NotifyButton(screen_id,control_id,msg->param[1]);                  
                    break;                                                             
                case kCtrlText:                                                     //文本控件
                    NotifyText(screen_id,control_id,msg->param);                       
                    break;                                                             
                case kCtrlProgress:                                                 //进度条控件
                    NotifyProgress(screen_id,control_id,value);                        
                    break;                                                             
                case kCtrlSlider:                                                   //滑动条控件
                    NotifySlider(screen_id,control_id,value);                          
                    break;                                                             
                case kCtrlMeter:                                                    //仪表控件
                    NotifyMeter(screen_id,control_id,value);                           
                    break;                                                             
                case kCtrlMenu:                                                     //菜单控件
                    NotifyMenu(screen_id,control_id,msg->param[0],msg->param[1]);      
                    break;                                                              
                case kCtrlSelector:                                                 //选择控件
                    NotifySelector(screen_id,control_id,msg->param[0]);                
                    break;                                                              
                case kCtrlRTC:                                                      //倒计时控件
                    NotifyTimer(screen_id,control_id);
                    break;
                default:
                    break;
                }
            } 
            break;  
        } 
    default:
        break;
    }
}
/*------------------------------------节点信息处理块--------------------------------*/

SARK_MSG  sark_box[MAX_SARK];
SARK_POINT sk_pt;

static void find_sark_cfg(void)     //查找待配置节点
{
	uint8_t i;
	for(i=0;i<MAX_SARK;i++)
	{
		if(sark_box[i].name[0]==0 && sark_box[i].node)  //查找待配置节点，
		{
			sk_pt.cofg = i+1; return;                   //找到
		}
		sk_pt.cofg = 0;                                 //遍历后都没有，将指针清空
	}
}

static void show_sark_list(void)
{
	uint8_t i;
	char str[16];

	sk_pt.item = 0;
	SetScreen(MAIN_INDEX);
	SetScreen(SARK_MANAGE);
	for(i=0;i<MAX_SARK;i++)
	{
		if(sark_box[i].node)
		{
			strncpy(str,sark_box[i].name,5);  //字符串拷贝
			if(str[0]==0) str[0] = ' ';
			if(str[1]==0) str[1] = ' ';
			if(str[2]==0) str[2] = ' ';
			if(str[3]==0) str[3] = ' ';
			str[4] = ' ';  //空格符
			str[5] = sark_box[i].node/10+'0';
			str[6] = sark_box[i].node%10+'0';
			str[7] = ' ';  //空格符
			str[8] = sark_box[i].num%10+'0'; 
			str[9] = ' ';  //空格符
			str[10] = sark_box[i].ov_time/10+'0';
			str[11] = sark_box[i].ov_time%10+'0';
			if(sk_pt.cofg==(i+1))
			{
				str[12] = ' '; str[13] = '<'; str[14] = '-'; str[15] = 0;
			}
			else str[12] = 0;
			show_string(SARK_MANAGE,574,288+sk_pt.item*27,1,6,(uint8_t*)str);
			sk_pt.item++;
		}
	}
}

void sark_msg_init(void)
{
	uint8_t i;
	sk_pt.del=0;
	sk_pt.cofg=0;
	sk_pt.item=0;
	sk_pt.lins=0;
	for(i=0;i<MAX_SARK;i++)
	{
		sark_box[i].name[0] = 0;
		sark_box[i].name[1] = 0;
		sark_box[i].name[2] = 0;
		sark_box[i].name[3] = 0;
		sark_box[i].name[4] = 0;
		sark_box[i].node = 0;
		sark_box[i].num  = 0;
		sark_box[i].ov_time = 0;
	}
}

void sark_join_cfg(uint8_t node_ad)
{
	uint8_t i,data[8];
	
	for(i=0;i<MAX_SARK;i++)
	{
		if(sark_box[i].node==node_ad && sark_box[i].name[0]!=0) return; //查询是否已在线且已匹配别名
	}
	
	lcd_update_en = 1;
	
//	sark_param_read(node_ad,data);   //查询flash中有无该节点信息
	
	if(data[0] == node_ad)   //flash有该节点则向系统环境注入参数（证明已设置过该节点）
	{
		for(i=0;i<MAX_SARK;i++)
		{
			if(sark_box[i].node==0)  //节点为0
			{
				sark_box[i].node = data[0];
				sark_box[i].num = data[1];
				sark_box[i].ov_time = data[2];
				sk_pt.lins++;
				strncpy(sark_box[i].name,(char*)&data[3],5);
				return;
			}
		}
	}
	else           //flash中没有该节点信息
	{
		for(i=0;i<MAX_SARK;i++)     //查询节点有没有注册过
		{
			if(sark_box[i].node==node_ad) return;     //如果已经注册，则不重复
		}
		for(i=0;i<MAX_SARK;i++)     //没有注册 查找空闲
		{
			if(sark_box[i].node==0)
			{
				sark_box[i].node=node_ad;   //填写
				sk_pt.lins++;  break;
			}
		}
		if(sk_pt.cofg==0)
		{
			find_sark_cfg();	//设置配置指针
		}
	}
}

void sark_discon_remov(uint8_t addr)
{
	uint8_t i;
	for(i=0;i<MAX_SARK;i++)
	{
		if(sark_box[i].node==addr) 
		{
			sark_box[i].node = 0;
			sark_box[i].name[0] = 0;
			sk_pt.lins--;
			lcd_update_en = 1;
		}
	}
	lcd_update_en = 1;             //更新LCD
	find_sark_cfg();               //设置配置指针
}

static void sark_del_appoint(unsigned char *str)
{
	uint8_t i;
	for(i=0;i<MAX_SARK;i++)
	{
		if(strcmp((char*)str,sark_box[i].name)==0)
		{
			sk_pt.del = i;
			sk_pt.lins--;  break;
		}
	}
}

static void sark_del_remov(void)
{
//	sark_param_clear(sark_box[sk_pt.del].node);
	//sark_box[sk_pt.del].node = 0;
	sark_box[sk_pt.del].name[0]=0;
	sark_box[sk_pt.del].name[1]=0;
	sark_box[sk_pt.del].name[2]=0;
	sark_box[sk_pt.del].name[3]=0;
	sark_box[sk_pt.del].name[4]=0;
	sark_box[sk_pt.del].num = 0;
	sark_box[sk_pt.del].ov_time = 0;
	clesr_text(SARK_MANAGE,64);    //清除删除文字控件信息
	lcd_update_en = 1;             //更新LCD
	find_sark_cfg();               //设置配置指针
}

static uint8_t sark_save_param(sark_msg_t tag)
{
	if(tag->node>30 && tag->node<64)
	{
		if(tag->num!=0 && tag->ov_time!=0 && tag->name[0] != 0)
		{ 
//			sark_param_store(tag); 
			clesr_text(SARK_MANAGE,62);
			clesr_text(SARK_MANAGE,65);
			clesr_text(SARK_MANAGE,66);
			lcd_update_en = 1;             //更新LCD
			find_sark_cfg();               //设置配置指针
			return 1; 
		}
	}
	return 0;
}

/*------------------------------ end -- 节点信息处理块  ----------------------------*/

static void card_manager_rst(void)
{
	switch(UI)
	{
		case SYS_KEY_CRCF:  key_card_init();       break;
		case MANA_CARD_SET: manager_card_init();   break;
		case PRIV_CARD_SET: privilege_card_init(); break;
		case NORM_CARD_SET: normaal_card_init();   break;
	}
}

static void card_manager_make(void)
{
	switch(UI)
	{
		case SYS_KEY_CRCF:   key_card_make();       break;
		case MANA_CARD_SET:  manager_card_make();   break;
		case PRIV_CARD_SET:  privilege_card_make(); break;
		case NORM_CARD_SET:  normaal_card_make();   break;
	}
}

static void card_manager_clear(void)
{
	switch(UI)
	{
		case SYS_KEY_CRCF:  key_card_clear();       break;
		case MANA_CARD_SET: manager_card_clear();   break;
		case PRIV_CARD_SET: privilege_card_clear(); break;
		case NORM_CARD_SET: normaal_card_clear();   break;
	}
}


/*! 
 *  \brief  按钮控件通知
 *  \details  当按钮状态改变(或调用GetControlValue)时，执行此函数
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 *  \param state 按钮状态：0弹起，1按下
 */
static void NotifyButton(unsigned short screen_id, unsigned short control_id, unsigned char  state)
{
    switch(screen_id)
    {
		case MAIN_INDEX   :   if(control_id == 10) { UI=TEST_UI; test_screen(); }
							  else if(control_id == 11)	{ UI=SARK_MANAGE; show_sark_list(); }
							  else if(control_id == 13) { UI=SYS_TIME_SETUP; SetScreen(SYS_TIME_SETUP); }
							  break;             
        case SYS_CFG_INDEX:
				switch(control_id)
				{
					case 101: UI=MANA_CARD_SET; SetScreen(CARD_MANAG);   
							  show_string(CARD_MANAG,258,62,1,9,(uint8_t*)cof_card_str); break;     //进入配置卡
					case 102: UI=PRIV_CARD_SET; SetScreen(CARD_MANAG);  
							  show_string(CARD_MANAG,258,62,1,9,(uint8_t*)pri_card_str); break;     //进入特权卡设置
					case 103: UI=NORM_CARD_SET; SetScreen(CARD_MANAG);  
							  show_string(CARD_MANAG,258,62,1,9,(uint8_t*)nom_card_str); break;     //进入普通卡
					case 104: UI=SYS_TIME_SETUP; SetScreen(SYS_TIME_SETUP); break;     //进入系统时间设置
					case 105: UI=SARK_MANAGE; show_sark_list();    break;     //柜门管理
					case 106: UI=MAIN_INDEX; SetScreen(MAIN_INDEX);
				}
				break;
        case SYS_TIME_SETUP: 
				if(control_id==208)
				{
					sys_datetime.year = sys_status_lcd.setting_datetime.year;
					if(sys_datetime.year==0) break;
					sys_datetime.month = sys_status_lcd.setting_datetime.month;
					if(sys_datetime.month==0) break;
					sys_datetime.mday = sys_status_lcd.setting_datetime.mday;
					if(sys_datetime.mday==0) break;
					sys_datetime.hour = sys_status_lcd.setting_datetime.hour;
					if(sys_datetime.hour==0) break;
					sys_datetime.min = sys_status_lcd.setting_datetime.min;
					if(sys_datetime.min==0) break;
					sys_datetime.sec = sys_status_lcd.setting_datetime.sec;
					if(rtc_set_time(&sys_datetime)==1) rt_kprintf("set time 0k !\n \n");
					if(rtc_get_time(&sys_datetime)) rt_kprintf("get time ok !\n \n");
//					rt_kprintf("setuptime：%04dyy%02dmm%02dd %02dhour%02dmin%02dsec\r\n",sys_datetime.year,sys_datetime.month,sys_datetime.mday,sys_datetime.hour,sys_datetime.min,sys_datetime.sec);                    
					SetRtc(sys_datetime.year,sys_datetime.month,sys_datetime.mday,sys_datetime.wday,sys_datetime.hour,sys_datetime.min,sys_datetime.sec);                    
					sys_status_lcd.sys_state = sys_status_lcd.sys_last_state;
					UI=MAIN_INDEX; SetScreen(MAIN_INDEX);
				}
				else if(control_id==207)
				{
					sys_status_lcd.sys_state = sys_status_lcd.sys_last_state;
					UI=SYS_CFG_INDEX; SetScreen(SYS_CFG_INDEX);
				}
				break;
        case CARD_MANAG:  
				switch(control_id)
				{
					case 31:  card_manager_rst();  break;          //初始化卡，函数内区分卡类型：钥匙卡，配置卡等
					case 32:  card_manager_make();  break;         //制作配置卡
					case 33:  card_manager_clear(); break;         //重置制作卡
					case 34:  UI=SYS_CFG_INDEX; SetScreen(SYS_CFG_INDEX); break;
					case 35:  complete_card_work();   break;	
				}
				break;
        case SARK_MANAGE:
				switch(control_id)
				{
					case 61:  
						#ifdef NRF24L01_USING_NODE	
							test_send_node(5);
						#else
							nrf_assign_addr(0);
							rt_kprintf("asign node address...\n");
						#endif 
						break;
					case 63:  sark_del_remov();	  break;
					case 67:  UI=SYS_CFG_INDEX; SetScreen(SYS_CFG_INDEX); break;
					case 68:  sark_save_param((sark_msg_t)&sark_box[sk_pt.cofg-1]);  break;
				}
				break;
		case DOOR_OPEN_MANA:  
				switch(control_id)
				{
					case 70:
					case 71:
					case 72:
					case 73:
					case 74:
					case 75:
					case 76:
					case 77:
					case 78:
					case 79:  UI=MAIN_INDEX; SetScreen(MAIN_INDEX); break;
				}
				break;
		case TEST_UI:
				switch(control_id)
				{
					case 81:  test_send_node(1);  break;
					case 82:  test_send_node(2);  break;
					case 83:  test_send_node(3);  break;
					case 84:  test_send_node(4);  break;
					case 85:  test_send_node(5);  break;
					case 86:  UI=MAIN_INDEX; SetScreen(MAIN_INDEX);  break;
				}
				break;
    }        
}


//字符串转整数
static long StringToInt32(unsigned char *str)
{
	long v = 0;
	sscanf((char *)str,"%ld",&v);
	return v;
}

/*! 
 *  \brief  文本控件通知
 *  \details  当文本通过键盘更新(或调用GetControlValue)时，执行此函数
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 *  \param str 文本控件内容
 */
static void NotifyText(unsigned short screen_id, unsigned short control_id, unsigned char *str)
{
	switch(screen_id)
    {
        case SYS_CFG_INDEX:   break;
        case SYS_TIME_SETUP:                          //接收LCD传来的参数并缓存
				 switch(control_id)
				 {
					 case 201: sys_status_lcd.setting_datetime.year = StringToInt32(str);  break;
					 case 202: sys_status_lcd.setting_datetime.month = StringToInt32(str); break;
					 case 203: sys_status_lcd.setting_datetime.mday = StringToInt32(str);  break;
					 case 204: sys_status_lcd.setting_datetime.hour = StringToInt32(str);  break;
					 case 205: sys_status_lcd.setting_datetime.min = StringToInt32(str);   break;
					 case 206: sys_status_lcd.setting_datetime.sec = StringToInt32(str);   break;
				 }
				 break;
        case MANA_CARD_SET:       break;
        case PRIV_CARD_SET:	      break;
		case NORM_CARD_SET:       break;
		case SARK_MANAGE: 
				switch(control_id)
				{
					case 62:  strncpy(sark_box[sk_pt.cofg-1].name,(char*)str,5); break; //保存柜名(将收到的字符串填写进银柜的名称数组)
					case 64:  sark_del_appoint(str);  break;
					case 65:  sark_box[sk_pt.cofg-1].num =  StringToInt32(str); break;  //柜门总数
					case 66:  sark_box[sk_pt.cofg-1].ov_time = StringToInt32(str); break;  //柜门超时时间
				}
				lcd_update_en = 1;   //更新LCD
				break;
		case DOOR_OPEN_MANA:      break;
    } 
}


/*! 
 *  \brief  写用户FLASH状态返回
 *  \param status 0失败，1成功
 */
static void NotifyWriteFlash(unsigned char status)
{
}

/*! 
 *  \brief  触摸坐标事件响应
 *  \param press 1按下触摸屏，3松开触摸屏
 *  \param x x坐标
 *  \param y y坐标
 */
static void NotifyTouchXY(unsigned char press,unsigned short x,unsigned short y)
{
	//TODO: 添加用户代码
}

static void NotifyProgress(unsigned short screen_id, unsigned short control_id, unsigned long value)
{
}


/*! 
 *  \brief  滑动条控件通知
 *  \details  当滑动条改变(或调用GetControlValue)时，执行此函数
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 *  \param value 值
 */
static void NotifySlider(unsigned short screen_id, unsigned short control_id, unsigned long value)
{
}


/*! 
 *  \brief  读取用户FLASH状态返回
 *  \param status 0失败，1成功
 *  \param _data 返回数据
 *  \param length 数据长度
 */
static void NotifyReadFlash(unsigned char status,unsigned char *_data,unsigned short length)
{
}

/*! 
 *  \brief  仪表控件通知
 *  \details  调用GetControlValue时，执行此函数
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 *  \param value 值
 */
static void NotifyMeter(unsigned short screen_id, unsigned short control_id, unsigned long value)
{
}

/*! 
 *  \brief  菜单控件通知
 *  \details  当菜单项按下或松开时，执行此函数
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 *  \param item 菜单项索引
 *  \param state 按钮状态：0松开，1按下
 */
static void NotifyMenu(unsigned short screen_id, unsigned short control_id, unsigned char  item, unsigned char  state)
{
}

/*! 
 *  \brief  选择控件通知
 *  \details  当选择控件变化时，执行此函数
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 *  \param item 当前选项
 */
static void NotifySelector(unsigned short screen_id, unsigned short control_id, unsigned char  item)
{
}

/*! 
 *  \brief  定时器超时通知处理
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 */
static void NotifyTimer(unsigned short screen_id, unsigned short control_id)
{
}
/*! 
*  \brief  更新数据
*/ 

void UpdateUI(void)
{
	switch(UI)
	{
		case SARK_MANAGE: show_sark_list(); break;
	}
	
//     if(sys_status_lcd.config_card_count<CONFIG_CARD_MAX_COUNT)
//    {
//        sys_status_lcd.sys_last_state = sys_status_lcd.sys_state = make_config_card_state;
////        SetTextValue(SYS_INIT_INDEX,255,(unsigned char*)"提示：请将IC卡放置在感应区不要移动！");
////        SetScreen(SYS_INIT_INDEX);
//    }
//    else
//    {
//		if(sys_status_lcd.sys_state!=user_card_open_door_state && sys_status_lcd.sys_state!=power_card_open_door_state)
//		{
//			sys_status_lcd.sys_last_state = sys_status_lcd.sys_state = open_close_door_state;
//		}
//        SetScreen(DOOR_OPEN_MANA);
//    }                          
}

static void lcd_updata_entry(void* param)
{
	while(1)
	{
		if(lcd_update_en&&(lcd_update_time_count==100))
		{        
			UpdateUI();
			lcd_update_time_count = 0;
			lcd_update_en = 0;
		}
		if(lcd_update_time_count<100) 
		{lcd_update_time_count++;}    //该变量加至100时允许更新屏幕
		rt_thread_delay(10);
	}
}

static int lcd_updata(void)
{
	rt_thread_t lcd_up = rt_thread_create("lcd_up",lcd_updata_entry,
										   RT_NULL,1024,15,10);
	if(lcd_up!=RT_NULL)
		rt_thread_startup(lcd_up);
	return 0;
}
INIT_APP_EXPORT(lcd_updata);






