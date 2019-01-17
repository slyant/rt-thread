#ifndef __APP_GPS_H__
#define __APP_GPS_H__

#include <rtc_calendar.h>


typedef void (*gps_update_hook_t)(calendar_t cal);

void gps_update_set_hook(gps_update_hook_t hook);

#endif
