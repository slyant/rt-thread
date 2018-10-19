#include <st_mcuid.h>

unsigned int idAddr[]={
	0x1FFFF7AC,  /*STM32F0ΨһID��ʼ��ַ*/
	0x1FFFF7E8,  /*STM32F1ΨһID��ʼ��ַ*/
	0x1FFF7A10,  /*STM32F2ΨһID��ʼ��ַ*/
	0x1FFFF7AC,  /*STM32F3ΨһID��ʼ��ַ*/
	0x1FFF7A10,  /*STM32F4ΨһID��ʼ��ַ*/
	0x1FF0F420,  /*STM32F7ΨһID��ʼ��ַ*/
	0x1FF80050,  /*STM32L0ΨһID��ʼ��ַ*/
	0x1FF80050,  /*STM32L1ΨһID��ʼ��ַ*/
	0x1FFF7590,  /*STM32L4ΨһID��ʼ��ַ*/
	0x1FF0F420
}; /*STM32H7ΨһID��ʼ��ַ*/

/*��ȡMCU��ΨһID*/
void GetSTM32MCUID(unsigned int *id,MCUTypedef type)
{
  if(id)
  {
    id[0]=*(unsigned int*)(idAddr[type]);
    id[1]=*(unsigned int*)(idAddr[type]+4);
    id[2]=*(unsigned int*)(idAddr[type]+8);
  }
}
