#ifndef __DRV_PCF8563_H__
#define __DRV_PCF8563_H__

#include <rtthread.h>

rt_uint8_t rtc_set_time(int year, int month, int mday, int hour, int min, int sec);
rt_uint8_t rtc_get_time(int *year, int *month, int *mday, int *wday, int *hour, int *min, int *sec);
rt_uint8_t ymd_to_wday(int year, int month, int mday);

#endif
