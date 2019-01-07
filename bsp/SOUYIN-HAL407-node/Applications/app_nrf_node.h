#ifndef __APP_NRF_NODE_H__

#include <rtthread.h>

#define CMD_SESSION_KEY     0   //会话密钥
#define CMD_DOOR_OPEN       1   //开柜门
#define CMD_GROUP_OPEN      2   //开整组柜门
#define CMD_DOOR_STA        3   //柜门状态
#define CMD_NRF_RESET       4   //NRF复位

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

struct nrf_net_option
{
    rt_uint8_t addr;
    rt_uint32_t pwd;
    rt_uint8_t md5[16];
};
typedef struct nrf_net_option *nrf_net_option_t;

void nrf_option_clear(void);
void nrf_send_door_sta(rt_uint16_t sta);

#endif
