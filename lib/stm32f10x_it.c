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

void USART1_IRQHandler(void)                	//����1�жϷ������
	{
	u8 Res;
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //�����ж�(���յ������ݱ�����0x0d 0x0a��β)
		{
		Res =USART_ReceiveData(USART1);	//��ȡ���յ�������
		
		if((USART_RX_STA&0x8000)==0)//����δ���
			{
			if(USART_RX_STA&0x4000)//���յ���0x0d
				{
				if(Res!=0x0a)USART_RX_STA=0;//���մ���,���¿�ʼ
				else USART_RX_STA|=0x8000;	//��������� 
				}
			else //��û�յ�0X0D
				{	
				if(Res==0x0d)USART_RX_STA|=0x4000;
				else
					{
					USART_RX_BUF[USART_RX_STA&0X3FFF]=Res ;
					USART_RX_STA++;
					if(USART_RX_STA>(USART_REC_LEN-1))USART_RX_STA=0;//�������ݴ���,���¿�ʼ����	  
					}		 
				}
			}   		 
     } 
										 
} 
void USART2_IRQHandler(void) //lora ----TIM5
{
	u8 res;	      
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)//���յ�����
	{	 
		res =USART_ReceiveData(USART2);		 
		if((USART2_RX_STA&(1<<15))==0)              //�������һ������,��û�б�����,���ٽ�����������
		{ 
			if(USART2_RX_STA<USART2_MAX_RECV_LEN)	//�����Խ�������
			{			
				if(!Lora_mode)//���ù�����(������ʱ����ʱ)
				{
					TIM_SetCounter(TIM5,0);             //���������          				
					if(USART2_RX_STA==0) 				//ʹ�ܶ�ʱ��5���ж� 
					{
						TIM_Cmd(TIM5,ENABLE);           //ʹ�ܶ�ʱ��5
					}
				}
				USART2_RX_BUF[USART2_RX_STA++]=res;	//��¼���յ���ֵ	 
			}else 
			{
				USART2_RX_STA|=1<<15;				//ǿ�Ʊ�ǽ������
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
	if(USART_GetITStatus(UART4, USART_IT_RXNE) != RESET)  //���յ�����
	{
		F_UART4_RX_RECEIVING=1;//���ڽ��մ�������
		UART4_RX_TIMEOUT_COUNT=0;//��ʱ��������
		UART4_RX_BUF[UART4_RX_COUNT] = USART_ReceiveData(UART4);	//��ȡ���յ�������
		UART4_RX_COUNT++;
		if(UART4_RX_COUNT>UART4_REC_LEN)//���������
		{
		UART4_RX_COUNT = 0x0000;
		}	
		//end ����δ���   		 		 
	}	//end ���յ�����
}
void TIM4_IRQHandler(void)
{ 		
	if(TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)//�Ǹ����ж�
	{	 			   
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update  ); //���TIM5�����жϱ�־ 
		USART3_RX_STA|=1<<15;	 //��ǽ������
		TIM_Cmd(TIM4, DISABLE); //�ر�TIM5
	}	    
}
void TIM5_IRQHandler(void)
{ 		
	if(TIM_GetITStatus(TIM5, TIM_IT_Update) != RESET)//�Ǹ����ж�
	{	 			   
		TIM_ClearITPendingBit(TIM5, TIM_IT_Update  ); //���TIM5�����жϱ�־ 
		USART2_RX_STA|=1<<15;	 //��ǽ������
		TIM_Cmd(TIM5, DISABLE); //�ر�TIM5
	}	    
}
 
//��ʱ��7�жϷ������		    
void TIM7_IRQHandler(void)
{ 	
	if (TIM_GetITStatus(TIM7, TIM_IT_Update) != RESET)//�Ǹ����ж�
	{
	
		TIM_ClearITPendingBit(TIM7, TIM_IT_Update  );  //���TIM7�����жϱ�־    
	    if(F_UART4_RX_RECEIVING)//���ڽ��մ�������
		{
			UART4_RX_TIMEOUT_COUNT++;//���ڳ�ʱ����
			if(UART4_RX_TIMEOUT_COUNT>3)//���ݽ��ռ������3ms
			{//���ڽ�����ɻ����
				F_UART4_RX_RECEIVING=0;
				UART4_RX_TIMEOUT_COUNT=0;
				F_UART4_RX_FINISH=1;
			}
		}
	}	    
}
//=========================================================


