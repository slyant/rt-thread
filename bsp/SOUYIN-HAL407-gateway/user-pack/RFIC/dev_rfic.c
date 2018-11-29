

#include <rtthread.h>
#include <string.h>
#include <drv_spi.h>
#include <drv_usart.h>
#include "dev_rfic.h"
#include "ic_card_protocol.h"
#include "stm32f4xx_hal.h"


#define RFIC_LOG  rt_kprintf("[RFIC]:");rt_kprintf

#define MAXRLEN 18
unsigned char com_mf522_buf[MAXRLEN];

static void rfic_delay_us(rt_uint32_t nus)
{
	extern void delay_us(rt_uint32_t nus);
	delay_us(nus);
}

static void rfic_delay_ms(rt_uint32_t nms)
{
	extern void delay_ms(rt_uint32_t nms);
	delay_ms(nms);
}

#ifdef  RC_BUS_SPI
	
	#define RFIC_RST_PIN   29

	#define RFIC_CFG                 			\
	{                                               		\
		.mode = RT_SPI_MODE_0 | RT_SPI_MSB | RT_SPI_MASTER,	\
		.data_width = 8,                            		\
		.max_hz = 10 * 1000 * 1000,                 		\
	}

	static rt_mutex_t rfic_device_mutex_lock = RT_NULL;
	static struct rt_spi_device *rfic_device = RT_NULL;
	struct rt_spi_configuration cfg = RFIC_CFG;
	
	static void rfic_lock(void)
	{	
		rt_mutex_take(rfic_device_mutex_lock, RT_WAITING_FOREVER);
	}

	static void rfic_unlock(void)
	{
		rt_mutex_release(rfic_device_mutex_lock);
	}
	
#endif

#ifdef RC_BUS_UART
	
	#define RFIC_RST_PIN   39
	#define UART_RX_EVENT (1 << 0)
	static struct rt_event rfic_event;
	static rt_device_t rfic_device = RT_NULL;
	
	static rt_err_t rfic_rx_call_back(rt_device_t dev, rt_size_t size)
	{
		/* �����¼� */
		rt_event_send(&rfic_event, UART_RX_EVENT);

		return RT_EOK;
	}
	
	static void rfic_putchar(const rt_uint8_t c)
	{
		rt_size_t len=0;
		rt_uint32_t timeout = 0;
		do{
			len = rt_device_write(rfic_device,0,&c,1);
			timeout++;
		}while(len!=1 && timeout<500);
	}
	
	rt_uint8_t rfic_getchar(void)
	{
		rt_uint32_t e;
		rt_uint8_t ch;

		/* ��ȡ1�ֽ����� */
		while (rt_device_read(rfic_device, 0, &ch, 1) != 1)
		{
			 /* �����¼� */
			rt_event_recv(&rfic_event, UART_RX_EVENT,RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR,RT_WAITING_FOREVER, &e);
		}

		return ch;
	}

#endif
	
