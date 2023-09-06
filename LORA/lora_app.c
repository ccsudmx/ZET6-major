#include "lora_app.h"
#include "lora_ui.h"
#include "usart3.h"
#include "string.h"
#include "delay.h"
#include "stdio.h"
#include "fan.h"
#include "my_json.h"
 Json Fan=
{
	.name="FAN1",
	.status=0,
	.value=0,
	
};
Json tep={
  .name="TH",
  .status=1,
  .value=0,
   

};
Json time={
    .name="TIME",
    .hours=0,
    .min=0,
    .sec=0   

};

//设备参数初始化(具体设备参数见lora_cfg.h定义)
_LoRa_CFG LoRa_CFG=
{
	.addr = LORA_ADDR,       //设备地址
	.power = LORA_POWER,     //发射功率
	.chn = LORA_CHN,         //信道
	.wlrate = LORA_RATE,     //空中速率
	.wltime = LORA_WLTIME,   //睡眠时间
	.mode = LORA_MODE,       //工作模式
	.mode_sta = LORA_STA,    //发送状态
	.bps = LORA_TTLBPS ,     //波特率设置
	.parity = LORA_TTLPAR    //校验位设置
};

//全局参数
EXTI_InitTypeDef EXTI_InitStructure;
NVIC_InitTypeDef NVIC_InitStructure;

//设备工作模式(用于记录设备状态)
u8 Lora_mode=0;//0:配置模式 1:接收模式 2:发送模式
//记录中断状态
static u8 Int_mode=0;//0:关闭 1:上升沿 2:下降沿

//AUX中断设置
//mode:配置的模式 0:关闭 1:上升沿 2:下降沿
void Aux_Int(u8 mode)
{
    if(!mode)
	{
		EXTI_InitStructure.EXTI_LineCmd = DISABLE;//关闭中断
		NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;	
	}else
	{
		if(mode==1)
			 EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; //上升沿
	    else if(mode==2)
			 EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;//下降沿
		
		EXTI_InitStructure.EXTI_LineCmd = ENABLE;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		
	}
	Int_mode = mode;//记录中断模式
	EXTI_Init(&EXTI_InitStructure);
	NVIC_Init(&NVIC_InitStructure);  
    
}

//LORA_AUX中断服务函数
void EXTI4_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line4))
	{  
	   if(Int_mode==1)//上升沿(发送:开始发送数据 接收:数据开始输出)     
	   {
		  if(Lora_mode==1)//接收模式
		  {
			 USART3_RX_STA=0;//数据计数清0
		  }
		  Int_mode=2;//设置下降沿触发
	   }
       else if(Int_mode==2)//下降沿(发送:数据已发送完 接收:数据输出结束)	
	   {
		  if(Lora_mode==1)//接收模式
		  {
			 USART3_RX_STA|=1<<15;//数据计数标记完成
		  }else if(Lora_mode==2)//发送模式(串口数据发送完毕)
		  {
			 Lora_mode=1;//进入接收模式
		  }
		  Int_mode=1;//设置上升沿触发
	   }
       Aux_Int(Int_mode);//重新设置中断边沿
	   EXTI_ClearITPendingBit(EXTI_Line4); //清除LINE4上的中断标志位  
	}	
}

