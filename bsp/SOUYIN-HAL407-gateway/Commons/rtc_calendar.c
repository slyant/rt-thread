#include "rtc_calendar.h"

static const unsigned char mon_table[12]={31,28,31,30,31,30,31,31,30,31,30,31};//平年的月份日期数据表
static const unsigned char table_week[12]={0,3,3,6,1,4,6,2,5,0,3,5}; //月修正数据表

//计算两个日期（calendar1 - calendar0）的时间差（秒）
//isN:输出参数，是否为负值，1为负值，0为正值
//return:返回两个日期的差（秒）
unsigned long get_difftime(calendar_t calendar1, calendar_t calendar0, unsigned char *isN)
{
  unsigned long t1, t0, dt;  
  t1 = rtc_make_time(calendar1);//call function in local file
  t0 = rtc_make_time(calendar0);//call function in local file
  if(t1 >= t0) 
  {
    dt = t1 - t0;//get abs value
    *isN = FALSE;
  }
  else
  {
    dt = t0 - t1;//get abs value
    *isN = TRUE;
  }
    
  return dt;
}

//判断calendar2是否在calendar0和calendar1之间
unsigned char datetime_is_between(calendar_t calendar0, calendar_t calendar1, calendar_t calendar2)
{
    unsigned char isN = TRUE;
    get_difftime(calendar2, calendar0, &isN);
    if(isN==FALSE)
    {
        get_difftime(calendar1, calendar2, &isN);
    }
    if(isN==FALSE)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

//将UTC日期转换为UTC秒
unsigned long rtc_make_time(calendar_t calendar)
{
	unsigned int t;
	unsigned long seccount=0;
	if(calendar->year<START_YEAR||calendar->year>END_YEAR)return 0;  
	for(t=START_YEAR;t<calendar->year;t++)
	{
		  if(is_leap(t))seccount+=31622400;
		  else seccount+=31536000;
	}
	for(t=0;t<calendar->month-1;t++)
	{
		  seccount+=(unsigned long)mon_table[t]*86400;
		  if(is_leap(calendar->year)&&t==1)seccount+=86400;
	}
	seccount+=(unsigned long)(calendar->mday-1)*86400;
	seccount+=(unsigned long)calendar->hour*3600;
	seccount+=(unsigned long)calendar->min*60;
	seccount+=calendar->sec;
	return seccount;
} 

//将UTC秒转换为UTC日期
void rtc_local_time(unsigned long total_secs, calendar_t calendar)
{
    unsigned long timecount = 0;
    unsigned long temp = 0;
    unsigned long temp1 = 0;
    timecount = total_secs;
    temp = timecount / SEC_IN_DAY;    
    //
    calendar->year = START_YEAR;
    calendar->month = 1;
    calendar->mday = 1;
    calendar->hour = 0;
    calendar->min = 0;
    calendar->sec = 0;
    //
    if(temp>0)
    {
        temp1 = START_YEAR;
        while(temp>=365)
        {
            if(is_leap(temp1))
            {
                if(temp>=366)
                {
                    temp -= 366;
                }
                else
                {
                    break;
                }
                temp1++;
            }
            else
            {
                temp -= 365;
                temp1++;
            }
        }
        calendar->year = temp1;
        temp1 = 0;
        while(temp>=28)
        {
            if(is_leap(calendar->year) && temp1==1)
            {
                if(temp>=29)
                {
                    temp -= 29;
                }
                else                    
                {
                    break;
                }
            }
            else
            {
                if(temp>=mon_table[temp1])
                {
                    temp -= mon_table[temp1];
                }
                else
                {
                    break;
                }
            }
            temp1++;
        }
        calendar->month = temp1 + 1;
        calendar->mday = temp + 1;
    }
    temp = timecount % SEC_IN_DAY;
    calendar->hour = temp/SEC_IN_HOUR;
    calendar->min = (temp%SEC_IN_HOUR)/60;
    calendar->sec = (temp%SEC_IN_HOUR)%60;
    calendar->wday = ymd2wday(calendar->year,calendar->month,calendar->mday);
}

//根据年月日计算星期
int ymd2wday(int nYear, int nMonth, int nMday)
{ 
   unsigned int temp2;
   unsigned char yearH,yearL;  
   yearH=nYear/100;
    yearL=nYear%100;
   if (yearH>19)yearL+=100;
   temp2=yearL+yearL/4;
   temp2=temp2%7;
   temp2=temp2+nMday+table_week[nMonth-1];
   if (yearL%4==0&&nMonth<3)temp2--;
   return(temp2%7);
}  
//判断year年是否为润年
unsigned char is_leap(int year)
{
	if(year%4==0)
	{
		if(year%100==0)
		{
			if(year%400==0)
            {
                return 1;
            }
            else
            {
                return 0;
            }
		}
        else
        {
            return 1;
        }
	}
    else
    {
        return 0;
    }
}