#ifdef RC_BUS_IIC

	#define RFIC_RST_PIN   39

	#define  IIC_USE_RTOS_FUN    0      //�����ֵȷ��ʹ�õĺ�����0ʹ���û��Լ��ĺ�����1ʹ��rtthreadϵͳ�ĺ���      
	
	#if IIC_USE_RTOS_FUN == 1
	
		#define  RDIC_ADDR      0x28
		static struct rt_i2c_bus_device   *rfic_card_bus;     //RFIC���豸���

	#else
		
		#define  RDIC_ADDR     0x50
		
		#define  RFIC_SDA_PIN  38
		#define  RFIC_SCL_PIN  37
		
		#define  SET_IIC_SDA(d)     rt_pin_write(RFIC_SDA_PIN,(rt_base_t)(d));
		#define  SET_IIC_SCL(d)     rt_pin_write(RFIC_SCL_PIN,(rt_base_t)(d));
	
	
	static void iic_start(void)
	{
		SET_IIC_SDA(1);     //������ʼ����
		SET_IIC_SCL(1);
		rfic_delay_us(6);
		SET_IIC_SDA(0);      //������ʼ�ź�
		rfic_delay_us(5);
		SET_IIC_SCL(0);
		rfic_delay_us(5);                        //ǯס����
	}
	
	static void iic_stop(void)
	{
		SET_IIC_SDA(0);
		rfic_delay_us(5);
		SET_IIC_SCL(1);
		rfic_delay_us(5);
		SET_IIC_SDA(1);     //�ͷ�����
		rfic_delay_us(5);  
	}
	
	static void iic_send_ack(unsigned char ack)     //����Ӧ�𣨲�Ӧ���ź�:0=Ӧ��1=��Ӧ��
	{
		SET_IIC_SDA(ack);
		rfic_delay_us(2);
		SET_IIC_SCL(1);
		rfic_delay_us(5);
		SET_IIC_SCL(0);
		rfic_delay_us(5);
	}
	
	static uint8_t iic_read_ack(void)               //��ѯӦ���ź�
	{
		uint8_t ack;
		SET_IIC_SDA(1);       //�ͷ�������
		SET_IIC_SCL(1);
		rfic_delay_us(5);
		ack = rt_pin_read(RFIC_SDA_PIN);
		SET_IIC_SCL(0);
		rfic_delay_us(5);
		return ack;
	}
	
	static void iic_send_byte(unsigned char txd)
	{
		unsigned char t;
		SET_IIC_SCL(0);
		rfic_delay_us(5);
		for(t=0;t<8;t++)
		{
			SET_IIC_SDA((txd&0X80)>>7);
			txd<<=1;
			rfic_delay_us(5);
			SET_IIC_SCL(1);
			rfic_delay_us(5);
			SET_IIC_SCL(0);
			rfic_delay_us(5);
		}
	}
	
	unsigned char iic_read_byte(void)
	{
		unsigned char retc;
		unsigned char i;
		retc=0;
		
		for(i=0;i<8;i++)
		{
			SET_IIC_SCL(1);
			retc = retc<<1;
			rfic_delay_us(5);
			if(rt_pin_read(RFIC_SDA_PIN)) retc|=1;
			SET_IIC_SCL(0);
			rfic_delay_us(5);
		}
		return retc;
	}
	#endif
	
	
	/*        �������ݺ���������0���ʹ��󣬷���1��ɷ���       */
	static uint8_t iic_send_data(uint8_t addr, uint8_t addr_sub, uint8_t *buf, uint8_t len)
	{
		#if IIC_USE_RTOS_FUN ==1
		
		    uint8_t tbuf[len+1] ,i;
			tbuf[0] = addr_sub;      //�Ĵ�����ַ
			for(i=0;i<len;i++)	{ tbuf[1+i] = buf[i]; }
			if(rt_i2c_master_send(rfic_card_bus, addr, 0, tbuf, len+1)==len+1 )
				return 1;
			else return 0;
			
		#else
			uint8_t i;
			iic_start();
			iic_send_byte(addr);
			if(iic_read_ack()) return 0;
			iic_send_byte(addr_sub);
			if(iic_read_ack()) return 0;
			for(i=0;i<len;i++)
			{
				iic_send_byte(*buf);
				if(iic_read_ack()) return 0;
				buf++;
			}
			iic_stop();
			return 1;
		#endif
	}
	/*        �����ݺ���������0������ȡ���󣬷���1�ɹ���ȡ       */
	static uint8_t iic_read_data(uint8_t addr, uint8_t addr_sub, uint8_t *buf, uint8_t len)
	{
		#if IIC_USE_RTOS_FUN == 1

			if(rt_i2c_master_send(rfic_card_bus, addr, 0, &addr_sub, 1)==1)   //����д�������к����ֹͣ����
			{
				if(rt_i2c_master_recv(rfic_card_bus, addr, 1, buf, len)==len)
					return 1;
				else return 0;
			}
			else return 0;
			
		#else
			uint8_t i;
			iic_start();
			iic_send_byte(addr);
			if(iic_read_ack()) return 0;
			iic_send_byte(addr_sub);
			if(iic_read_ack()) return 0;
			
			iic_stop();
			rfic_delay_us(5);
			
			iic_start();
			iic_send_byte(addr+1);
			if(iic_read_ack()) return 0;
			for(i=0;i<len-1;i++)
			{
				*buf=iic_read_byte();
				iic_send_ack(0);         //����Ӧ��
				buf++;
			}
			*buf=iic_read_byte();
			iic_send_ack(1);              //������Ӧ��
			iic_stop();
			return 1;
		#endif
	}
	
	
#endif


