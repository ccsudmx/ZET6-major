/**
  ******************************************************************************
  * @file    GPIO/IOToggle/stm32f10x_it.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and peripherals
  *          interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h" 
#include "stdio.h"
#include "usart.h"
#include "usart2.h"
#include "lora_app.h"
#include "timer.h"
#include "bsp_uart4.h"
#include "usart3.h"
#include "rfid.h"
void NMI_Handler(void)
{
}
 
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
	  printf("HardFault_Handler\r\n");
  }
}
 
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

 
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}
 
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}
 
void SVC_Handler(void)
{
}
 
void DebugMon_Handler(void)
{
}

void USART1_IRQHandler(void)                	//串口1中断服务程序
	{
	u8 Res;
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
		{
		Res =USART_ReceiveData(USART1);	//读取接收到的数据
		
		if((USART_RX_STA&0x8000)==0)//接收未完成
			{
			if(USART_RX_STA&0x4000)//接收到了0x0d
				{
				if(Res!=0x0a)USART_RX_STA=0;//接收错误,重新开始
				else USART_RX_STA|=0x8000;	//接收完成了 
				}
			else //还没收到0X0D
				{	
				if(Res==0x0d)USART_RX_STA|=0x4000;
				else
					{
					USART_RX_BUF[USART_RX_STA&0X3FFF]=Res ;
					USART_RX_STA++;
					if(USART_RX_STA>(USART_REC_LEN-1))USART_RX_STA=0;//接收数据错误,重新开始接收	  
					}		 
				}
			}   		 
     } 
										 
} 
void USART2_IRQHandler(void) //lora ----TIM5
{
	u8 res;	      
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)//接收到数据
	{	 
		res =USART_ReceiveData(USART2);		 
		if((USART2_RX_STA&(1<<15))==0)              //接收完的一批数据,还没有被处理,则不再接收其他数据
		{ 
			if(USART2_RX_STA<USART2_MAX_RECV_LEN)	//还可以接收数据
			{			
				if(!Lora_mode)//配置功能下(启动定时器超时)
				{
					TIM_SetCounter(TIM5,0);             //计数器清空          				
					if(USART2_RX_STA==0) 				//使能定时器5的中断 
					{
						TIM_Cmd(TIM5,ENABLE);           //使能定时器5
					}
				}
				USART2_RX_BUF[USART2_RX_STA++]=res;	//记录接收到的值	 
			}else 
			{
				USART2_RX_STA|=1<<15;				//强制标记接收完成
			} 
		}
	}  				 											 
}   
void USART3_IRQHandler(void)
{	
  if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
  { 

		if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
  	{ 
			Uart3RxBuf[Uart3RxDataConut]=USART_ReceiveData(USART3);
			Uart3RxDataConut++;
			if(Uart3RxDataConut >= UART3_RX_BUF_LEN)
			{
				Uart3RxDataConut = 0;
			}
			if(Uart3RxBuf[1]== Uart3RxDataConut)
			{
				Uart3RxDataConut = 0;
				Rx3Flag = 1;
			}
  	}
  }
}
  
void UART4_IRQHandler(void)
{
	if(USART_GetITStatus(UART4, USART_IT_RXNE) != RESET)  //接收到数据
	{
		F_UART4_RX_RECEIVING=1;//正在接收串口数据
		UART4_RX_TIMEOUT_COUNT=0;//超时计数清零
		UART4_RX_BUF[UART4_RX_COUNT] = USART_ReceiveData(UART4);	//读取接收到的数据
		UART4_RX_COUNT++;
		if(UART4_RX_COUNT>UART4_REC_LEN)//缓存区溢出
		{
		UART4_RX_COUNT = 0x0000;
		}	
		//end 接收未完成   		 		 
	}	//end 接收到数据
}
void TIM4_IRQHandler(void)
{ 		
	if(TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)//是更新中断
	{	 			   
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update  ); //清除TIM5更新中断标志 
		USART3_RX_STA|=1<<15;	 //标记接收完成
		TIM_Cmd(TIM4, DISABLE); //关闭TIM5
	}	    
}
void TIM5_IRQHandler(void)
{ 		
	if(TIM_GetITStatus(TIM5, TIM_IT_Update) != RESET)//是更新中断
	{	 			   
		TIM_ClearITPendingBit(TIM5, TIM_IT_Update  ); //清除TIM5更新中断标志 
		USART2_RX_STA|=1<<15;	 //标记接收完成
		TIM_Cmd(TIM5, DISABLE); //关闭TIM5
	}	    
}
 
//定时器7中断服务程序		    
void TIM7_IRQHandler(void)
{ 	
	if (TIM_GetITStatus(TIM7, TIM_IT_Update) != RESET)//是更新中断
	{
	
		TIM_ClearITPendingBit(TIM7, TIM_IT_Update  );  //清除TIM7更新中断标志    
	    if(F_UART4_RX_RECEIVING)//正在接收串口数据
		{
			UART4_RX_TIMEOUT_COUNT++;//串口超时计数
			if(UART4_RX_TIMEOUT_COUNT>3)//数据接收间隔超过3ms
			{//串口接收完成或结束
				F_UART4_RX_RECEIVING=0;
				UART4_RX_TIMEOUT_COUNT=0;
				F_UART4_RX_FINISH=1;
			}
		}
	}	    
}
//=========================================================


