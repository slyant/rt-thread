#include <rtthread.h>
#include <rtdevice.h>
#include "drv_pcf8563.h"

#define  PCF_ADDR  0x51

rt_uint8_t time_buf[7] = {0};  //用于保存时间,[0]秒，[1]分，[2]时，[3]日，[4]星期，[5]月，[6]年
const rt_uint8_t tab_week[12]={0,3,3,6,1,4,6,2,5,0,3,5}; //月修正数据表

static struct rt_i2c_bus_device  *pcf_bus;

static rt_uint8_t pcf8563_set_time(rt_uint8_t * buf)
{
	rt_uint8_t ts,i,tbuf[8];
	tbuf[0]=2;
	for(i=0;i<8;i++)
	{
		tbuf[i+1] = buf[i];
	}
	ts = rt_i2c_master_send(pcf_bus,PCF_ADDR,0,tbuf,8);
	if(ts == 8) return 1;
	else return 0;
}

static rt_uint8_t pcf8563_get_time(rt_uint8_t * buf)
{
	rt_uint8_t i,ts,rbuf[7];
	rbuf[0] = 2;
	ts = rt_i2c_master_send(pcf_bus,PCF_ADDR,0,&rbuf[0],1);
	if(ts==1)
	{
		ts = rt_i2c_master_recv(pcf_bus,PCF_ADDR,0,rbuf,7);
		if(ts==7)
		{
			for(i=0;i<7;i++)
			{
				buf[i] = rbuf[i];
			}
			return 1;
		}
	}
	return 0;
}

rt_uint8_t ymd_to_wday(int year, int month, int mday)
{
	int tmp;
	rt_uint8_t yearh,yearl;
	yearh = year/100;
	yearl = year&100;
	if(yearh>19) yearl+=100;
	tmp = (yearl+yearl/4)%7;
	tmp += mday+tab_week[month-1];
	if(yearl%4 ==0 && month<3) tmp--;
	return tmp%7;
}

rt_uint8_t rtc_set_time(calendar_t ptime)
{
	rt_uint8_t ds;
	time_buf[0] = ptime->sec/10;		//秒
	time_buf[0] <<= 4;
	time_buf[0] |= (ptime->sec % 10);
	time_buf[1] = ptime->min/10;  	//分
	time_buf[1] <<= 4;
	time_buf[1] |= (ptime->min % 10);
	time_buf[2] = ptime->hour/10;  	//时
	time_buf[2] <<= 4;
	time_buf[2] |= (ptime->hour % 10);
	//
	time_buf[3] = ptime->mday / 10;	//日
	time_buf[3] <<= 4;
	time_buf[3] |= (ptime->mday % 10); 
	time_buf[4] = ymd_to_wday(ptime->year,ptime->month,ptime->mday);	//星期
	time_buf[5] = 0;						//世纪0或1
	time_buf[5] <<= 3;
	time_buf[5] |= ptime->month / 10;	//月
	time_buf[5] <<= 4;
	time_buf[5] |= ptime->month % 10;				
	time_buf[6] = (ptime->year % 100) / 10;	//年
	time_buf[6] <<= 4;
	time_buf[6] |= (ptime->year % 100) % 10;	
	
	ds=pcf8563_set_time(time_buf);
	return ds;
}

rt_uint8_t rtc_get_time(calendar_t ptime)
{
	uint64_t ds;
	ds = pcf8563_get_time(time_buf);
	if(ds!=1) return 0;
	ptime->year = 2000 + ((time_buf[6] & 0xff)>>4)*10 + (time_buf[6] & 0x0f);
	if(ptime->year<2011||ptime->year>2145)ptime->year = 2011;
    ptime->month = ((time_buf[5] & 0x1f)>>4)*10 + (time_buf[5] & 0x0f);
	if(ptime->month<1 || ptime->month>12)ptime->month = 1;
	ptime->mday = ((time_buf[3] & 0x3f)>>4)*10 + (time_buf[3] & 0x0f);
	if(ptime->mday<1 || ptime->mday>31)ptime->mday = 1;
	ptime->hour = ((time_buf[2] & 0x3f)>>4)*10 + (time_buf[2] & 0x0f);
	if(ptime->hour<0 || ptime->hour>23)ptime->hour = 0;
	ptime->min = ((time_buf[1] & 0x7f)>>4)*10 + (time_buf[1] & 0x0f);
	if(ptime->min<0 || ptime->min>59)ptime->min = 0;
	ptime->sec = ((time_buf[0] & 0x7f)>>4)*10 + (time_buf[0] & 0x0f);
	if(ptime->sec<0 || ptime->sec>59)ptime->sec = 0;
	ptime->wday = ymd_to_wday(ptime->year,ptime->month,ptime->mday);
	return 1;
}

static int pcf8563_init(void)
{
	pcf_bus = rt_i2c_bus_device_find(I2C_BUS_NAME);
	if(pcf_bus == RT_NULL)
		rt_kprintf("pcf8563 iic bus not find ! \n");
	return RT_EOK;
}
INIT_ENV_EXPORT(pcf8563_init);