/////////////////////////////////////////////////////////////////////
//��    �ܣ�дRC632�Ĵ���
//����˵����Address[IN]:�Ĵ�����ַ
//          value[IN]:д���ֵ
//��    ��  1=OK 0=falut
/////////////////////////////////////////////////////////////////////
static uint8_t write_raw_rc_ex(unsigned char Address, unsigned char value)
{  
#ifdef RC_BUS_SPI  //SPI����
	unsigned char ucAddr[2];
	ucAddr[0] = (Address<<1)&0x7F ;
	ucAddr[1] = value ;
	rt_spi_send(rfic_device,ucAddr,2);
	return 1;
#endif
#ifdef RC_BUS_UART  //UART
	unsigned char ucAddr;
	ucAddr = Address & 0x3F;
    rfic_putchar(ucAddr);    //д������ӵ�ַ
    rfic_putchar(value);     //д����
	return 1;
#endif
#ifdef RC_BUS_IIC  //IIC
	unsigned char ucAddr;
	ucAddr = Address;
	return iic_send_data(RDIC_ADDR,ucAddr,&value,0x01);
#endif
}

/////////////////////////////////////////////////////////////////////
//��    �ܣ���RC632�Ĵ���
//����˵����Address[IN]:�Ĵ�����ַ
//��    �أ�������ֵ
/////////////////////////////////////////////////////////////////////
static uint8_t read_raw_rc_ex(unsigned char Address)
{
	unsigned char ucResult=0;
	unsigned char ucAddr;
#ifdef RC_BUS_SPI
    ucAddr = (Address<<1)|0x80;
	rt_spi_send_then_recv(rfic_device,&ucAddr,1,&ucResult,1);
//    rt_spi_send(rfic_device,&ucAddr,1);
//	ucResult = rt_spi_recv(rfic_device,&ucResult,1);
#endif
#ifdef RC_BUS_UART
    ucAddr = (Address&0x3F) | 0x80;
    rfic_putchar(ucAddr);                   //д�Ĵ�����ַ
	ucResult = rfic_getchar();              //���Ĵ�������
#endif
#ifdef RC_BUS_IIC
	ucAddr = Address;
    iic_read_data(RDIC_ADDR,ucAddr,&ucResult,0x01);
#endif	
	return ucResult;
}

/////////////////////////////////////////////////////////////////////
//��    �ܣ���λRC522
//��    ��: �ɹ�����0,���ɹ����س������
/////////////////////////////////////////////////////////////////////
static uint8_t pcd_reset_ex(void)
{
	char tmp;
	uint8_t data;
	tmp=0;
	rt_pin_write(RFIC_RST_PIN,PIN_HIGH);
	rfic_delay_us(10);
	rt_pin_write(RFIC_RST_PIN,PIN_LOW);
	rfic_delay_us(50);
	rt_pin_write(RFIC_RST_PIN,PIN_HIGH);
	rfic_delay_us(100);
	write_raw_rc_ex(COMMAND_REG,PCD_RESETPHASE);   //�������к�������߷�ʽ
#ifdef 	RC_BUS_UART
	tmp = read_raw_rc_ex(SERIAL_SPEED_REG);
	RFIC_LOG("SERIAL_SPEED_REG = 0x%x \n",tmp);
//	write_raw_rc_ex(SERIAL_SPEED_REG,0x7A);	//����MFRC522����ͨѶ����
	rfic_delay_us(10);
	tmp = read_raw_rc_ex(SERIAL_SPEED_REG);
	RFIC_LOG("SERIAL_SPEED_REG = 0x%x \n",tmp);
//	uart_set_baud_rate("uart1",BAUD_RATE_115200);    //�������Ʊ�����rt_thread��ʼ��ע���˵Ĵ������ƣ��ַ�����
#endif
	rfic_delay_us(10);
	
    write_raw_rc_ex(MODE_REG,0x3D);            //��Mifare��ͨѶ��CRC��ʼֵ0x6363
	data = read_raw_rc_ex(MODE_REG);
	if(data!=0x3D){RFIC_LOG(" read 0x%x \n",data); tmp++;}
	
    write_raw_rc_ex(T_RELOAD_REG_L,30);
	data = read_raw_rc_ex(T_RELOAD_REG_L);
	if(data!=30) {RFIC_LOG(" read 0x%x \n",data); tmp++;}
	
    write_raw_rc_ex(T_RELOAD_REG_H,0);
	data = read_raw_rc_ex(T_RELOAD_REG_H);
	if(data!=0) {RFIC_LOG(" read 0x%x \n",data); tmp++;}
	
    write_raw_rc_ex(TMOD_REG,0x8D);
	data = read_raw_rc_ex(TMOD_REG);
	if(data!=0x8D) {RFIC_LOG(" read 0x%x \n",data); tmp++;}
	
    write_raw_rc_ex(T_PRESCALER_REG,0x3E);
	data = read_raw_rc_ex(T_PRESCALER_REG);
	if(data!=0x3E) {RFIC_LOG(" read 0x%x \n",data); tmp++;}
	
    write_raw_rc_ex(TX_AUTO_REG,0x40);   
	data = read_raw_rc_ex(TX_AUTO_REG);
	if(data!=0x40) {RFIC_LOG(" read 0x%x \n",data); tmp++;}	
	
	return tmp;
}

