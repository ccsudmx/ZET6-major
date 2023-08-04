
/*
  * @author  CCSU dmx
  * @version 1.0
  * @date    7-June-2023
  * @brief   ----???????--?????????-
  ???zet6 --?????????stm32f10x_it.c 
  
  ******************************************************************************
*/

#include "hal_core.h"
#include "string.h"
#include "debug_printf.h"
#include "WiFiToCloud.h"
#include "lora_app.h"
#include "usart2.h"
#include "rfid.h"
#include "stdio.h"
#include "Buzzer.h"
int main(void)
{
   	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//�ж����ȼ�����->��ռ���ȼ�2����Ӧ���ȼ�2
    SYSTICK_init();    //��ʼ��SysTick��ʱ��
    delay_init();      //��ʱ��ʼ�� ---����ǰ�棨��������ʼ�����õ���ʱ��
    TIME6_init();    //������ʹ��  
    usart3_init(9600); //��ʼ������3����FRIDͨ�ţ�
	uart_init(115200); //��ʼ������1 (����λ��ͨ��)
 	UART4_Init(115200);//��ʼ������4����ESP8266ͨ�ţ�
    ESP8266_init();
	Lora_Test(); //ʹ�ô���2 --�������ʼ��
    // printf("hello world");
	while(1)
	{		
	  // buzzer_on=1;
     cloud_task(); //����ͨ�Ž���--Ҫ�޸�   
     Lora_task();//lora����--�����˶���ֵ���߼�����
	 if(ReadId()==0)
    {
       //��ȡ�ɹ�
    }        
     
	//delay_ms(150);
	
	}
}
 


