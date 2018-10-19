#include <rtthread.h>
#include <board.h>

static rt_uint32_t fac_us=0;							//us��ʱ������
			   
//��ʼ���ӳٺ���
//��ʹ��ucos��ʱ��,�˺������ʼ��ucos��ʱ�ӽ���
//SYSTICK��ʱ�ӹ̶�ΪAHBʱ��
//SYSCLK:ϵͳʱ��Ƶ��
int delay_init(void)
{
	fac_us=RT_HSE_HCLK/1000000;						//�����Ƿ�ʹ��OS,fac_us����Ҫʹ��
	return 0;
}
INIT_BOARD_EXPORT(delay_init);

//��ʱnus
//nusΪҪ��ʱ��us��.	
//nus:0~190887435(���ֵ��2^32/fac_us@fac_us=22.5)	 
void delay_us(rt_uint32_t nus)
{		
	rt_uint32_t ticks;
	rt_uint32_t told,tnow,tcnt=0;
	rt_uint32_t reload=SysTick->LOAD;				//LOAD��ֵ	    	 
	ticks=nus*fac_us; 						//��Ҫ�Ľ����� 
	told=SysTick->VAL;        				//�ս���ʱ�ļ�����ֵ
	while(1)
	{
		tnow=SysTick->VAL;	
		if(tnow!=told)
		{	    
			if(tnow<told)tcnt+=told-tnow;	//����ע��һ��SYSTICK��һ���ݼ��ļ������Ϳ�����.
			else tcnt+=reload-tnow+told;	    
			told=tnow;
			if(tcnt>=ticks)break;			//ʱ�䳬��/����Ҫ�ӳٵ�ʱ��,���˳�.
		}  
	};
}
RTM_EXPORT(delay_us);
//��ʱnms
//nms:Ҫ��ʱ��ms��
void delay_ms(rt_uint16_t nms)
{
	rt_uint32_t i;
	for(i=0;i<nms;i++) delay_us(1000);
}
RTM_EXPORT(delay_ms);