/////////////////////////////////////////////////////////////////////
//��    �ܣ���RC522�Ĵ���λ
//����˵����reg[IN]:�Ĵ�����ַ
//          mask[IN]:��λֵ
/////////////////////////////////////////////////////////////////////
static void clear_bit_mask_ex(unsigned char reg,unsigned char mask)
{
	char tmp = 0x0;
    tmp = read_raw_rc_ex(reg);
    write_raw_rc_ex(reg, tmp & ~mask);  // clear bit mask
}

/////////////////////////////////////////////////////////////////////
//��    �ܣ���RC522�Ĵ���λ
//����˵����reg[IN]:�Ĵ�����ַ
//          mask[IN]:��λֵ
/////////////////////////////////////////////////////////////////////
static void set_bit_mask_ex(unsigned char reg,unsigned char mask)
{
	char tmp = 0x0;
    tmp = read_raw_rc_ex(reg);
    write_raw_rc_ex(reg,tmp | mask);  // set bit mask
}

/////////////////////////////////////////////////////////////////////
//��������  
//ÿ��������ر����շ���֮��Ӧ������1ms�ļ��
/////////////////////////////////////////////////////////////////////
static void pcd_antenna_on_ex(void)
{
    unsigned char i;
    i = read_raw_rc_ex(TX_CONTROL_REG);
    if (!(i & 0x03))
    {
        set_bit_mask_ex(TX_CONTROL_REG, 0x03);
    }
}

/////////////////////////////////////////////////////////////////////
//�ر�����
/////////////////////////////////////////////////////////////////////
static void pcd_antenna_off_ex(void)
{
    clear_bit_mask_ex(TX_CONTROL_REG, 0x03);
}

//////////////////////////////////////////////////////////////////////
//����RC632�Ĺ�����ʽ 
//////////////////////////////////////////////////////////////////////
static int m599_pcd_config_isotype_ex(unsigned char type)
{
	if (type == 'A')                     //ISO14443_A
	{ 
		clear_bit_mask_ex(STATUS2_REG,0x08);
		write_raw_rc_ex(MODE_REG,0x3D);//3F
		write_raw_rc_ex(RX_SEL_REG,0x86);//84
		write_raw_rc_ex(RFC_FG_REG,0x7F);   //4F
		write_raw_rc_ex(T_RELOAD_REG_L,30);//tmoLength);// TReloadVal = 'h6a =tmoLength(dec) 
		write_raw_rc_ex(T_RELOAD_REG_H,0);
		write_raw_rc_ex(TMOD_REG,0x8D);
		write_raw_rc_ex(T_PRESCALER_REG,0x3E);
		rfic_delay_ms(10);
		pcd_antenna_on_ex();
	}
	else{ return MI_NOTAGERR; }
   
	return MI_OK;
}

