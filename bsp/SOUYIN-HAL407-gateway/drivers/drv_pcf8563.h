#ifndef __DRV_PCF8563_H__
#define __DRV_PCF8563_H__

#include <rtthread.h>

struct calendar
{       /* date and time components */
    int     sec;    //senconds after the minute, 0 to 59
    int     min;    //minutes after the hour, 0 to 59
    int     hour;   //hours since midnight, 0 to 23
    int     mday;   //day of the month, 1 to 31
    int     month;  //months of the year, 1 to 12
    int     year;   //years, START_YEAR to START_YEAR+135
    int     wday;   //days since Sunday, 0 to 6
    int     yday;   //days of the year, 1 to 366
};
typedef struct calendar *calendar_t;

rt_uint8_t rtc_set_time(calendar_t ptime);
rt_uint8_t rtc_get_time(calendar_t ptime);
rt_uint8_t ymd_to_wday(int year, int month, int mday);

#endif