//LoRa模块初始化
//返回值: 0,检测成功
//        1,检测失败
u8 LoRa_Init(void)
{
	 u8 retry=0;
	 u8 temp=1;
	
	 GPIO_InitTypeDef  GPIO_InitStructure;
		
	 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	//使能PA端口时钟
	 RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);	//使能复用功能时钟

     GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);//禁止JTAG,从而PA15可以做普通IO使用,否则PA15不能做普通IO!!!	
	
	 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;	    		 //LORA_MD0
	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
	 GPIO_Init(GPIOA, &GPIO_InitStructure);	  				 //推挽输出 ，IO口速度为50MHz
	
	 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;				 //LORA_AUX
	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; 		     //下拉输入
	 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
	 GPIO_Init(GPIOA, &GPIO_InitStructure);					 //根据设定参数初始化GPIOA.4
	
	 GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource4);
	 EXTI_InitStructure.EXTI_Line=EXTI_Line4;
  	 EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
  	 EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  //上升沿触发
  	 EXTI_InitStructure.EXTI_LineCmd = DISABLE;              //中断线关闭(先关闭后面再打开)
  	 EXTI_Init(&EXTI_InitStructure);//根据EXTI_InitStruct中指定的参数初始化外设EXTI寄存器
	
	 NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;			//LORA_AUX
  	 NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;//抢占优先级2， 
  	 NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;		//子优先级3
  	 NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE; //关闭外部中断通道（后面再打开）
   	 NVIC_Init(&NVIC_InitStructure); 
	 
	 LORA_MD0=0;
	 LORA_AUX=0;
	
	 while(LORA_AUX)//确保LORA模块在空闲状态下(LORA_AUX=0)
	 {
		 printf("模块正忙,请稍等!!\n"); 	
		 delay_ms(500);
	 }
	 usart3_init(115200);//初始化串口3
	 
	 LORA_MD0=1;//进入AT模式
	 delay_ms(40);
	 retry=3;
	 
	 while(retry--)
	 {
		 if(!lora_send_cmd("ATE0","OK",70)) //OK
		 {
			 temp=0;//检测成功
			 break;
		 }	
	 }
	 if(retry==0) temp=1;//检测失败
	 return temp;
}

//Lora模块参数配置
void LoRa_Set(void)
{
	u8 sendbuf[20];
	u8 lora_addrh,lora_addrl=0;
	
	usart3_set(LORA_TTLBPS_115200,LORA_TTLPAR_8N1);//进入配置模式前设置通信波特率和校验位(115200 8位数据 1位停止 无数据校验）
	usart3_rx(1);//开启串口3接收
	
	while(LORA_AUX);//等待模块空闲
	LORA_MD0=1; //进入配置模式
	delay_ms(40);
	Lora_mode=0;//标记"配置模式"
	
	lora_addrh =  (LoRa_CFG.addr>>8)&0xff;
	lora_addrl = LoRa_CFG.addr&0xff;
    sprintf((char*)sendbuf,"AT+CGMR?");//设置设备地址
	lora_send_cmd(sendbuf,"OK",50);
   
	sprintf((char*)sendbuf,"AT+ADDR=%02x,%02x",lora_addrh,lora_addrl);//设置设备地址
	lora_send_cmd(sendbuf,"OK",50);
	sprintf((char*)sendbuf,"AT+WLRATE=%d,%d",LoRa_CFG.chn,LoRa_CFG.wlrate);//设置信道和空中速率
	lora_send_cmd(sendbuf,"OK",50);
	sprintf((char*)sendbuf,"AT+TPOWER=%d",LoRa_CFG.power);//设置发射功率
	lora_send_cmd(sendbuf,"OK",50);
	sprintf((char*)sendbuf,"AT+CWMODE=%d",LoRa_CFG.mode);//设置工作模式
	lora_send_cmd(sendbuf,"OK",50);
	sprintf((char*)sendbuf,"AT+TMODE=%d",LoRa_CFG.mode_sta);//设置发送状态
	lora_send_cmd(sendbuf,"OK",50);
	sprintf((char*)sendbuf,"AT+WLTIME=%d",LoRa_CFG.wltime);//设置睡眠时间
	lora_send_cmd(sendbuf,"OK",50);
	sprintf((char*)sendbuf,"AT+UART=%d,%d",LoRa_CFG.bps,LoRa_CFG.parity);//设置串口波特率、数据校验位
	lora_send_cmd(sendbuf,"OK",50);

	LORA_MD0=0;//退出配置,进入通信
	delay_ms(40);
	while(LORA_AUX);//判断是否空闲(模块会重新配置参数)
	USART3_RX_STA=0;
	Lora_mode=1;//标记"接收模式"
	usart3_set(LoRa_CFG.bps,LoRa_CFG.parity);//返回通信,更新通信串口配置(波特率、数据校验位)
	Aux_Int(1);//设置LORA_AUX上升沿中断	
	
}


u8 Tran_Data[200]={0};//透传数组


//Lora模块发送数据
void LoRa_SendData(char* message)
{   	
   

	
	Lora_mode=2;//标记"发送状态"
	
	if(LoRa_CFG.mode_sta == LORA_STA_Tran)//透明传输
	{
		sprintf((char*)Tran_Data,"%s",message);
		u3_printf("%s",Tran_Data);

		printf("Send：%s\r\n",Tran_Data);//显示发送的数据	
	}

			
}

int TH=0;