static int rt_hw_card_init(void)
{
	rt_uint8_t i;
	
	rt_pin_mode(RFIC_RST_PIN,PIN_MODE_OUTPUT);
	rt_pin_write(RFIC_RST_PIN,PIN_LOW);                 //���͸�λ��
	
#ifdef RC_BUS_SPI
	
	stm32_spi_bus_attach_device(ICCARD_SPI_CS_PIN, ICCARD_SPI_BUS_NAME, ICCARD_SPI_DEV_NAME);
	rfic_device = (struct rt_spi_device *)rt_device_find(ICCARD_SPI_DEV_NAME);
	
#endif
#ifdef RC_BUS_UART
	
	rt_err_t res;
	rfic_device = rt_device_find("uart1");
	if(rfic_device != RT_NULL)
	{
		if(rt_device_set_rx_indicate(rfic_device,rfic_rx_call_back) == RT_EOK )      //�������ݽ��ջص�����
			{RFIC_LOG("RFIC set callback OK ! \n");}
		else  {RFIC_LOG("RFIC set callback error ! \n");   return RT_ERROR; }
		
		res = rt_device_open(rfic_device, RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX );  //���豸
		if(res != RT_EOK)
		{	RFIC_LOG("open RFIC device(usart1) error ! \n"); return RT_ERROR;	}
		else { RFIC_LOG("RFIC device(usart1) was opened OK ! \n"); }
		
		if(rt_event_init(&rfic_event, "RFID_event", RT_IPC_FLAG_FIFO) == RT_EOK)
			{RFIC_LOG("RFID_event set OK ! \n");}
		else  {RFIC_LOG("RFID event seterror ! \n");   return RT_ERROR; }
	}		
	
#endif
	
#ifndef RC_BUS_IIC

	if(rfic_device==RT_NULL) { RFIC_LOG("RFIC device not found ! \n");	return RT_ERROR; }
	
#endif

#ifdef RC_BUS_IIC
	
	#if IIC_USE_RTOS_FUN == 1
	
		rfic_card_bus = rt_i2c_bus_device_find("i2c2");
		if(rfic_card_bus==RT_NULL)
		{
			RFIC_LOG(" can't find RFID &s device \n",rfic_card_bus);
			return RT_ERROR;
		}
		RFIC_LOG(" RFIC set i2c bus to %s \n",rfic_card_bus);
		
	#else
		rt_pin_mode(RFIC_SDA_PIN,PIN_MODE_OUTPUT_OD);
		rt_pin_mode(RFIC_SCL_PIN,PIN_MODE_OUTPUT_OD);
		rt_pin_write(RFIC_SDA_PIN,PIN_HIGH);
		rt_pin_write(RFIC_SCL_PIN,PIN_HIGH);
	#endif

#endif

#ifdef RC_BUS_SPI	
	
	if(rt_spi_configure(rfic_device, &cfg)==RT_EOK)
			{RFIC_LOG(" %s Device Init OK.\n", ICCARD_SPI_DEV_NAME);}
	else 	{RFIC_LOG(" %s Device Init fault !!! \n", ICCARD_SPI_DEV_NAME);}

	rfic_device_mutex_lock = rt_mutex_create("rfid_lock", RT_IPC_FLAG_FIFO);
	RT_ASSERT(rfic_device_mutex_lock != RT_NULL);
	
	rfic_lock();
#endif
    uint8_t ts;
	RFIC_LOG(" start initing... \n"); rt_kprintf(" \n");
	for(i=0;i<2;i++)
	{
		ts=pcd_reset_ex();     //��λ
		if(ts) 	{RFIC_LOG(" pcd reset hanved %d error ... \n", ts);}     //���ts��Ϊ0�򱨴���ָ��������
		else
		{
			rfic_delay_ms(2);
			pcd_antenna_off_ex();  //�ر�����
			rfic_delay_ms(10);
			pcd_antenna_on_ex();   //������
			rfic_delay_ms(2);
			if(m599_pcd_config_isotype_ex('A')!=MI_OK)   //���ù�����ʽ
				{RFIC_LOG(" m599 pcd config ERR ! \n");}
			else {RFIC_LOG(" m599 pcd config success ! \n");}
		}
	}
//	RFIC_LOG(" than init card...\n");
//	if(card_init())
//		{RFIC_LOG(" card init was error ! \n");}
//	else {RFIC_LOG(" card init OK ! \n");}
//	rt_kprintf(" \n");
	
#ifdef RC_BUS_SPI	
	rfic_unlock();
#endif
	
//	rt_thread_t rfic_test_thread= rt_thread_create("rfic_test",rfic_test_entry,RT_NULL,
//													512, 10, 5);
//	if(rfic_test_thread!= RT_NULL) rt_thread_startup(rfic_test_thread);
		
	return RT_EOK;
}
INIT_DEVICE_EXPORT(rt_hw_card_init);

/********************************************************************************************************************************************/
/********************************************************************************************************************************************/

