#include <rtthread.h>
#include <uart_lcd_process.h>

#define UART_LCD_PROCESS_DEBUG

#define DBG_ENABLE
#define DBG_COLOR
#define DBG_SECTION_NAME		"uart.lcd.process"
#ifdef UART_LCD_PROCESS_DEBUG
#define DBG_LEVEL				DBG_LOG
#else
#define DBG_LEVEL				DBG_INFO
#endif
#include <rtdbg.h>

static notify_handle_t notify_button = RT_NULL;
static notify_handle_t notify_text = RT_NULL;
static notify_handle_t notify_screen = RT_NULL;

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

}


/*! 
 *  \brief  画面切换通知
 *  \details  当前画面改变时(或调用GetScreen)，执行此函数
 *  \param screen_id 当前画面ID
 */
static void NotifyScreen(unsigned short screen_id)
{
    if(notify_screen)
	{
		notify_screen(screen_id, 0, 0);
	}
}

/*! 
 *  \brief  按钮控件通知
 *  \details  当按钮状态改变(或调用GetControlValue)时，执行此函数
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 *  \param state 按钮状态：0弹起，1按下
 */
static void NotifyButton(unsigned short screen_id, unsigned short control_id, unsigned char state)
{
	if(notify_button)
	{
		notify_button(screen_id, control_id, (void *)(unsigned long)state);
	}
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
	if(notify_text)
	{
		notify_text(screen_id, control_id, (void *)str);
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
*  \brief  消息处理流程
*  \param msg 待处理消息
*  \param size 消息长度
*/
void ProcessMessage(uint8_t *msg_buff, uint16_t size)
{
	PCTRL_MSG msg = (PCTRL_MSG)msg_buff;
    uint8_t cmd_type = msg->cmd_type;                                                  //指令类型
    uint8_t ctrl_msg = msg->ctrl_msg;                                                  //消息的类型
    uint8_t control_type = msg->control_type;                                          //控件类型
    uint16_t screen_id = PTR2U16(&msg->screen_id_high);                                //画面ID
    uint16_t control_id = PTR2U16(&msg->control_id_high);                              //控件ID
    uint32_t value = PTR2U32(msg->param);
	uint8_t* cmd_buffer = (uint8_t*)msg;

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

void uart_lcd_set_button_notify_hook(notify_handle_t button_handle)
{
	notify_button = button_handle;
}

void uart_lcd_set_text_notify_hook(notify_handle_t text_handle)
{
	notify_text = text_handle;
}

void uart_lcd_set_screen_notify_hook(notify_handle_t screen_handle)
{
	notify_screen = screen_handle;
}
