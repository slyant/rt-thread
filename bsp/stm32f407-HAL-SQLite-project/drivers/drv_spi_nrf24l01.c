#include <drv_spi_nrf24l01.h>
#include <rtthread.h>
#include <board.h>
#include <drv_spi.h>
#include <drivers/spi.h>
#include <drivers/pin.h>

#define ENABLE_LOG	1
#define NRF_LOG		if(ENABLE_LOG)rt_kprintf("[NRF24L01]:");rt_kprintf
#define NRF_LOGS	if(ENABLE_LOG)rt_kprintf
#define NRF_CE(v)	rt_pin_write(NRF_CE_PIN, v)
static struct rt_spi_device *nrf_spi_device = RT_NULL;
static nrf_recv_handle nrf_recv = RT_NULL;
static rt_mutex_t nrf_mutex_lock = RT_NULL;
static rt_event_t nrf_send_event = RT_NULL;
#define NRF_DEFAULT_SPI_CFG                 			\
{                                               		\
    .mode = RT_SPI_MASTER | RT_SPI_MODE_0 | RT_SPI_MSB,	\
    .data_width = 8,                            		\
    .max_hz = 8 * 1000 * 1000,                 		\
}
static void NRF_DelayUs(u32 us)
{
//	extern void delay_us(rt_uint32_t nus);
//	delay_us(i);
    int i = (HAL_RCC_GetHCLKFreq() / 4000000 * us);
    while (i)
    {
        i--;
    }
}
static void nrf_irq_handle(void* args)
{
	rt_event_send(nrf_send_event, NRF_EVENT);
}
/*
ע��NRF���ݽ��մ�����
*/
void nrf_reg_recv_handler(nrf_recv_handle recv)
{
	nrf_recv = recv;
	rt_pin_attach_irq(NRF_IRQ_PIN, PIN_IRQ_MODE_FALLING, nrf_irq_handle, RT_NULL);
	rt_pin_irq_enable(NRF_IRQ_PIN, PIN_IRQ_ENABLE);
}
static void nrf_lock(void)
{	
    rt_mutex_take(nrf_mutex_lock, RT_WAITING_FOREVER);
}