/////////////////////////////////////////////////////////////////////
//��MF522����CRC16����
/////////////////////////////////////////////////////////////////////
static void calulate_crc_ex(unsigned char *pIndata,unsigned char len,unsigned char *pOutData)
{
    unsigned char i,n;
    clear_bit_mask_ex(DIV_IRQ_REG,0x04);
    write_raw_rc_ex(COMMAND_REG,PCD_IDLE);
    set_bit_mask_ex(FIFO_LEVEL_REG,0x80);
    for (i=0; i<len; i++)
    {   write_raw_rc_ex(FIFO_DATA_REG, *(pIndata+i));   }
    write_raw_rc_ex(COMMAND_REG, PCD_CALCCRC);
    i = 0xFF;
    do 
    {
        n = read_raw_rc_ex(DIV_IRQ_REG);
        i--;
    }
    while ((i!=0) && !(n&0x04));
    pOutData[0] = read_raw_rc_ex(CRC_RESULT_REG_L);
    pOutData[1] = read_raw_rc_ex(CRC_RESULT_REG_M);
}


/////////////////////////////////////////////////////////////////////
//��    �ܣ�ͨ��RC522��ISO14443��ͨѶ
//����˵����Command[IN]:RC522������
//          pInData[IN]:ͨ��RC522���͵���Ƭ������
//          InLenByte[IN]:�������ݵ��ֽڳ���
//          pOutData[OUT]:���յ��Ŀ�Ƭ��������
//          *pOutLenBit[OUT]:�������ݵ�λ����
/////////////////////////////////////////////////////////////////////

extern uint32_t SystemCoreClock;
static int pcd_com_mf522_ex(unsigned char Command, 
                 unsigned char *pInData, 
                 unsigned char InLenByte,
                 unsigned char *pOutData, 
                 unsigned int  *pOutLenBit)
{
    int status = MI_ERR;
    unsigned char irqEn   = 0x00;
    unsigned char waitFor = 0x00;
    unsigned char lastBits;
    unsigned char n;
    unsigned int i;
    switch (Command)
    {
       case PCD_AUTHENT:
          irqEn   = 0x12;
          waitFor = 0x10;
          break;
       case PCD_TRANSCEIVE:
          irqEn   = 0x77;
          waitFor = 0x30;
          break;
       default:
         break;
    }
   
    write_raw_rc_ex(COM_IEN_REG,irqEn|0x80);
    clear_bit_mask_ex(COM_IRQ_REG,0x80);
    write_raw_rc_ex(COMMAND_REG,PCD_IDLE);
    set_bit_mask_ex(FIFO_LEVEL_REG,0x80);
    for (i=0; i<InLenByte; i++)
    { write_raw_rc_ex(FIFO_DATA_REG, pInData[i]); }
    write_raw_rc_ex(COMMAND_REG, Command);    
    if (Command == PCD_TRANSCEIVE)
    { set_bit_mask_ex(BIT_FRAMING_REG,0x80); }
	//
	{
		unsigned long dwTime = 25000;//����M1�����ȴ�ʱ��25ms
		unsigned long dwCurCounter=0;                                //��ǰʱ�����ֵ
		unsigned long dwPreTickVal=SysTick->VAL;                     //��һ��SYSTICK����ֵ
		unsigned long dwCurTickVal;                                  //��һ��SYSTICK����ֵ
		dwTime=dwTime*(SystemCoreClock/1000000);    //����ʱʱ�䣬������ʱ�����
		for(;;){
			n = read_raw_rc_ex(COM_IRQ_REG);
			//
			dwCurTickVal=SysTick->VAL;
			if(dwCurTickVal<dwPreTickVal){
				dwCurCounter=dwCurCounter+dwPreTickVal-dwCurTickVal;
			}
			else{
				dwCurCounter=dwCurCounter+dwPreTickVal+SysTick->LOAD-dwCurTickVal;
			}
			dwPreTickVal=dwCurTickVal;
			if(dwCurCounter>=dwTime || (n&0x01) || (n&waitFor)){
				break;
			}
		}		
	}
	//
    clear_bit_mask_ex(BIT_FRAMING_REG,0x80);
    if (i!=0)
    {    
         if(!(read_raw_rc_ex(ERROR_REG)&0x1B))    ////<--------------------------------------------
         {
             status = MI_OK;
             if (n & irqEn & 0x01)
             {   status = MI_NOTAGERR;   }
             if (Command == PCD_TRANSCEIVE)
             {
               	n = read_raw_rc_ex(FIFO_LEVEL_REG);
              	lastBits = read_raw_rc_ex(CONTROL_REG) & 0x07;
                if (lastBits)
                {   *pOutLenBit = (n-1)*8 + lastBits;   }
                else
                {   *pOutLenBit = n*8;   }
                if (n == 0)
                {   n = 1;    }
                if (n > MAXRLEN)
                {   n = MAXRLEN;   }
                for (i=0; i<n; i++)
                {   pOutData[i] = read_raw_rc_ex(FIFO_DATA_REG);    }
            }
         }
         else
         {   status = MI_ERR;   }        
   }
   set_bit_mask_ex(CONTROL_REG,0x80);           // stop timer now
   write_raw_rc_ex(COMMAND_REG,PCD_IDLE); 
   return status;
}


