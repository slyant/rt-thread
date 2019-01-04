#ifndef __APP_DOOR_H__
#define __APP_DOOR_H__

#include <rtthread.h>

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

#define   D595    84
#define   EN595   85
#define   OE595   86
#define   CK595   87

#define   ADD_SET1   39
#define   ADD_SET2   38
#define   ADD_SET3   37
#define   ADD_SET4   36
#define   ADD_SET5   35
#define   ADD_SET6   34

typedef void (*door_update_hook_t)(rt_uint16_t);

rt_uint8_t door_get_group_addr(void);
void door_update_set_hook(door_update_hook_t hook);
void door_open(rt_uint8_t door_index);
void door_group_open(rt_uint8_t door_count);

#endif