static void nrf_unlock(void)
{
	rt_mutex_release(nrf_mutex_lock);
}
//��ȡSPI�Ĵ���ֵ
static u8 NRF_ReadReg(const u8 reg)
{
	u8 reg_val;
	reg_val = rt_spi_sendrecv8(nrf_spi_device, R_REGISTER|reg);
	return(reg_val);
}
//SPIд�Ĵ���
static u8 NRF_WriteReg(const u8 reg, const  u8 value)
{
	u8 status,t;
	t = W_REGISTER|reg;
	status = rt_spi_send_then_send(nrf_spi_device, &t, 1, &value, 1);
	return(status);
}
//��ָ��λ�ö���ָ�����ȵ�����
//���ض�����״̬�Ĵ���ֵ 
static u8 NRF_ReadBuf(const u8 reg, u8 *pBuf, const u8 len)
{
	u8 status;
	status= rt_spi_sendrecv8(nrf_spi_device, (R_REGISTER|reg));
	rt_spi_recv(nrf_spi_device, pBuf, len);
	return status;
}
//��ָ��λ��дָ�����ȵ�����
//���ض�����״̬�Ĵ���ֵ
static u8 NRF_WriteBuf(const u8 reg,const u8 *pBuf,const u8 len)
{
	u8 status;
	status= rt_spi_sendrecv8(nrf_spi_device, (W_REGISTER|reg));
	rt_spi_send(nrf_spi_device, pBuf, len);
	return status;
}
static void NRF_PowerDown(void)
{
	NRF_CE(0);
	NRF_WriteReg(CONFIG, 0x79);//RX_DR,TX_DS,MAX_RT�ж�����,CRC:8,POWER_DOWN,����ģʽ
}
u8 NRF_Check(void)
{
	u8 buf[5]={0X95,0X94,0X93,0X92,0X91};
	u8 i;
	nrf_lock();
	NRF_WriteBuf(TX_ADDR,buf,5);//д��5���ֽڵĵ�ַ.	
	NRF_ReadBuf(TX_ADDR,buf,5); //����д��ĵ�ַ 
	nrf_unlock();	
	for(i=0;i<5;i++)
	{
		if(buf[i]!=0X95-i)
			break;	
	}		
	if(i!=5)
		return 0;	//���24L01����	
	return 1;		//��⵽24L01
}
u8 NRF_CheckReg(void)
{
	u8 res = 1;
	if(NRF_ReadReg(CONFIG)!=0x0b) 
		res = 0;
	if(NRF_ReadReg(EN_AA)!=0x01) 
		res = 0;
	if(NRF_ReadReg(EN_RXADDR)!=0x03) 
		res = 0;
	if(NRF_ReadReg(SETUP_AW)!=0x03) 
		res = 0;
	if(NRF_ReadReg(SETUP_RETR)!=0x71) 
		res = 0;
	if(NRF_ReadReg(RF_SETUP)!=0x27) 
		res = 0;
	if(NRF_ReadReg(STATUS)!=0x0e) 
		res = 0;
	if(NRF_ReadReg(FIFO_STATUS)!=0x11) 
		res = 0;
	return res;
}
u8 NRF_SetRfPower(const u8 level)//(0-3)
{
	u8 state = 0,power;	
	if(level<4)
	{
		nrf_lock();
		NRF_CE(0);
		power=level;
		state=NRF_ReadReg(RF_SETUP);//������
		state&=0XF9;//��powerλ(B11111001)
		power&=0x03;//����power(B00000011)
		NRF_WriteReg(RF_SETUP, state|(power<<1));//����powerλ
		nrf_unlock();
	}
	return state;
}
u8 NRF_ReadChannel(void)
{
	nrf_lock();
	rt_uint8_t res;
	res = NRF_ReadReg(RF_CH);//��Ƶ��
	nrf_unlock();
	return res;
}
void NRF_SetMac(u8 *addr)
{
	nrf_lock();
	NRF_CE(0);
	NRF_WriteBuf(RX_ADDR_P1, addr, MAC_LEN);//дͨ��1,RX��ַ
	NRF_WriteReg(CONFIG, 0x0B);//����ģʽ
	NRF_CE(1);
	nrf_unlock();
}
void NRF_SetChannel(const u8 channel)
{
	nrf_lock();
	if(channel<126)
	{
		//NRF_CE = 0;
		NRF_WriteReg(RF_CH, channel);//����RFͨ��Ƶ��
	}
	nrf_unlock();
}
//��⵱ǰƵ����ⲿ�����ź�ǿ��
u8 NRF_DisturbanceDetect(void)
{
	rt_uint8_t res;
	nrf_lock();
	NRF_CE(0);
	NRF_DelayUs(1);
	NRF_WriteReg(CONFIG, 0x0B);//RCRC:8,����ģʽ
	NRF_CE(1);
	NRF_DelayUs(200);//����ģʽ->����ģʽ(130us)+40us	
	res = NRF_ReadReg(CD);
	nrf_unlock();
	return res;
}

