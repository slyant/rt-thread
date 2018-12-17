

/**********************************  额外添加的函数    ******************************/


#include "gd32.h"
#include "gpio_oper.h"
#include <spi_flash_sfud.h>


#define  W25QXX_CS   88

//static int w25qxx_init(void)
//{
//	stm32_spi_bus_attach_device(W25QXX_CS,SPI3_BUS_NAME,SPI3_DEV_NAME);
//	gd_init(SPI3_BUS_NAME,SPI3_DEV_NAME);
//	return 0;
//}
//INIT_ENV_EXPORT(w25qxx_init);

static int rt_hw_spi_flash_with_sfud_init(void)
{
    if (RT_NULL == rt_sfud_flash_probe("DG32", SPI3_DEV_NAME))
    {
        return RT_ERROR;
    };

    return RT_EOK;
}
INIT_APP_EXPORT(rt_hw_spi_flash_with_sfud_init);

#ifdef NRF24L01_USING_GATEWAY

void sark_param_store(sark_msg_t arg)
{
	uint8_t tmpbuf[256];
	uint16_t ts;
//	w25qxx_read(0,tmpbuf,256);
	ts = (arg->node-31)<<3;
	tmpbuf[ts] =  arg->node;    ts++;
	tmpbuf[ts] =  arg->num;     ts++;
	tmpbuf[ts] =  arg->ov_time; ts++;
	tmpbuf[ts] =  arg->name[0]; ts++;
	tmpbuf[ts] =  arg->name[1]; ts++;
	tmpbuf[ts] =  arg->name[2]; ts++;
	tmpbuf[ts] =  arg->name[3]; ts++;
	tmpbuf[ts] =  arg->name[4]; 
//	w25qxx_page_write(0,tmpbuf);
}

void sark_param_read(uint8_t which, uint8_t* buf)
{
//	w25qxx_read((which-31)<<3,buf,8);
}

void sark_param_clear(uint8_t which)
{
	uint8_t tmpbuf[256];
	uint8_t  ts,i;
	if(which<31 || which>63) return;
	ts = (which-31)<<3;
//	w25qxx_read(0,tmpbuf,256);
	for(i=0;i<8;i++)
	{
		tmpbuf[ts+i] = 0xFF;
	}
//	w25qxx_page_write(0,tmpbuf);
}

#else

void w25_store_param(rt_uint8_t addr,rt_uint32_t pwd)
{
	uint8_t tmpbuf[256];
	w25qxx_read(0,tmpbuf,256);
	tmpbuf[0] =  ((pwd>>24)&0xFF);
	tmpbuf[1] =  ((pwd>>16)&0xFF);
	tmpbuf[2] =  ((pwd>>8)&0xFF);
	tmpbuf[3] =  (pwd&0xFF);
	w25qxx_page_write(0,tmpbuf);
}

void w25_read_param(rt_uint8_t *addr,rt_uint32_t *pwd)
{
	uint8_t ts;
	uint8_t tmp[4];
	ts = get_nrf_addr();
	*addr = 31 + ts;
	w25qxx_read(0,tmp,4);
	*pwd = (uint32_t) tmp[0]<<24 | (uint32_t) tmp[1]<<16 | (uint32_t) tmp[2]<<8 | (uint32_t) tmp[3];
	if(*pwd==0xFFFFFFFF || *pwd==0) *addr = 0;
}

void w25_clear_store_data(void)
{
	uint8_t tmpbuf[256];
	uint16_t ts;
	w25qxx_read(0,tmpbuf,128);
	for(ts=0;ts<256;ts++)
	{
		tmpbuf[ts] = 0xFF;
	}
	w25qxx_page_write(0,tmpbuf);
}

#endif


/**********************************  end adding funs    *****************************/


