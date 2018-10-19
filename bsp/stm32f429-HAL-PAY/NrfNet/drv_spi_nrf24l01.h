#ifndef __DRV_SPI_NRF24L01_H__
#include <rtthread.h>

#define u8	uint8_t
#define u16 uint16_t
#define u32 uint32_t

//NRF24L01�Ĵ����������� //SPI commands for nRF24L01
#define R_REGISTER         0x00  //�����üĴ���,��5λΪ�Ĵ�����ַ
#define W_REGISTER         0x20  //д���üĴ���,��5λΪ�Ĵ�����ַ
#define R_RX_PAYLOAD       0x61  //��RX��Ч����,1~32�ֽ�
#define W_TX_PAYLOAD       0xA0  //дTX��Ч����,1~32�ֽ�
#define FLUSH_TX           0xE1  //���TX FIFO�Ĵ���.����ģʽ����
#define FLUSH_RX           0xE2  //���RX FIFO�Ĵ���.����ģʽ����
#define REUSE_TX_PL        0xE3  //����ʹ����һ������,CEΪ��,���ݰ������Ϸ���
#define ACTIVATE	       0x50  //������������0x73�������R_RX_PL_WID��W_ACK_PAYLOAD��W_TX_PAYLOAD_NOACK
								 //�ٴ�ʹ�ø�����ͬ�������ݣ����ر���������
#define R_RX_PL_WID        0x60  //���յ������ݳ���
#define W_ACK_PAYLOAD      0xA8  //дӦ������
#define W_TX_PAYLOAD_NOACK 0xB0  //дTX����,��Ӧ��
#define NOP                0xFF  //�ղ���,����������״̬�Ĵ���

//NRF24L01�Ĵ�����ַ
#define CONFIG        0x00  //���üĴ���;bit0:1����ģʽ,0����ģʽ;bit1:��ѡ��;bit2:CRCģʽ;bit3:CRCʹ��;
                            //bit4:�ж�MAX_RT(����ط������ж�)ʹ��;bit5:�ж�TX_DSʹ��;bit6:�ж�RX_DRʹ��
#define EN_AA         0x01  //ʹ���Զ�Ӧ��;bit0~5,��Ӧͨ��0~5
#define EN_RXADDR     0x02  //���յ�ַ����;bit0~5,��Ӧͨ��0~5
#define SETUP_AW      0x03  //���õ�ַ���(��������ͨ��);bit1:0;00,3�ֽ�;01,4�ֽ�;02,5�ֽ�;
#define SETUP_RETR    0x04  //�����Զ��ط�;bit3:0;�Զ��ط�������;bit7:4,�Զ��ط���ʱ 250*x+86us
#define RF_CH         0x05  //RFͨ��;bit6:0;����ͨ��Ƶ��;
#define RF_SETUP      0x06  //RF�Ĵ���;bit3:��������(0:1Mbps,1:2Mbps);bit2:1,���书��;bit0:�������Ŵ�������
#define STATUS        0x07  //״̬�Ĵ���;bit0:TX FIFO����־;bit3:1,��������ͨ����(���:6);bit4,�ﵽ�����ط�
                            //bit5:���ݷ�������ж�;bit6:���������ж�;
