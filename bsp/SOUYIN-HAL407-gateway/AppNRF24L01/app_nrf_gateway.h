#ifndef __APP_NRF_GATEWAY_H__

#include <rtthread.h>

#define CMD_SESSION_KEY     0   //会话密钥
#define CMD_DOOR_OPEN       1   //开柜门
#define CMD_GROUP_OPEN      2   //开整组柜门
#define CMD_DOOR_STA        3   //柜门状态

union args
{
    rt_uint8_t door_index;
    rt_uint16_t door_sta;
    rt_uint8_t key[8];    
};

struct payload_data
{
    rt_uint8_t cmd;
    union args args;
    rt_uint8_t temp[8];
};
typedef struct payload_data *payload_data_t;

typedef void (*nrf_door_update_hook_t)(rt_uint8_t group_index, rt_uint16_t sta);

void nrfreset(void);
void nrf_send_net_pwd(void);
void nrf_send_door_open(rt_uint8_t group_index, rt_uint8_t door_index);
void nrf_send_group_open(rt_uint8_t group_index);
void nrf_set_remote_door_update_hook(nrf_door_update_hook_t hook);
#endif
