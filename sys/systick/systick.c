/*************************************************************************
#    FileName: bsp_systick.c
#      Author: Allen
#       Email: qiurenguo@gmail.com
#    HomePage: Allen
#       Brief: 
#  LastChange: 2014-05-09 11:21:53
*************************************************************************/
/* Includes ------------------------------------------------------------*/
#include <stdio.h>
#include "stm32f10x.h"
#include "systick.h"

/* Types ---------------------------------------------------------------*/
/* Constants -----------------------------------------------------------*/
/* Define --------------------------------------------------------------*/
/* Variables -----------------------------------------------------------*/
volatile long jiffies=0;
/* Functions prototypes ------------------------------------------------*/
/* Functions -----------------------------------------------------------*/
/*
@brief  systick.
@param  None.
@retval None.
*/

//#include "debug_printf.h"


void SYSTICK_NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
	NVIC_InitStructure.NVIC_IRQChannel = (uint8_t)SysTick_IRQn;//Cortex-M3 System Tick Interrupt
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3 ;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);	
}


void SYSTICK_Configuration (void)
{
	jiffies = 0;
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);
	if (720000 > SysTick_LOAD_RELOAD_Msk)  while(1);/* Reload value impossible */
	SysTick->LOAD  = (720000 & SysTick_LOAD_RELOAD_Msk) - 1;/* set reload register */
	//NVIC_SetPriority (SysTick_IRQn, (1<<__NVIC_PRIO_BITS) - 1);/* set Priority for Cortex-M0 System Interrupts */
	SysTick->VAL   = 0;/* Load the SysTick Counter Value */
	SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk | 
	SysTick_CTRL_TICKINT_Msk | 
	SysTick_CTRL_ENABLE_Msk;/* Enable SysTick IRQ and SysTick Timer */
// 	if(SysTick_Config(720000) == 1){//10ms
// 		printf("\r\nsystick fail");
// 		while(1);
// 	}	
}

void SYSTICK_init(void)
{
	//systick NVIC init
	SYSTICK_NVIC_Configuration();

	//systick init
	SYSTICK_Configuration();
}

void SYSTICK_IntDisable(void)
{
	SysTick->CTRL &=(~SysTick_CTRL_TICKINT_Msk);
}
void SYSTICK_IntEnable(void)
{
	SysTick->CTRL |=(SysTick_CTRL_TICKINT_Msk);
}
void SYSTICK_ClearSystickVal (void)
{
	SysTick->VAL = 0;
}
long SYSTICK_GetVal(void)
{
	return SysTick->VAL;
}
void SYSTICK_Delay10ms(long i)
{
	long jif;
	SysTick->VAL = 0;
	jif = i+jiffies;
	while(jif>jiffies);
}

long SYSTICK_get_time(void)
{
	return jiffies;
}

//void delay_ms(int ms)
//{
//	int i,j;

//	for(i = 0; i < ms*2; i++)
//		for(j = 0; j < 5993; j++)
//			;
//}

//void delay_us(int us)
//{
//	int i,j;

//	for(i = 0; i < us; i++)
//		for(j = 0; j < 10; j++)
//			__ASM("NOP");
//}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
	//10ms one time

	jiffies ++;
}



/*********************************END OF FILE****************************/