#define OBSERVE_TX    0x08  //���ͼ��Ĵ���,bit7:4,���ݰ���ʧ������;bit3:0,�ط�������
#define CD            0x09  //�ز����Ĵ���,bit0,�ز����;
#define RX_ADDR_P0    0x0A  //����ͨ��0���յ�ַ,��󳤶�5���ֽ�,���ֽ���ǰ
#define RX_ADDR_P1    0x0B  //����ͨ��1���յ�ַ,��󳤶�5���ֽ�,���ֽ���ǰ
#define RX_ADDR_P2    0x0C  //����ͨ��2���յ�ַ,����ֽڿ�����,���ֽ�,����ͬRX_ADDR_P1[39:8]���;
#define RX_ADDR_P3    0x0D  //����ͨ��3���յ�ַ,����ֽڿ�����,���ֽ�,����ͬRX_ADDR_P1[39:8]���;
#define RX_ADDR_P4    0x0E  //����ͨ��4���յ�ַ,����ֽڿ�����,���ֽ�,����ͬRX_ADDR_P1[39:8]���;
#define RX_ADDR_P5    0x0F  //����ͨ��5���յ�ַ,����ֽڿ�����,���ֽ�,����ͬRX_ADDR_P1[39:8]���;
#define TX_ADDR       0x10  //���͵�ַ(���ֽ���ǰ),ShockBurstTMģʽ��,RX_ADDR_P0��˵�ַ���
#define RX_PW_P0      0x11  //��������ͨ��0��Ч���ݿ��(1~32�ֽ�),����Ϊ0��Ƿ�
#define RX_PW_P1      0x12  //��������ͨ��1��Ч���ݿ��(1~32�ֽ�),����Ϊ0��Ƿ�
#define RX_PW_P2      0x13  //��������ͨ��2��Ч���ݿ��(1~32�ֽ�),����Ϊ0��Ƿ�
#define RX_PW_P3      0x14  //��������ͨ��3��Ч���ݿ��(1~32�ֽ�),����Ϊ0��Ƿ�
#define RX_PW_P4      0x15  //��������ͨ��4��Ч���ݿ��(1~32�ֽ�),����Ϊ0��Ƿ�
#define RX_PW_P5      0x16  //��������ͨ��5��Ч���ݿ��(1~32�ֽ�),����Ϊ0��Ƿ�
#define FIFO_STATUS   0x17  //FIFO״̬�Ĵ���;bit0,RX FIFO�Ĵ����ձ�־;bit1,RX FIFO����־;bit2,3,����
                            //bit4,TX FIFO�ձ�־;bit5,TX FIFO����־;bit6,1,ѭ��������һ���ݰ�.0,��ѭ��;

#define DYNPD         0x1C  //ʹ�ܶ�̬���ݳ���ͨ��,bit0~5,��Ӧͨ��0~5
#define FEATRUE       0x1D  //bit2,ʹ�ܶ�̬���ݳ���;bit1,ʹ������Ӧ��;bit0,ʹ�����ݲ�Ӧ��;

#define MAX_RT        0x10  //�ﵽ����ʹ����ж�
#define TX_DS         0x20  //TX��������ж�
#define RX_DR         0x40  //���յ������ж�

#define RX_P_NO_MASK  0x0E
#define IRQ_ALL       (TX_DS|RX_DR|MAX_RT)

#define DataPipe0     0
#define DataPipe1     1
#define DataPipe2     2
#define DataPipe3     3
#define DataPipe4     4
#define DataPipe5     5
#define TxPipe        6

#define MAC_LEN				(5)
#define PAYLOAD_LEN			(32)

#define NRF_EVENT			(1<<0)
#define NRF_SEND_FINISH		(1<<1)
#define NRF_SEND_MAXCOUNT	(1<<2)

typedef void(*nrf_hw_recv_callback_t)(rt_uint8_t pipe_num, rt_uint8_t *rxdata, rt_uint8_t length);

void nrf_reg_hw_recv_callback(nrf_hw_recv_callback_t recv);
void NRF_Config(const u8 *addr,const u8 channel);
u8 NRF_SendPayload(const u8 ack,const u8 *addr,const u8 channel,const u8 *buf);
void NRF_RxMode(const u8 *addr,const u8 channel);
u8 NRF_DisturbanceDetect(void);
void NRF_SetChannel(const u8 channel);
u8 NRF_ReadChannel(void);
u8 NRF_SetRfPower(const u8 level);//(0-3)
u8 NRF_Check(void);
void NRF_SetPipe1Mac(u8 *addr);
void NRF_SetPipe2Mac(u8 *addr);

#endif

