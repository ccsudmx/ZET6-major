
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

	//允许TIM6的时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6,ENABLE);

	//重新将Timer设置为缺省值
	TIM_DeInit(TIM6);

	//采用内部时钟给TIM2提供时钟源
	//TIM_InternalClockConfig(TIM6);

	//预分频系数为36000-1，这样计数器时钟为72MHz/72= 1us
	TIM_TimeBaseStructure.TIM_Prescaler = (SystemCoreClock /72) -1;

	//设置时钟分割
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;

	//设置计数器模式为向上计数模式
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

	//设置计数溢出大小，每计1000个数就产生一个更新事件  1ms
	TIM_TimeBaseStructure.TIM_Period = 1000-1;//arr ---250
	//将配置应用到TIM2中
	TIM_TimeBaseInit(TIM6,&TIM_TimeBaseStructure);
	
	//清除溢出中断标志
	TIM_ClearFlag(TIM6, TIM_FLAG_Update);

	//禁止ARR预装载缓冲器
	TIM_ARRPreloadConfig(TIM6, ENABLE);

	//开启TIM2的中断
	TIM_ITConfig(TIM6,TIM_IT_Update,ENABLE);

}

void TIME6_init(void)  
{
	 

	//interrupt init
	TIME6_NVIC_Configuration();
    //引脚初始化
    Buzzer_GPIO_Configuration();
    
	TIME6_cfg();
	
	//开启定时6
	TIM_Cmd(TIM6,ENABLE);
	
	
}

//enter ISR per TIME2_PERIOD_US
void TIM6_IRQHandler(void)
{
	static uint16_t data = 0;
	
   //检测是否发生溢出更新事件
   if(TIM_GetITStatus(TIM6, TIM_IT_Update) != RESET)
   {        
     
		 //清除TIM2的中断待处理位
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



