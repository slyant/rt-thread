#ifndef __DRV_PCF8563_H__
#define __DRV_PCF8563_H__

#include <rtthread.h>
#include <rtc_calendar.h>

rt_err_t rtc_set_time(calendar_t cal);
rt_err_t rtc_get_time(calendar_t cal);

#endif
