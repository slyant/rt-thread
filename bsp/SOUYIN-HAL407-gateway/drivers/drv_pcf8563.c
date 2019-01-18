#include <rtthread.h>
#include <rtdevice.h>
#include <drv_pcf8563.h>

#define  PCF_ADDR  (0xA2>>1)
static rt_uint8_t time_buf[7];  //用于保存时间,[0]秒，[1]分，[2]时，[3]日，[4]星期，[5]月，[6]年
static struct rt_i2c_bus_device  *pcf_bus;
static struct rt_mutex mutex_dt;

static rt_err_t pcf8563_set_time(rt_uint8_t * buf)
{
	rt_uint8_t ts,i,tbuf[8];
	tbuf[0]=2;
	for(i=0;i<8;i++)
	{
		tbuf[i+1] = buf[i];
	}
	ts = rt_i2c_master_send(pcf_bus,PCF_ADDR,0,tbuf,8);
	if(ts == 8) return RT_EOK;
	else return -RT_ERROR;
}

static rt_err_t pcf8563_get_time(rt_uint8_t * buf)
{
	rt_uint8_t i,ts,rbuf[7];
	rbuf[0] = 2;
	ts = rt_i2c_master_send(pcf_bus,PCF_ADDR,0,rbuf,1);
	if(ts==1)
	{
		ts = rt_i2c_master_recv(pcf_bus,PCF_ADDR,0,rbuf,7);
		if(ts==7)
		{
			for(i=0;i<7;i++)
			{
				buf[i] = rbuf[i];
			}
			return RT_EOK;
		}
	}
	return -RT_ERROR;
}

rt_err_t rtc_set_time(calendar_t cal)
{
	rt_err_t result;
	rt_mutex_take(&mutex_dt,RT_WAITING_FOREVER);	
	time_buf[0] = cal->sec/10;		//秒
	time_buf[0] <<= 4;
	time_buf[0] |= (cal->sec % 10);
	time_buf[1] = cal->min/10;  	//分
	time_buf[1] <<= 4;
	time_buf[1] |= (cal->min % 10);
	time_buf[2] = cal->hour/10;  	//时
	time_buf[2] <<= 4;
	time_buf[2] |= (cal->hour % 10);
	//
	time_buf[3] = cal->mday / 10;	//日
	time_buf[3] <<= 4;
	time_buf[3] |= (cal->mday % 10); 
	time_buf[4] = ymd2wday(cal->year, cal->month, cal->mday);	//星期
	time_buf[5] = 0;						//世纪0或1
	time_buf[5] <<= 3;
	time_buf[5] |= cal->month / 10;	//月
	time_buf[5] <<= 4;
	time_buf[5] |= cal->month % 10;				
	time_buf[6] = (cal->year % 100) / 10;	//年
	time_buf[6] <<= 4;
	time_buf[6] |= (cal->year % 100) % 10;	
	
	result = pcf8563_set_time(time_buf);
	rt_mutex_release(&mutex_dt);
	return result;
}

rt_err_t rtc_get_time(calendar_t cal)
{
	rt_err_t result;
	rt_mutex_take(&mutex_dt,RT_WAITING_FOREVER);
	result = pcf8563_get_time(time_buf);
	rt_mutex_release(&mutex_dt);
	if(result == RT_EOK)
    {
        cal->year = 2000 + ((time_buf[6] & 0xff)>>4)*10 + (time_buf[6] & 0x0f);
        if(cal->year<2011||cal->year>2145)cal->year = 2011;
        cal->month = ((time_buf[5] & 0x1f)>>4)*10 + (time_buf[5] & 0x0f);
        if(cal->month<1 || cal->month>12)cal->month = 1;
        cal->mday = ((time_buf[3] & 0x3f)>>4)*10 + (time_buf[3] & 0x0f);
        if(cal->mday<1 || cal->mday>31)cal->mday = 1;
        cal->hour = ((time_buf[2] & 0x3f)>>4)*10 + (time_buf[2] & 0x0f);
        if(cal->hour<0 || cal->hour>23)cal->hour = 0;
        cal->min = ((time_buf[1] & 0x7f)>>4)*10 + (time_buf[1] & 0x0f);
        if(cal->min<0 || cal->min>59)cal->min = 0;
        cal->sec = ((time_buf[0] & 0x7f)>>4)*10 + (time_buf[0] & 0x0f);
        if(cal->sec<0 || cal->sec>59)cal->sec = 0;
        cal->wday = ymd2wday(cal->year, cal->month, cal->mday);
    }
	return result;
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
