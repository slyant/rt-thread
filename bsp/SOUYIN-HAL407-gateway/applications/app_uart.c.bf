
#include <rthw.h>
#include <rtthread.h>
#include "app_uart.h"
#include "cmd_queue.h"
#include "hmi_driver.h"
#include "lcd_oper.h"








extern void SendNU8(unsigned char *pData,unsigned short nDataLen);




/***********************************************************************************************************/
extern uint8_t sys_key_a[6];
extern uint8_t sys_key_b[6];    //系统密钥对，由密钥函数生成，并存入加密文件系统，这里暂时定义便于调试
extern volatile uint8_t UI;
extern const uint8_t  key_card_str[];
extern const uint8_t  cof_card_str[];
/***********************************************************************************************************/





