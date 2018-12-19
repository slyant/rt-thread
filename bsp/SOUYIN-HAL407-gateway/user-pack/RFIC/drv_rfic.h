#ifndef __DRV_RFIC_H__
#define __DRV_RFIC_H__

/****************************************************************/
/*---------------     一下三项只能定义一个     -----------------*/
#define  RC_BUS_SPI             //SPI总线                     */
//#define  RC_BUS_UART            //UART总线                    */
//#define  RC_BUS_IIC             //IIC总线                     */
/****************************************************************/


/////////////////////////////////////////////////////////////////////
//MF522命令字
/////////////////////////////////////////////////////////////////////
#define PCD_IDLE              0x00               //取消当前命令
#define PCD_AUTHENT           0x0E               //验证密钥
#define PCD_RECEIVE           0x08               //接收数据
#define PCD_TRANSMIT          0x04               //发送数据
#define PCD_TRANSCEIVE        0x0C               //发送并接收数据
#define PCD_RESETPHASE        0x0F               //复位
#define PCD_CALCCRC           0x03               //CRC计算


/////////////////////////////////////////////////////////////////////
//Mifare_One卡片命令字
/////////////////////////////////////////////////////////////////////
#define PICC_REQIDL           0x26               //寻天线区内未进入休眠状态
#define PICC_REQALL           0x52               //寻天线区内全部卡
#define PICC_ANTICOLL1        0x93               //防冲撞
#define PICC_ANTICOLL2        0x95               //防冲撞
#define PICC_AUTHENT1A        0x60               //验证A密钥
#define PICC_AUTHENT1B        0x61               //验证B密钥
#define PICC_READ             0x30               //读块
#define PICC_WRITE            0xA0               //写块
#define PICC_DECREMENT        0xC0               //扣款
#define PICC_INCREMENT        0xC1               //充值
#define PICC_RESTORE          0xC2               //调块数据到缓冲区
#define PICC_TRANSFER         0xB0               //保存缓冲区中数据
#define PICC_HALT             0x50               //休眠


/////////////////////////////////////////////////////////////////////
//MF522寄存器定义
/////////////////////////////////////////////////////////////////////
// PAGE 0
#define     RFU_00                    0x00    
#define     COMMAND_REG               0x01    
#define     COM_IEN_REG               0x02    
#define     DIV_IEN_REG               0x03    
#define     COM_IRQ_REG               0x04    
#define     DIV_IRQ_REG               0x05
#define     ERROR_REG                 0x06    
#define     STATUS1_REG               0x07    
#define     STATUS2_REG               0x08    
#define     FIFO_DATA_REG             0x09
#define     FIFO_LEVEL_REG            0x0A
#define     WATER_LEVEL_REG           0x0B
#define     CONTROL_REG               0x0C
#define     BIT_FRAMING_REG           0x0D
#define     COLL_REG                  0x0E
#define     RFU_0F                    0x0F
// PAGE 1     
#define     RFU_10                    0x10
#define     MODE_REG                  0x11
#define     TX_MODE_REG               0x12
#define     RX_MODE_REG               0x13
#define     TX_CONTROL_REG            0x14
#define     TX_AUTO_REG               0x15
#define     TX_SEL_REG                0x16
#define     RX_SEL_REG                0x17
#define     RX_THRES_HOLD_REG         0x18
#define     DEMOD_REG                 0x19
#define     RFU_1A                    0x1A
#define     RFU_1B                    0x1B
#define     MI_FARE_REG               0x1C
#define     RFU_1D                    0x1D
#define     RFU_1E                    0x1E
#define     SERIAL_SPEED_REG          0x1F
// PAGE 2    
#define     RFU_20                    0x20  
#define     CRC_RESULT_REG_M          0x21
#define     CRC_RESULT_REG_L          0x22
#define     RFU_23                    0x23
#define     MOD_WIDTH_REG             0x24
#define     RFU_25                    0x25
#define     RFC_FG_REG                0x26
#define     GSN_REG                   0x27
#define     CW_GS_CFG_REG             0x28
#define     MOD_GS_CFG_REG            0x29
#define     TMOD_REG                  0x2A
#define     T_PRESCALER_REG           0x2B
#define     T_RELOAD_REG_H            0x2C
#define     T_RELOAD_REG_L            0x2D
#define     T_COUNTEER_VAL_REG_H      0x2E
#define     T_COUNTEER_VAL_REG_L      0x2F
// PAGE 3      
#define     RFU_30                    0x30
#define     TEST_SEL1_REG             0x31
#define     TEST_SEL2_REG             0x32
#define     TEST_PIN_EN_REG           0x33
#define     TEST_PIN_VAL_REG          0x34
#define     TEST_BUS_REG              0x35
#define     AUTO_TEST_REG             0x36
#define     VERSION_REG               0x37
#define     ANALONG_TEST_REG          0x38
#define     TEST_DAC1_REG             0x39  
#define     TEST_DAC2_REG             0x3A   
#define     TEST_ADC_REG              0x3B   
#define     RFU_3C                    0x3C   
#define     RFU_3D                    0x3D   
#define     RFU_3E                    0x3E   
#define     RFU_3F		              0x3F


/////////////////////////////////////////////////////////////////////
//和MF522通讯时返回的错误代码
/////////////////////////////////////////////////////////////////////
#define MI_OK                          0
#define MI_NOTAGERR                    1
#define MI_ERR                         2


int pcd_request_ex(unsigned char req_code,unsigned char *pTagType);
int pcd_anticoll_ex(unsigned char *pSnr);
int pcd_select_ex(unsigned char *pSnr);
int pcd_auth_state_ex(unsigned char auth_mode,unsigned char addr,unsigned char *pKey,unsigned char *pSnr);
int pcd_read_ex(unsigned char addr,unsigned char *pData);
int pcd_write_ex(unsigned char addr,unsigned char *pData);



#endif