/////////////////////////////////////////////////////////////////////
//��    �ܣ�Ѱ��
//����˵��: req_code[IN]:Ѱ����ʽ
//                0x52 = Ѱ��Ӧ�������з���14443A��׼�Ŀ�
//                0x26 = Ѱδ��������״̬�Ŀ�
//          pTagType[OUT]����Ƭ���ʹ���
//                0x4400 = Mifare_UltraLight
//                0x0400 = Mifare_One(S50)
//                0x0200 = Mifare_One(S70)
//                0x0800 = Mifare_Pro(X)
//                0x4403 = Mifare_DESFire
//��    ��: �ɹ�����MI_OK
/////////////////////////////////////////////////////////////////////
int pcd_request_ex(unsigned char req_code,unsigned char *pTagType)
{
   int status;  
   unsigned int  unLen;
    
//  unsigned char xTest ;
   clear_bit_mask_ex(STATUS2_REG,0x08);
   write_raw_rc_ex(BIT_FRAMING_REG,0x07);
//  xTest = read_raw_rc_ex(BIT_FRAMING_REG);
//  if(xTest == 0x07 )
 //   { LED_GREEN  =0 ;}
 // else {LED_GREEN =1 ;while(1){}}
   set_bit_mask_ex(TX_CONTROL_REG,0x03);
   com_mf522_buf[0] = req_code;

   status = pcd_com_mf522_ex(PCD_TRANSCEIVE,com_mf522_buf,1,com_mf522_buf,&unLen);
//     if(status  == MI_OK )
//   { LED_GREEN  =0 ;}
//   else {LED_GREEN =1 ;}
   if ((status == MI_OK) && (unLen == 0x10))
   {    
       *pTagType     = com_mf522_buf[0];
       *(pTagType+1) = com_mf522_buf[1];
   }
   else
   {   status = MI_ERR;   }
   
   return status;
}

/////////////////////////////////////////////////////////////////////
//��    �ܣ�����ײ
//����˵��: pSnr[OUT]:��Ƭ���кţ�4�ֽ�
//��    ��: �ɹ�����MI_OK
/////////////////////////////////////////////////////////////////////  
int pcd_anticoll_ex(unsigned char *pSnr)
{
    int status;
    unsigned char i,snr_check=0;
    unsigned int  unLen;

    clear_bit_mask_ex(STATUS2_REG,0x08);
    write_raw_rc_ex(BIT_FRAMING_REG,0x00);
    clear_bit_mask_ex(COLL_REG,0x80);
 
    com_mf522_buf[0] = PICC_ANTICOLL1;
    com_mf522_buf[1] = 0x20;

    status = pcd_com_mf522_ex(PCD_TRANSCEIVE,com_mf522_buf,2,com_mf522_buf,&unLen);

    if (status == MI_OK)
    {
    	 for (i=0; i<4; i++)
         {   
             *(pSnr+i)  = com_mf522_buf[i];
             snr_check ^= com_mf522_buf[i];
         }
         if (snr_check != com_mf522_buf[i])
         {   status = MI_ERR;    }
    }
    
    set_bit_mask_ex(COLL_REG,0x80);
    return status;
}

/////////////////////////////////////////////////////////////////////
//��    �ܣ�ѡ����Ƭ
//����˵��: pSnr[IN]:��Ƭ���кţ�4�ֽ�
//��    ��: �ɹ�����MI_OK
/////////////////////////////////////////////////////////////////////
int pcd_select_ex(unsigned char *pSnr)
{
    int status;
    unsigned char i;
    unsigned int  unLen;     
    
    com_mf522_buf[0] = PICC_ANTICOLL1;
    com_mf522_buf[1] = 0x70;
    com_mf522_buf[6] = 0;
    for (i=0; i<4; i++)
    {
    	com_mf522_buf[i+2] = *(pSnr+i);
    	com_mf522_buf[6]  ^= *(pSnr+i);
    }
    calulate_crc_ex(com_mf522_buf,7,&com_mf522_buf[7]);
  
    clear_bit_mask_ex(STATUS2_REG,0x08);

    status = pcd_com_mf522_ex(PCD_TRANSCEIVE,com_mf522_buf,9,com_mf522_buf,&unLen);
    
    if ((status == MI_OK) && (unLen == 0x18))
    {   status = MI_OK;  }
    else
    {   status = MI_ERR;    }

    return status;
}

