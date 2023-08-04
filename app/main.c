
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
   	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//中断优先级分组->抢占优先级2，响应优先级2
    SYSTICK_init();    //初始化SysTick定时器
    delay_init();      //延时初始化 ---放在前面（后面许多初始化会用到延时）
    TIME6_init();    //蜂鸣器使用  
    usart3_init(9600); //初始化串口3（与FRID通信）
	uart_init(115200); //初始化串口1 (与上位机通信)
 	UART4_Init(115200);//初始化串口4（与ESP8266通信）
    ESP8266_init();
	Lora_Test(); //使用串口2 --在里面初始化
    // printf("hello world");
	while(1)
	{		
	  // buzzer_on=1;
     cloud_task(); //网络通信进程--要修改   
     Lora_task();//lora进程--包含了对阈值的逻辑控制
	 if(ReadId()==0)
    {
       //读取成功
    }        
     
	//delay_ms(150);
	
	}
}
 