//Lora模块接收数据
void LoRa_ReceData(void)
{

	    int count=0;
		u8 msg[1024];
    
	if(USART3_RX_STA&0x8000)
	{  		 

			while(USART3_RX_BUF[count]!='\0')
			{
			    msg[count]=USART3_RX_BUF[count];
				count++;			
			}
			count++;
			msg[count]='\0';
            
		USART3_RX_BUF[USART3_RX_STA&0X7FFF]=0;//添加结束符
        USART3_RX_STA=0;
	
		if(strstr((const char *)msg, (const char *)"FAN1") != NULL){
		    
		       JSON_LORA((char *)msg,&Fan);
            if(Fan.status)
            {
			   TIM_SetCompare1(TIM1,(100-Fan.value)*72);
			   LoRa_SendData("FAN1 ON!");
				                         
            }
            else {
              TIM_SetCompare1(TIM1,100*72);
              LoRa_SendData("FAN1 OFF\r\n");
                  }
                  printf("FAN1_NEW_State: %s:%d:%d\r\n",Fan.name,Fan.status,Fan.value);
                
			}
        
        if(strstr((const char *)msg,(const char *) "TH")!=NULL)
        {
            
           JSON_LORA((char *)msg,&tep);
           TH=tep.value;
        
        
        }
        if(strstr((const char *)msg,(const char *)"TIME")!=NULL)
        {
           
              Json_time((char *)msg,&time);
        
        
        }
      
		
	
		memset((char*)USART3_RX_BUF,0x00,USART3_MAX_RECV_LEN);//串口接收缓冲区清0
		memset((char *)msg,0x00,count);
		count=0;
	}


	
}

#if 0
//发送和接收处理
void LoRa_Process(void)
{
	u8 key=0;
	u8 t=0;
		
 DATA:
	Process_ui();//界面显示
	LoRa_Set();//LoRa配置(进入配置需设置串口波特率为115200) 
	while(1)
	{
		
		key = KEY_Scan(0);
		
		if(key==KEY0_PRES)
		{
			if(LoRa_CFG.mode_sta==LORA_STA_Dire)//若是定向传输,则进入配置目标地址和信道界面
			{
				usart3_rx(0);//关闭串口接收
				Aux_Int(0);//关闭中断
				Dire_Set();//进入设置目标地址和信道
				goto DATA;
			}
		}else if(key==WKUP_PRES)//返回主菜单页面
		{
			LORA_MD0=1; //进入配置模式
	        delay_ms(40);
			usart3_rx(0);//关闭串口接收
			Aux_Int(0);//关闭中断
			break;
		}
		else if(key==KEY1_PRES)//发送数据
		{
			  if(!LORA_AUX&&(LoRa_CFG.mode!=LORA_MODE_SLEEP))//空闲且非省电模式
			  {
				  Lora_mode=2;//标记"发送状态"
				  LoRa_SendData();//发送数据    
			  }
		}	
		//数据接收
		//usart3_rx(1);
		LoRa_ReceData();
		
		t++;
		if(t==20)
		{
			t=0;
			LED1=~LED1;
		}			
		delay_ms(10);		
   }
	
}

#endif

void Lora_Test(void)
{
	delay_ms(6000);
	while(LoRa_Init())//初始化ATK-LORA-01模块 初始化为无写回模式
	{
		printf("未检测到模块!!!\r\n"); 	
		delay_ms(300);
	}
	printf("检测到模块!!!\r\n");
  delay_ms(500); 	
	
	//1.进入配置模式
	LORA_MD0=1; //进入配置模式
	delay_ms(40);
	usart3_rx(0);//关闭串口接收
	Aux_Int(0);//关闭中断
	
	LoRa_CFG.addr = 0x5410; // 0x5410
	LoRa_CFG.chn = 0x10; //0x20
	LoRa_CFG.power = LORA_PW_20Bbm;
	LoRa_CFG.wlrate = LORA_RATE_19K2;
	LoRa_CFG.wltime = LORA_WLTIME_1S;
	LoRa_CFG.mode = LORA_MODE_GEN;
	LoRa_CFG.mode_sta = LORA_STA_Tran;//LORA_STA_Dire
	LoRa_CFG.bps = LORA_TTLBPS_115200;
	LoRa_CFG.parity = LORA_TTLPAR_8N1;	
	LoRa_Set();

}
