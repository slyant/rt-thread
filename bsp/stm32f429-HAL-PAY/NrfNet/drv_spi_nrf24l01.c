#include <drv_spi_nrf24l01.h>
#include <rtthread.h>
#include <board.h>
#include <drv_spi.h>
#include <drivers/spi.h>
#include <drivers/pin.h>

//#define USING_NRF_LOG
#ifdef USING_NRF_LOG
	#define NRF_LOG		rt_kprintf("[NRF_DRV]:");rt_kprintf
	#define NRF_LOGS	rt_kprintf
#else
	#define NRF_LOG(...)
	#define NRF_LOGS(...)
#endif
	
#define NRF_CE(v)	rt_pin_write(NRF_CE_PIN, v)
static struct rt_spi_device *nrf_spi_device = RT_NULL;
static nrf_hw_recv_callback_t nrf_hw_recv_callback = RT_NULL;
static rt_mutex_t nrf_device_mutex_lock = RT_NULL;
static rt_event_t nrf_event = RT_NULL;
#define NRF_DEFAULT_SPI_CFG                 			\
{                                               		\
    .mode = RT_SPI_MODE_0 | RT_SPI_MSB,	\
    .data_width = 8,                            		\
    .max_hz = 11 * 1000 * 1000,                 		\
}
static void NRF_DelayUs(u32 nus)
{
	extern void delay_us(rt_uint32_t nus);
	delay_us(nus);
}
static void nrf_irq_handle(void* args)
{
	rt_event_send(nrf_event, NRF_EVENT);
}
/*
ע��NRF���ݽ��մ�����
*/
void nrf_reg_hw_recv_callback(nrf_hw_recv_callback_t recv)
{
	nrf_hw_recv_callback = recv;
	rt_pin_attach_irq(NRF_IRQ_PIN, PIN_IRQ_MODE_FALLING, nrf_irq_handle, RT_NULL);
	rt_pin_irq_enable(NRF_IRQ_PIN, PIN_IRQ_ENABLE);
}
static void nrf_lock(void)
{	
    rt_mutex_take(nrf_device_mutex_lock, RT_WAITING_FOREVER);
}

