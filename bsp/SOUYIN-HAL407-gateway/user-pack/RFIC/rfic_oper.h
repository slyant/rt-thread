
#ifndef __RFIC_OPER_H__
#define __RFIC_OPER_H__



#define CONFIG_CARD_MAX_COUNT   2
#define POWER_CARD_MAX_COUNT    2
#define USER_CARD_MAX_COUNT     251


typedef struct
{       /* date and time components */
    int     sec;    //senconds after the minute, 0 to 59
    int     min;    //minutes after the hour, 0 to 59
    int     hour;   //hours since midnight, 0 to 23
    int     mday;   //day of the month, 1 to 31
    int     month;  //months of the year, 1 to 12
    int     year;   //years, START_YEAR to START_YEAR+135
    int     wday;   //days since Sunday, 0 to 6
    int     yday;   //days of the year, 1 to 366
}Calendar_Def;

typedef enum{
    user_card = 0,
    power_card,
    config_card,
    other_card
}card_type_t;

typedef enum{
    check_card_state = 0,
    power_card_open_door_state,
    user_card_open_door_state,
	open_close_door_state,
    config_card_setting_state,
    setting_sys_datetime_state,
    setting_close_timeout_state,
    setting_device_address_state,
    make_user_card_state,
    make_power_card_state,
    make_config_card_state
}sys_state_t;

typedef struct{
    unsigned short year;
    unsigned char month;
    unsigned char mday;
    unsigned char hour;
    unsigned char min;
    unsigned char sec;
}setting_datetime_t;

typedef struct{
    unsigned short device_addr0;
    unsigned char device_addr1;
    unsigned char device_door_count;
}setting_device_addr_t;

typedef struct{
    unsigned char   config_card_count;
    unsigned char   power_card_count;
	unsigned char	power_card_number;
    unsigned char   user_card_count;
    sys_state_t     sys_state;
    sys_state_t     sys_last_state;    
//    unsigned char   execute_open_status[USER_CARD_MAX_COUNT];
//    unsigned char   echo_open_status[USER_CARD_MAX_COUNT];
    setting_datetime_t  setting_datetime;
//    unsigned char   setting_close_door_timeout;
    setting_device_addr_t setting_device_addr;
	unsigned char	open_all_tag;
	unsigned char	open_all_index;
//	unsigned char	last_screen_index;
//    unsigned char   u_disk_ready;
//	unsigned char	ex_ic_ok_tag;
}sys_status_t;

typedef struct{
    unsigned short data_tag;
    unsigned char card_id[4];
    unsigned char card_number;
    card_type_t card_type;
    unsigned char open_count_per_day;
    unsigned char open_door_start_h;
    unsigned char open_door_start_m;
    unsigned char open_door_end_h;
    unsigned char open_door_end_m;
    unsigned char crc8;
}card_t;

typedef struct{
    unsigned short data_tag;
    card_t cards[POWER_CARD_MAX_COUNT+CONFIG_CARD_MAX_COUNT+USER_CARD_MAX_COUNT];
    unsigned char   close_door_timeout;
    unsigned short  device_addr0;
    unsigned char   device_addr1;
    unsigned char   device_door_count;
    unsigned char crc8;
}sys_config_t;



#endif