/////////////////////////////////////////////////////////////////////
//��    �ܣ���֤��Ƭ����
//����˵��: auth_mode[IN]: ������֤ģʽ
//                 0x60 = ��֤A��Կ
//                 0x61 = ��֤B��Կ 
//          addr[IN]�����ַ
//          pKey[IN]������
//          pSnr[IN]����Ƭ���кţ�4�ֽ�
//��    ��: �ɹ�����MI_OK
/////////////////////////////////////////////////////////////////////               
int pcd_auth_state_ex(unsigned char auth_mode,unsigned char addr,unsigned char *pKey,unsigned char *pSnr)
{
    int status;
    unsigned int  unLen;
    unsigned char i; 

    com_mf522_buf[0] = auth_mode;
    com_mf522_buf[1] = addr;
    for (i=0; i<6; i++)
    {    com_mf522_buf[i+2] = *(pKey+i);   }
    for (i=0; i<6; i++)
    {    com_mf522_buf[i+8] = *(pSnr+i);   }
    
    status = pcd_com_mf522_ex(PCD_AUTHENT,com_mf522_buf,12,com_mf522_buf,&unLen);
    if ((status != MI_OK) || (!(read_raw_rc_ex(STATUS2_REG) & 0x08)))
    {   status = MI_ERR;   }
    
    return status;
}

/////////////////////////////////////////////////////////////////////
//��    �ܣ���ȡM1��һ������
//����˵��: addr[IN]�����ַ
//          pData[OUT]�����������ݣ�16�ֽ�
//��    ��: �ɹ�����MI_OK
///////////////////////////////////////////////////////////////////// 
int pcd_read_ex(unsigned char addr,unsigned char *pData)
{
    int status;
    unsigned int  unLen;
    unsigned char i; 

    com_mf522_buf[0] = PICC_READ;
    com_mf522_buf[1] = addr;
    calulate_crc_ex(com_mf522_buf,2,&com_mf522_buf[2]);
   
    status = pcd_com_mf522_ex(PCD_TRANSCEIVE,com_mf522_buf,4,com_mf522_buf,&unLen);
    if ((status == MI_OK) && (unLen == 0x90))
    {
        for (i=0; i<16; i++)
        {    *(pData+i) = com_mf522_buf[i];   }
    }
    else
    {   status = MI_ERR;   }
    
    return status;
}

/////////////////////////////////////////////////////////////////////
//��    �ܣ�д���ݵ�M1��һ��
//����˵��: addr[IN]�����ַ
//          pData[IN]��д������ݣ�16�ֽ�
//��    ��: �ɹ�����MI_OK
/////////////////////////////////////////////////////////////////////                  
int pcd_write_ex(unsigned char addr,unsigned char *pData)
{
    int status;
    unsigned int  unLen;
    unsigned char i; 
    
    com_mf522_buf[0] = PICC_WRITE;
    com_mf522_buf[1] = addr;
    calulate_crc_ex(com_mf522_buf,2,&com_mf522_buf[2]);
 
    status = pcd_com_mf522_ex(PCD_TRANSCEIVE,com_mf522_buf,4,com_mf522_buf,&unLen);

    if ((status != MI_OK) || (unLen != 4) || ((com_mf522_buf[0] & 0x0F) != 0x0A))
    {   status = MI_ERR;   }
        
    if (status == MI_OK)
    {
        memcpy(com_mf522_buf, pData, 16);
        for (i=0; i<16; i++)
        {    com_mf522_buf[i] = *(pData+i);   }
        calulate_crc_ex(com_mf522_buf,16,&com_mf522_buf[16]);

        status = pcd_com_mf522_ex(PCD_TRANSCEIVE,com_mf522_buf,18,com_mf522_buf,&unLen);
        if ((status != MI_OK) || (unLen != 4) || ((com_mf522_buf[0] & 0x0F) != 0x0A))
        {   status = MI_ERR;   }
    }
    
    return status;
}