static void nrf_unlock(void)
{
	rt_mutex_release(nrf_device_mutex_lock);
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
	status = rt_spi_send_then_recv(nrf_spi_device, &reg, 1, pBuf, len);
	return status;
}
//��ָ��λ��дָ�����ȵ�����
//���ض�����״̬�Ĵ���ֵ
static u8 NRF_WriteBuf(const u8 reg,const u8 *pBuf,const u8 len)
{
	u8 status;
	u8 reg2 = W_REGISTER|reg;
	status = rt_spi_send_then_send(nrf_spi_device, &reg2, 1, pBuf, len);
	return status;
}
static void NRF_PowerDown(void)
{
	NRF_CE(0);
	NRF_WriteReg(CONFIG, 0x7D);//0111 1101	0,RX_DR,TX_DS,MAX_RT,Enable CRC,CRC(0:8|1:16),POWER_UP(1)|POWER_DOWN(0),PRX(1)|PTX(0)
	NRF_CE(1);
	NRF_DelayUs(20);
}
u8 NRF_Check(void)
{
	u8 buf[5]={0X95,0X94,0X93,0X92,0X91};
	u8 i;
	nrf_lock();
	NRF_CE(0);
	NRF_WriteBuf(TX_ADDR,buf,5);//д��5���ֽڵĵ�ַ.	
	NRF_ReadBuf(TX_ADDR,buf,5); //����д��ĵ�ַ 
	NRF_CE(1);
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
		NRF_CE(1);
		nrf_unlock();
	}
	return state;
}
u8 NRF_ReadChannel(void)
{
	rt_uint8_t res;
	nrf_lock();
	NRF_CE(0);
	res = NRF_ReadReg(RF_CH);//��Ƶ��
	NRF_CE(1);
	nrf_unlock();
	return res;
}
void NRF_SetPipe1Mac(u8 *addr)
{
	nrf_lock();
	NRF_CE(0);
	NRF_WriteBuf(RX_ADDR_P1, addr, MAC_LEN);//дͨ��1,RX��ַ
	NRF_WriteReg(CONFIG, 0x0f);
	NRF_CE(1);
	nrf_unlock();
}
void NRF_SetPipe2Mac(u8 *addr)
{
	nrf_lock();
	NRF_CE(0);
	NRF_WriteReg(RX_ADDR_P2, addr[0]);//дͨ��2,RX��ַ
	NRF_WriteReg(CONFIG, 0x0f);
	NRF_CE(1);
	nrf_unlock();
}
void NRF_SetChannel(const u8 channel)
{
	nrf_lock();
	
	if(channel<126)
	{
		NRF_CE(0);
		NRF_WriteReg(RF_CH, channel);//����RFͨ��Ƶ��
		NRF_CE(0);
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
	NRF_WriteReg(CONFIG, 0x0f);
	NRF_CE(1);
	NRF_DelayUs(200);//����ģʽ->����ģʽ(130us)+40us	
	res = NRF_ReadReg(CD);
	nrf_unlock();
	return res;
}

//����ģʽ
//��������
u8 NRF_SendPayload(const u8 ack,const u8 *addr,const u8 channel,const u8 *buf)
{
	rt_uint32_t e;
	nrf_lock();
	NRF_CE(0);
	NRF_WriteBuf(TX_ADDR, addr, MAC_LEN);//дTX�ڵ��ַ
	if(ack)
	{
		NRF_WriteBuf(RX_ADDR_P0, addr, MAC_LEN);//дͨ��0,RX��ַ
		NRF_WriteReg(EN_AA, 0x01);//ʹ��ͨ��0�Զ�Ӧ��
		NRF_WriteReg(EN_RXADDR, 0x07);//ʹ��0,1,2�Ľ��յ�ַ
		NRF_WriteReg(SETUP_RETR, 0xfa);//�ȴ�4000us,�ط�10��
	}
	else
	{
		NRF_WriteReg(EN_AA, 0x00);//����ͨ��0�Զ�Ӧ��	
	}
	NRF_WriteReg(RF_CH, channel);//����RFͨ��Ƶ��
	NRF_WriteReg(RF_SETUP, 0x0e);//����TX�������
	NRF_WriteBuf(W_TX_PAYLOAD, buf, PAYLOAD_LEN);
	NRF_WriteReg(CONFIG, 0x0e);//0000 1110	0,RX_DR,TX_DS,MAX_RT,Enable CRC,CRC(0:8|1:16),POWER_UP(1)|POWER_DOWN(0),PRX(1)|PTX(0)
	NRF_CE(1);//����CEΪ����������,CE�ߵ�ƽ����ʱ����СΪ10us
	nrf_unlock();
	if (rt_event_recv(nrf_event, NRF_SEND_FINISH|NRF_SEND_MAXCOUNT,
            RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR,
            rt_tick_from_millisecond(80), &e) == RT_EOK)
	{
		if(e&NRF_SEND_FINISH)
		{
			NRF_LOG("NRF Send Success!\n");
			return RT_EOK;
		}
		else if(e&NRF_SEND_MAXCOUNT)
		{
			NRF_LOG("NRF Send Over Maxcount!\n");
			return RT_ETIMEOUT;
		}
	}
	nrf_lock();
	NRF_WriteReg(STATUS, 0xff);//���״̬��־
	nrf_unlock();
	NRF_LOG("NRF Send Error!\n");
	return RT_ERROR;
}

void NRF_RxMode(const u8 *addr,const u8 channel)
{
	nrf_lock();
	NRF_CE(0);
	NRF_WriteBuf(RX_ADDR_P0, addr, MAC_LEN);//дͨ��0,RX��ַ
	NRF_WriteReg(EN_AA, 0x01);//ʹ��ͨ��0�Զ�Ӧ��
#ifdef USING_NRF24L01_NODE
	NRF_WriteReg(EN_RXADDR, 0x07);//ʹ��0,1,2�Ľ��յ�ַ
	NRF_WriteReg(RX_PW_P0, PAYLOAD_LEN);
	NRF_WriteReg(RX_PW_P1, PAYLOAD_LEN);
	NRF_WriteReg(RX_PW_P2, PAYLOAD_LEN);
#else
	NRF_WriteReg(EN_RXADDR, 0x03);//ʹ��0,1�Ľ��յ�ַ
	NRF_WriteReg(RX_PW_P0, PAYLOAD_LEN);
	NRF_WriteReg(RX_PW_P1, PAYLOAD_LEN);
#endif
	NRF_WriteReg(RF_CH, channel);//����RFͨ��Ƶ��
	NRF_WriteReg(RF_SETUP, 0x0e);//����TX�������
	NRF_WriteReg(CONFIG, 0x0f);//0000 1111	0,RX_DR,TX_DS,MAX_RT,Enable CRC,CRC(0:8|1:16),POWER_UP(1)|POWER_DOWN(0),PRX(1)|PTX(0)
	NRF_CE(1);
	nrf_unlock();
}
void NRF_Config(const u8 *addr,const u8 channel)
{
	nrf_lock();
	NRF_PowerDown();//����ģʽ
	NRF_CE(0);
	NRF_WriteBuf(RX_ADDR_P0, addr, MAC_LEN);//дͨ��0,RX��ַ
	NRF_WriteReg(EN_AA, 0x01);//ʹ��ͨ��0�Զ�Ӧ��
#ifdef USING_NRF24L01_NODE
	NRF_WriteReg(EN_RXADDR, 0x07);//ʹ��0,1,2�Ľ��յ�ַ
	NRF_WriteReg(RX_PW_P0, PAYLOAD_LEN);
	NRF_WriteReg(RX_PW_P1, PAYLOAD_LEN);
	NRF_WriteReg(RX_PW_P2, PAYLOAD_LEN);
#else
	NRF_WriteReg(EN_RXADDR, 0x03);//ʹ��0,1�Ľ��յ�ַ
	NRF_WriteReg(RX_PW_P0, PAYLOAD_LEN);
	NRF_WriteReg(RX_PW_P1, PAYLOAD_LEN);
#endif
	NRF_WriteReg(RF_CH, channel);//����RFͨ��Ƶ��
	NRF_WriteReg(RF_SETUP, 0x0e);//����TX�������
	NRF_WriteReg(CONFIG, 0x0f);//0000 1111	0,RX_DR,TX_DS,MAX_RT,Enable CRC,CRC(0:8|1:16),POWER_UP(1)|POWER_DOWN(0),PRX(1)|PTX(0)
	NRF_WriteReg(STATUS, 0xff);//����жϱ��
	NRF_WriteReg(FLUSH_RX,0xff);//�����FIFO	
	NRF_CE(1);
	nrf_unlock();
	NRF_DelayUs(2000);
}

static void nrf_hw_recv_thread_entry(void* param)
{
	rt_uint32_t e;
	while(1)
	{		
		if (rt_event_recv(nrf_event, NRF_EVENT,
            RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR,
            RT_WAITING_FOREVER, &e) == RT_EOK)
		{
			u8 status, pipe_num;	
			nrf_lock();
			NRF_CE(0);
			status = NRF_ReadReg(STATUS);//��״̬�Ĵ���,�ж��������ж�״̬
			NRF_WriteReg(STATUS, status);//����жϱ��			
			if(status & RX_DR)//���յ�����
			{
				if(nrf_hw_recv_callback)
				{
					u8 rxdata[PAYLOAD_LEN];
					rt_memset(rxdata, 0, PAYLOAD_LEN);
					pipe_num=(status&RX_P_NO_MASK)>>1;//��ȡͨ����(0-5)
					NRF_ReadBuf(R_RX_PAYLOAD, rxdata, PAYLOAD_LEN);//������յ�������
					NRF_WriteReg(FLUSH_RX,0xff);//�����FIFO
					nrf_unlock();
					nrf_hw_recv_callback(pipe_num, rxdata, PAYLOAD_LEN);
					nrf_lock();
				}
			}
			if(status & TX_DS)//�������
			{			
				rt_event_send(nrf_event, NRF_SEND_FINISH);
			}
			if(status & MAX_RT)//����ط�����
			{
				NRF_WriteReg(FLUSH_TX,0xff);//�巢��FIFO
				rt_event_send(nrf_event, NRF_SEND_MAXCOUNT);		
			}	
			NRF_CE(1);			
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
		NRF_LOG("spi device %s not found!\n", NRF_SPI_DEV_NAME);
		return RT_ERROR;
	}
	else
	{
		struct rt_spi_configuration cfg = NRF_DEFAULT_SPI_CFG;
		rt_spi_configure(nrf_spi_device, &cfg);		
		NRF_LOG("%s Device Init OK.\n", NRF_SPI_DEV_NAME);
		
		nrf_device_mutex_lock = rt_mutex_create("nrf_lock", RT_IPC_FLAG_FIFO);
		RT_ASSERT(nrf_device_mutex_lock != RT_NULL);
		
		nrf_event = rt_event_create("nrf_event", RT_IPC_FLAG_FIFO);
		RT_ASSERT(nrf_event != RT_NULL);
		
		rt_thread_t nrf_thread = rt_thread_create("nrf_recv", nrf_hw_recv_thread_entry, RT_NULL,
													512, 10, 5);
		if (nrf_thread != RT_NULL) rt_thread_startup(nrf_thread);
		return RT_EOK;
	}
}
INIT_DEVICE_EXPORT(rt_hw_spi_nrf24l01_init);