//��������
u8 NRF_SendPayload(const u8 ack,const u8 *addr,const u8 channel,const u8 *buf)
{
	u8 k;
	rt_uint32_t e;
	nrf_lock();	
	NRF_LOGS("MAC:");
	for(k=0;k<MAC_LEN;k++)
	{
		NRF_LOGS("%02X ",addr[k]);
	}
	NRF_LOGS("\r\n");	
	NRF_CE(0);
	NRF_WriteBuf(TX_ADDR, addr, MAC_LEN);//дTX�ڵ��ַ
	if(ack)
	{
		NRF_WriteBuf(RX_ADDR_P0, addr, MAC_LEN);//дͨ��0,RX��ַ
	}
	NRF_WriteReg(RF_CH, channel);//����RFͨ��Ƶ��
	NRF_WriteReg(STATUS, IRQ_ALL);//���״̬��־
	NRF_ReadReg(FLUSH_RX);//�����FIFO
	NRF_ReadReg(FLUSH_TX);//�巢��FIFO
	NRF_WriteBuf(W_TX_PAYLOAD, buf, PAYLOAD_LEN);
	NRF_WriteReg(CONFIG, 0x0A);//RX_DR,TX_DS,MAX_RT�ж�ʹ��,CRC:8,POWER_UP,����ģʽ
	NRF_CE(1);//����CEΪ����������,CE�ߵ�ƽ����ʱ����СΪ10us
	nrf_unlock();
	if (rt_event_recv(nrf_send_event, NRF_SEND_FINISH|NRF_SEND_MAXCOUNT,
            RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR,
            rt_tick_from_millisecond(5), &e) == RT_EOK)
	{
		if(e&NRF_SEND_FINISH)
		{
			return RT_EOK;
		}
		else if(e&NRF_SEND_MAXCOUNT)
		{
			return RT_ETIMEOUT;
		}
	}
	nrf_lock();
	NRF_ReadReg(FLUSH_TX);//�巢��FIFO
	NRF_WriteReg(STATUS, IRQ_ALL);//���״̬��־
	nrf_unlock();
	return RT_ERROR;
}
//����ģʽ
static void NRF_RxMode(const u8 *addr,const u8 channel)
{
	NRF_CE(0);
	NRF_WriteBuf(RX_ADDR_P0, addr, MAC_LEN);//дRX�ڵ��ַ
	NRF_WriteReg(RF_CH, channel);//����RFͨ��Ƶ��
	NRF_ReadReg(FLUSH_RX);//�����FIFO
	NRF_WriteReg(CONFIG, 0x0B);//RX_DR,TX_DS,MAX_RT�ж�ʹ��,CRC:8,POWER_UP,����ģʽ
	NRF_CE(1);
}
void NRF_Config(const u8 *addr,const u8 channel)
{
	nrf_lock();
	NRF_CE(0);
	NRF_PowerDown();//����ģʽ
	NRF_WriteReg(EN_AA, 0x01);//ʹ��ͨ��0�Զ�Ӧ��
//	NRF_WriteReg(EN_RXADDR, 0x3f);//ʹ������ͨ���Ľ��յ�ַ 
	NRF_WriteReg(EN_RXADDR, 0x03);//ʹ��0,1�Ľ��յ�ַ 
	NRF_WriteReg(SETUP_RETR, 0x71);//�ȴ�2000us,�ط�1��(250Kbps=2000us)
	NRF_WriteReg(SETUP_AW, 0x03);//��ַ���5
	NRF_WriteReg(RF_SETUP, 0x27);//����TX�������,0db����,250Kbps,���������濪��
//	NRF_WriteReg(RF_SETUP, 0x21);//����TX�������,250Kbps,-18db����,���������濪��
	NRF_WriteReg(STATUS, IRQ_ALL);//���״̬��־
	NRF_ReadReg(FLUSH_RX);//�����FIFO
	NRF_ReadReg(FLUSH_TX);//�巢��FIFO
	if((!NRF_ReadReg(FEATRUE))&&(!NRF_ReadReg(DYNPD)))
	{
		u8 a,b;
		a = ACTIVATE; b = 0x73;
		rt_spi_send_then_send(nrf_spi_device, &a, 1,&b, 1);//����R_RX_PL_WID,W_ACK_PAYLOAD,W_TX_PAYLOAD_NOACK
	}
	NRF_WriteReg(DYNPD, 0x3f);//����ͨ��ʹ�ܶ�̬����
	NRF_WriteReg(FEATRUE, 0x06);//ʹ�ܶ�̬����,ʹ������Ӧ��
	NRF_WriteBuf(RX_ADDR_P0, addr, MAC_LEN);//дͨ��0,RX��ַ
	NRF_DelayUs(2000);//����ģʽ->����ģʽǰ���뾭��1.5ms�Ĵ���ģʽ
	NRF_RxMode(addr, channel);//��ʼ��Ϊ����ģʽ
	NRF_DelayUs(150);//����ģʽ->����/����ģʽ130us
	nrf_unlock();
}
static void nrf_thread_entry(void* param)
{
	rt_uint32_t e;
	while(1)
	{		
		if (rt_event_recv(nrf_send_event, NRF_EVENT,
            RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR,
            RT_WAITING_FOREVER, &e) == RT_EOK)
		{
			u8 status, pipe_num;	
			nrf_lock();
			status = NRF_ReadReg(STATUS);//��״̬�Ĵ���,�ж��������ж�״̬
			if(status & RX_DR)//���յ�����
			{
				if(nrf_recv)
				{
					u8 rxdata[PAYLOAD_LEN];
					NRF_CE(0);
					rt_memset(rxdata, 0, PAYLOAD_LEN);
					status=NRF_ReadReg(STATUS);//��ȡstatus�Ĵ�����ֵ
					pipe_num=(status&RX_P_NO_MASK)>>1;//��ȡͨ����(0-5)
					NRF_ReadBuf(R_RX_PAYLOAD, rxdata, PAYLOAD_LEN);//������յ�������
					nrf_recv(pipe_num, rxdata);
				}
			}
			if(status & TX_DS)//�������
			{
				rt_event_send(nrf_send_event, NRF_SEND_FINISH);
			}
			if(status & MAX_RT)//����ط�����
			{
				NRF_ReadReg(FLUSH_TX);//�巢��FIFO
				rt_event_send(nrf_send_event, NRF_SEND_MAXCOUNT);		
			}
			NRF_WriteReg(STATUS, status);//���״̬��־	
			nrf_unlock();
		}		
	}
}
static int rt_hw_spi_nrf24l01_init(void)
{
	rt_pin_mode(NRF_CE_PIN, PIN_MODE_OUTPUT);
	rt_pin_write(NRF_CE_PIN, 0);
	rt_pin_mode(NRF_IRQ_PIN, PIN_MODE_INPUT_PULLUP);	
	stm32_spi_bus_attach_device(NRF_SPI_CS_PIN, NRF_SPI_BUS_NAME, NRF_SPI_DEV_NAME);
	nrf_spi_device = (struct rt_spi_device *)rt_device_find(NRF_SPI_DEV_NAME);
	if(nrf_spi_device==RT_NULL)
	{
		NRF_LOG("Not find the spi device:%s\n", NRF_SPI_DEV_NAME);
		return RT_ERROR;
	}
	else
	{
		struct rt_spi_configuration cfg = NRF_DEFAULT_SPI_CFG;
		rt_spi_configure(nrf_spi_device, &cfg);		
		NRF_LOG("%s Device Init OK.\n", NRF_SPI_DEV_NAME);
		
		nrf_mutex_lock = rt_mutex_create("nrf_lock", RT_IPC_FLAG_FIFO);
		RT_ASSERT(nrf_mutex_lock != RT_NULL);
		nrf_send_event = rt_event_create("nrf_event", RT_IPC_FLAG_FIFO);
		RT_ASSERT(nrf_send_event != RT_NULL);
		
		rt_thread_t nrf_thread = rt_thread_create("nrf_thread", nrf_thread_entry, RT_NULL,
													512, 10, 5);
		if (nrf_thread != RT_NULL) rt_thread_startup(nrf_thread);
		return RT_EOK;
	}
}
INIT_DEVICE_EXPORT(rt_hw_spi_nrf24l01_init);

