#include <rtthread.h>
#include <rtdevice.h>
#include "drv_pcf8563.h"

#define  PCF_ADDR  (0xA2>>1)

static rt_uint8_t time_buf[7] = {0};  //用于保存时间,[0]秒，[1]分，[2]时，[3]日，[4]星期，[5]月，[6]年
static const rt_uint8_t tab_week[12]={0,3,3,6,1,4,6,2,5,0,3,5}; //月修正数据表

static struct rt_i2c_bus_device  *pcf_bus;
static struct rt_mutex mutex_dt;

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

rt_uint8_t rtc_set_time(int year, int month, int mday, int hour, int min, int sec)
{
	rt_uint8_t ds;
	rt_mutex_take(&mutex_dt,RT_WAITING_FOREVER);	
	time_buf[0] = sec/10;		//秒
	time_buf[0] <<= 4;
	time_buf[0] |= (sec % 10);
	time_buf[1] = min/10;  	//分
	time_buf[1] <<= 4;
	time_buf[1] |= (min % 10);
	time_buf[2] = hour/10;  	//时
	time_buf[2] <<= 4;
	time_buf[2] |= (hour % 10);
	//
	time_buf[3] = mday / 10;	//日
	time_buf[3] <<= 4;
	time_buf[3] |= (mday % 10); 
	time_buf[4] = ymd_to_wday(year,month,mday);	//星期
	time_buf[5] = 0;						//世纪0或1
	time_buf[5] <<= 3;
	time_buf[5] |= month / 10;	//月
	time_buf[5] <<= 4;
	time_buf[5] |= month % 10;				
	time_buf[6] = (year % 100) / 10;	//年
	time_buf[6] <<= 4;
	time_buf[6] |= (year % 100) % 10;	
	
	ds=pcf8563_set_time(time_buf);
	rt_mutex_release(&mutex_dt);
	return ds;
}

rt_uint8_t rtc_get_time(int *year, int *month, int *mday, int *wday, int *hour, int *min, int *sec)
{
	uint64_t ds;
	rt_mutex_take(&mutex_dt,RT_WAITING_FOREVER);
	ds = pcf8563_get_time(time_buf);
	rt_mutex_release(&mutex_dt);
	if(ds!=1) return 0;
	*year = 2000 + ((time_buf[6] & 0xff)>>4)*10 + (time_buf[6] & 0x0f);
	if(*year<2011||*year>2145)*year = 2011;
    *month = ((time_buf[5] & 0x1f)>>4)*10 + (time_buf[5] & 0x0f);
	if(*month<1 || *month>12)*month = 1;
	*mday = ((time_buf[3] & 0x3f)>>4)*10 + (time_buf[3] & 0x0f);
	if(*mday<1 || *mday>31)*mday = 1;
	*hour = ((time_buf[2] & 0x3f)>>4)*10 + (time_buf[2] & 0x0f);
	if(*hour<0 || *hour>23)*hour = 0;
	*min = ((time_buf[1] & 0x7f)>>4)*10 + (time_buf[1] & 0x0f);
	if(*min<0 || *min>59)*min = 0;
	*sec = ((time_buf[0] & 0x7f)>>4)*10 + (time_buf[0] & 0x0f);
	if(*sec<0 || *sec>59)*sec = 0;
	*wday = ymd_to_wday(*year, *month, *mday);
	return 1;
}

static int pcf8563_init(void)
{
	rt_mutex_init(&mutex_dt, "dt_lock", RT_IPC_FLAG_FIFO);
	pcf_bus = rt_i2c_bus_device_find(I2C_BUS_NAME);
	if(pcf_bus == RT_NULL)
	{
		rt_kprintf("pcf8563 iic bus not find ! \n");
		return -RT_ERROR;
	}
	RT_ASSERT(pcf_bus != RT_NULL);
	
	return RT_EOK;
}
INIT_ENV_EXPORT(pcf8563_init);
