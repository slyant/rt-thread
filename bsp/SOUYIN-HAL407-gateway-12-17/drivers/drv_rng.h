#ifndef __DRV_RNG_H__
#define __DRV_RNG_H__

#include <rtthread.h>

int RNG_Init(void);
rt_uint32_t RNG_Get_RandomNum(void);//�õ������
int RNG_Get_RandomRange(int min,int max);//����[min,max]��Χ�������

#endif

