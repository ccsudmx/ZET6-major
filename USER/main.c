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
/*

（1）GPIO_Mode_AIN 模拟输入
（2）GPIO_Mode_IN_FLOATING 浮空输入
（3）GPIO_Mode_IPD 下拉输入
（4）GPIO_Mode_IPU 上拉输入
（5）GPIO_Mode_Out_OD 开漏输出
（6）GPIO_Mode_Out_PP 推挽输出
（7）GPIO_Mode_AF_OD 复用开漏输出
（8）GPIO_Mode_AF_PP 复用推挽输出

*/
void infrared_init()
{

   GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	//使能USART1，GPIOA时钟
  //红外
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;//PA3
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
  GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.10  
    //led灯
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;//PA2
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;//下拉输入
  GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.10  

}
extern int TH;
//static int mode=0;
int main(void)
{    
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	delay_init();
      infrared_init();
    TIM1_PWM_Init(7200,0);
	TIM2_Config(4000-1,36000-1);//((1+arr)/72M)*(1+psc)=(2000)/72M*(36000)=1s
  TIM_SetCompare1(TIM1,7200);
 //   ADC_Pin_Init();
	DHT11_Init();
	OLED_Init();
 //   BH1750_Init();
    uart_init(115200);
        
	usart3_init(115200);
	Lora_Test();
	//TIM4_Int_Init(999,71);			
	while(1)
	{     
        if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_3))
        {
        TIM_SetCompare1(TIM1,0);
           
        
        }
        else
        {
          TIM_SetCompare1(TIM1,7200);
        }
        
       char msg[10];char msg1[10];
       char msg2[10];char msg3[10];
       char msg_time[30];
       char ms[60];
        int value[6];
        Read_rtc(value);
       sprintf(msg_time,"%02d:%02d:%02d",value[3],value[4],value[5]);
//        for(int i=0;i<6;i++)
//              printf("%d ",value[i]);  
		if(SendFlag>=1)  //1s发送一次数据
		{
		  SendFlag=0;
         
//	      illumination= (int)LIght_Intensity();
//		  smoke=(ADC_Trans()*3.27)/4095.0;
		  DHT11_Read_Data(&temperature,&humidity);
            if(temperature<TH)
            {
            
             GPIO_WriteBit(GPIOA,GPIO_Pin_2,1);
              
            }
            else
            {
            GPIO_WriteBit(GPIOA,GPIO_Pin_2,0);
              
            }
                
//		  sprintf(msg,"%d",illumination); 
//		  sprintf(msg1,"%d",smoke);
		  sprintf(msg2,"%d",temperature);
            
            sprintf(ms,"{TIME:\"%s\",\"tep\":\"%d\"",msg_time,temperature);
//		  sprintf(msg3,"%d",humidity);
                
//	      sprintf(ms, "{\"i1\":\"%d\",\"s1\":\"%d\",\"h1\":\"%d\",\"t1\":\"%d\"}",illumination, smoke, temperature, humidity);	
		  LoRa_SendData((char *)ms);
		}
         show_led(msg,msg1,msg2,msg3);
         show_rtc(msg_time);
         LoRa_ReceData();
		
	}
}

