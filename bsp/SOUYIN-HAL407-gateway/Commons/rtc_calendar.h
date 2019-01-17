#ifndef __RTC_CALENDAR_H__
#define __RTC_CALENDAR_H__


/***********************************************************************
*START_YEAR and START_WDAY are programmer determined definitions.
*Through those, a calendar from START_YEAR to START_YEAR+135 can be gotten.
*Do not need to care about local time or daylight saving time.
*When the user calibrates the calendar to a certain time zone, 
*it will run based on this zone.
***********************************************************************/
#define   START_YEAR      		(1970)//default 1970, Jan.1, 00:00:00
#define	  END_YEAR				(START_YEAR+135)
#define   SEC_IN_DAY      		(24*60*60)//one day includes 86400 seconds
#define	  SEC_IN_HOUR	  		(60*60)
#define   DAY_IN_YEAR(nYear)	(is_leap(nYear) ? 366 : 365)  

#define TRUE	1
#define FALSE	0

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

unsigned char is_leap(int nYear);
int ymd2wday(int nYear, int nMonth, int nMday);	
//get difference time in seconds between 2 calenders
unsigned long get_difftime(calendar_t calendar1, calendar_t calendar0, unsigned char *isN);
//calendar1 >= calendar2 >= calendar0 ?
unsigned char datetime_is_between(calendar_t calendar0, calendar_t calendar1, calendar_t calendar2);
unsigned long rtc_make_time(calendar_t calendar);
void rtc_local_time(unsigned long total_secs, calendar_t calendar);

#endif

/***************************END OF FILE********************************/
