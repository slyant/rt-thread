#ifndef __APP_RFIC_H__
#define __APP_RFIC_H__

#include <rtthread.h>

rt_bool_t init_card_key(void);
rt_bool_t reset_card_key(void);

rt_bool_t init_card_app(void);
rt_bool_t reset_card_app(void);

rt_bool_t create_app_abkey(void);
rt_bool_t create_app_config(void);
rt_bool_t create_app_power(void);

#endif
