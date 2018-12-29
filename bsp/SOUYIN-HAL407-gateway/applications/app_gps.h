#ifndef __APP_GPS_H__
#define __APP_GPS_H__

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

typedef void (*gps_update_hook_t)(calendar_t cal);

void gps_set_update_hook(gps_update_hook_t hook);

#endif
