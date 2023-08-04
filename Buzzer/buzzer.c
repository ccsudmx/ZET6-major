
#include "stm32f10x.h"
#include "buzzer.h"
#include "stdio.h"
static uint16_t PrescalerValue = 0;
unsigned char buzzer_on = 0;



void Buzzer_GPIO_Configuration(void)
{ 
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_8;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
}

static void TIME6_NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM6_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3 ;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
}

static void TIME6_cfg(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

	//����TIM6��ʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6,ENABLE);

	//���½�Timer����Ϊȱʡֵ
	TIM_DeInit(TIM6);

	//�����ڲ�ʱ�Ӹ�TIM2�ṩʱ��Դ
	//TIM_InternalClockConfig(TIM6);

	//Ԥ��Ƶϵ��Ϊ36000-1������������ʱ��Ϊ72MHz/72= 1us
	TIM_TimeBaseStructure.TIM_Prescaler = (SystemCoreClock /72) -1;

	//����ʱ�ӷָ�
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;

	//���ü�����ģʽΪ���ϼ���ģʽ
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

	//���ü��������С��ÿ��1000�����Ͳ���һ�������¼�  1ms
	TIM_TimeBaseStructure.TIM_Period = 1000-1;//arr ---250
	//������Ӧ�õ�TIM2��
	TIM_TimeBaseInit(TIM6,&TIM_TimeBaseStructure);
	
	//�������жϱ�־
	TIM_ClearFlag(TIM6, TIM_FLAG_Update);

	//��ֹARRԤװ�ػ�����
	TIM_ARRPreloadConfig(TIM6, ENABLE);

	//����TIM2���ж�
	TIM_ITConfig(TIM6,TIM_IT_Update,ENABLE);

}

void TIME6_init(void)  
{
	 

	//interrupt init
	TIME6_NVIC_Configuration();
    //���ų�ʼ��
    Buzzer_GPIO_Configuration();
    
	TIME6_cfg();
	
	//������ʱ6
	TIM_Cmd(TIM6,ENABLE);
	
	
}

//enter ISR per TIME2_PERIOD_US
void TIM6_IRQHandler(void)
{
	static uint16_t data = 0;
	
   //����Ƿ�����������¼�
   if(TIM_GetITStatus(TIM6, TIM_IT_Update) != RESET)
   {        
     
		 //���TIM2���жϴ�����λ
		 TIM_ClearITPendingBit(TIM6 , TIM_FLAG_Update);

			if(buzzer_on){
				GPIO_ResetBits(GPIOA ,GPIO_Pin_8);
				data++;
				
			}
			if(data>=1000)
			{
				data=0;
				buzzer_on=0;
				GPIO_SetBits(GPIOA ,GPIO_Pin_8);
	
			}
	
   }
}



