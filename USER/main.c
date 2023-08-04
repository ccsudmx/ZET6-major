#include "delay.h"
#include "sys.h"
#include "oled.h"
#include "bmp.h"
#include "lora_app.h"
#include "bh1750.h"
#include "stdio.h"
#include "usart.h"
#include "MQ2.h"
#include "usart3.h"
#include "timer.h"
#include "dht11.h"
#include "fan.h"
#include "bsp_rtc.h"

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	delay_init();
    TIM1_PWM_Init(7200,0);
	TIM2_Config(2000-1,36000-1);//((1+arr)/72M)*(1+psc)=(2000)/72M*(36000)=1s
  //TIM_SetCompare1(TIM1,7200);重新设置Pulse
    ADC_Pin_Init();
	DHT11_Init();
	OLED_Init();
    BH1750_Init();
    uart_init(115200);
	usart3_init(115200);
	Lora_Test();
	//TIM4_Int_Init(999,71);			
	while(1)
	{     
       char msg[10];char msg1[10];
       char msg2[10];char msg3[10];
       char ms[60];
      
		if(SendFlag>=1)  //1s发送一次数据
		{
		  SendFlag=0;
	      illumination= (int)LIght_Intensity();
		  smoke=(ADC_Trans()*3.27)/4095.0;
		  DHT11_Read_Data(&temperature,&humidity);
		  sprintf(msg,"%d",illumination); 
		  sprintf(msg1,"%d",smoke);
		  sprintf(msg2,"%d",temperature);
		  sprintf(msg3,"%d",humidity);
                
	      sprintf(ms, "{\"i1\":\"%d\",\"s1\":\"%d\",\"h1\":\"%d\",\"t1\":\"%d\"}",illumination, smoke, temperature, humidity);	
		  LoRa_SendData((char *)ms);
		}
         show_led(msg,msg1,msg2,msg3);
         LoRa_ReceData();
		
	}
}

