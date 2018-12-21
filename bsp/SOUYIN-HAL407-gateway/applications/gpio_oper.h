

#ifndef _GPIO_OPER_
#define _GPIO_OPER_

#include <rtthread.h>



#define   LED     97
#define   BEEP    98

#define   SW1     57
#define   SW2     58
#define   SW3     59
#define   SW4     60
#define   SW5     61
#define   SW6     77
#define   SW7     78
#define   SW8     79
#define   SW9     1
#define   SW10    2
#define   SW11    3
#define   SW12    4
#define   SW13    5
#define   SW14    7
#define   SW15    23
#define   SW16    24

#define   ADD_SET1   39
#define   ADD_SET2   38
#define   ADD_SET3   37
#define   ADD_SET4   36
#define   ADD_SET5   35
#define   ADD_SET6   34

#define   D595    84
#define   EN595   85
#define   OE595   86
#define   CK595   87

typedef struct
{
	uint16_t  door_sta;
	uint16_t open_time;
}DOOR_REG;


void write_h595(uint16_t dat);
uint8_t get_nrf_addr(void);
void beep_on(rt_uint8_t count);

#endif


