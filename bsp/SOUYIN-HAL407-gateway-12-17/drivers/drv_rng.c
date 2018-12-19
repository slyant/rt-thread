#include <rtthread.h>
#include <board.h>
#include <drv_rng.h>

RNG_HandleTypeDef hrng;

static int RNG_Init(void)
{
  hrng.Instance = RNG;
  if (HAL_RNG_Init(&hrng) != HAL_OK)
  {
    rt_kprintf("RNG_Init Failed!\n");
	  return RT_ERROR;
  }
  return RT_EOK;
}
INIT_BOARD_EXPORT(RNG_Init);

void HAL_RNG_MspInit(RNG_HandleTypeDef* rngHandle)
{
  if(rngHandle->Instance==RNG)
  {
    /* RNG clock enable */
    __HAL_RCC_RNG_CLK_ENABLE();
  }
}

void HAL_RNG_MspDeInit(RNG_HandleTypeDef* rngHandle)
{
  if(rngHandle->Instance==RNG)
  {
    /* Peripheral clock disable */
    __HAL_RCC_RNG_CLK_DISABLE();
  }
} 

//�õ������
//����ֵ:��ȡ���������
rt_uint32_t RNG_Get_RandomNum(void)
{
    return HAL_RNG_GetRandomNumber(&hrng);
}

//����[min,max]��Χ�������
int RNG_Get_RandomRange(int min,int max)
{ 
   return HAL_RNG_GetRandomNumber(&hrng)%(max-min+1) +min;
}
