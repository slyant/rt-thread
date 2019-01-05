#ifndef __ST_MCUID_H__
/*定义STM32 MCU的类型*/
typedef enum {
  STM32F0_MCU,
  STM32F1_MCU,
  STM32F2_MCU,
  STM32F3_MCU,
  STM32F4_MCU,
  STM32F7_MCU,
  STM32L0_MCU,
  STM32L1_MCU,
  STM32L4_MCU,
  STM32H7_MCU,
}mcu_type_t;

void get_stm32_mcuid(unsigned int *id,mcu_type_t type);
#endif
