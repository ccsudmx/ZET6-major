/*
******************************************************************************
* @file    TIM/TimeBase/main.c 
* @author  产品中心.硬件部
* @version V1.0.0
* @date    2017.09.07
* @brief   通用定时器2
******************************************************************************
*/

//Includes
#include "sys.h"
#include "bsp_timer3.h"
#include "stm32f10x_tim.h"
#include "debug_printf.h"
TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

NVIC_InitTypeDef NVIC_InitStructure;
TIM_OCInitTypeDef  TIM_OCInitStructure;
uint16_t capture = 0;
__IO uint16_t CCR1_Val = 60000;//1ms,1KHz
__IO uint16_t CCR2_Val = 600;//0.1ms,10KHz
__IO uint16_t CCR3_Val = 120;//0.02ms,50KHz
__IO uint16_t CCR4_Val = 60000;//10ms,100Hz


/*******************************************************************
*函数：void TIM2_Init(void)
*功能：定时器2初始化
*输入：无
*输出：无
*特殊说明：无
*******************************************************************/
void TIM3_Init(void)
{
	uint16_t PrescalerValue = 0;
	   
	/* --------------------------- System Clocks Configuration ---------------------*/
	/* TIM2 clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	/* --------------------------NVIC Configuration -------------------------------*/
	/* Enable the TIM2 gloabal Interrupt */
	MY_NVIC_Init(2, 2, TIM3_IRQn, NVIC_PriorityGroup_2);	//中断分组2
  
  
	/* ------------------- TIM2 Configuration:Output Compare Timing Mode ---------*/

	/* 
	SystemCoreClock = SYSCLK_FREQ_72MHz;
	PrescalerValue = (uint16_t) (SystemCoreClock / 6000000) - 1=11;
	TIM2 counter clock at SystemCoreClock/(PrescalerValue+1)=72/(11+1) MHz = 6 MHz
	CC1 update rate = TIM2 counter clock / CCR1_Val = 1KHz
	CC2 update rate = TIM2 counter clock / CCR2_Val = 10KHz
	CC3 update rate = TIM2 counter clock / CCR3_Val = 0.25KHz
	CC4 update rate = TIM2 counter clock / CCR4_Val = 0.1KHz
	*/

	/* Compute the prescaler value */
	PrescalerValue = (uint16_t) (SystemCoreClock / 6000000) - 1;

	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Period = 65535;
	TIM_TimeBaseStructure.TIM_Prescaler = 0;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

	/* Prescaler configuration */
	TIM_PrescalerConfig(TIM3, PrescalerValue, TIM_PSCReloadMode_Immediate);

	/* Output Compare Timing Mode configuration: Channel1 */
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Timing;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = CCR1_Val;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

	TIM_OC1Init(TIM3, &TIM_OCInitStructure);

	TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Disable);

//	/* Output Compare Timing Mode configuration: Channel2 */
//	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
//	TIM_OCInitStructure.TIM_Pulse = CCR2_Val;

//	TIM_OC2Init(TIM2, &TIM_OCInitStructure);

//	TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Disable);

//	/* Output Compare Timing Mode configuration: Channel3 */
//	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
//	TIM_OCInitStructure.TIM_Pulse = CCR3_Val;

//	TIM_OC3Init(TIM2, &TIM_OCInitStructure);

//	TIM_OC3PreloadConfig(TIM2, TIM_OCPreload_Disable);

//	/* Output Compare Timing Mode configuration: Channel4 */
//	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
//	TIM_OCInitStructure.TIM_Pulse = CCR4_Val;

//	TIM_OC4Init(TIM2, &TIM_OCInitStructure);

//	TIM_OC4PreloadConfig(TIM2, TIM_OCPreload_Disable);

	/* TIM IT enable */
	TIM_ITConfig(TIM3, TIM_IT_CC1| TIM_IT_CC4 /*| TIM_IT_CC2 | TIM_IT_CC3 | TIM_IT_CC4*/, ENABLE);

	/* TIM2 enable counter */
	TIM_Cmd(TIM3, ENABLE);

}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
