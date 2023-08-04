#include "lora_app.h"
#include "lora_ui.h"
#include "usart2.h"
#include "string.h"
#include "delay.h"
#include "stdio.h"

#include "hal_core.h"
#include "app_cloud.h"
#include "WiFiToCloud.h"
#include "hardWareVariables.h"

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
NVIC_InitTypeDef Lora_NVIC_InitStructure;

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
		Lora_NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;	
	}else
	{
		if(mode==1)
			 EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; //上升沿
	    else if(mode==2)
			 EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;//下降沿
		
		EXTI_InitStructure.EXTI_LineCmd = ENABLE;
		Lora_NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		
	}
	Int_mode = mode;//记录中断模式
	EXTI_Init(&EXTI_InitStructure);
	NVIC_Init(&Lora_NVIC_InitStructure);  
    
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
			 USART2_RX_STA=0;//数据计数清0
		  }
		  Int_mode=2;//设置下降沿触发
	   }
       else if(Int_mode==2)//下降沿(发送:数据已发送完 接收:数据输出结束)	
	   {
		  if(Lora_mode==1)//接收模式
		  {
			 USART2_RX_STA|=1<<15;//数据计数标记完成
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
	
	  Lora_NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;			//LORA_AUX
  	 Lora_NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;//抢占优先级2， 
  	 Lora_NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;		//子优先级3
  	 Lora_NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE; //关闭外部中断通道（后面再打开）
   	 NVIC_Init(&Lora_NVIC_InitStructure); 
	 
	 LORA_MD0=0;
	 LORA_AUX=0;
	
	 while(LORA_AUX)//确保LORA模块在空闲状态下(LORA_AUX=0)
	 {
		 printf("模块正忙,请稍等!!\n"); 	
		 delay_ms(500);
	 }
	 usart2_init(115200);//初始化串口2
	 
	 LORA_MD0=1;//进入AT模式
	 delay_ms(40);
	 retry=3;
	 
	 while(retry--)
	 {
		 if(!lora_send_cmd("ATE0","OK",70)) //OK
		 {
			 temp=0;//检测成功
			 printf("检测成功");
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
	
	usart2_set(LORA_TTLBPS_115200,LORA_TTLPAR_8N1);//进入配置模式前设置通信波特率和校验位(115200 8位数据 1位停止 无数据校验）
	usart2_rx(1);//开启串口2接收
	
	while(LORA_AUX);//等待模块空闲
	LORA_MD0=1; //进入配置模式
	delay_ms(40);
	Lora_mode=0;//标记"配置模式"
	
	lora_addrh =  (LoRa_CFG.addr>>8)&0xff;
	lora_addrl = LoRa_CFG.addr&0xff;
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
	USART2_RX_STA=0;
	Lora_mode=1;//标记"接收模式"
	usart2_set(LoRa_CFG.bps,LoRa_CFG.parity);//返回通信,更新通信串口配置(波特率、数据校验位)
	Aux_Int(1);//设置LORA_AUX上升沿中断	
	
}

u8 Dire_Date[]={0x11,0x22,0x33,0x44,0x55};//定向传输数据
u8 date[30]={0};//定向数组

#define Dire_DateLen sizeof(Dire_Date)/sizeof(Dire_Date[0])
extern u32 obj_addr;//记录用户输入目标地址
extern u8 obj_chn;//记录用户输入目标信道
u8 Tran_Data[200]={0};//透传数组

u8 wlcd_buff[10]={0}; //LCD显示字符串缓冲区
//Lora模块发送数据
void LoRa_SendData(char* message)
{      
	static u8 num=0;
    u16 addr;
	u8 chn;
	u16 i=0; 
		
	Lora_mode=2;//标记"发送状态"
	if(LoRa_CFG.mode_sta == LORA_STA_Tran)//透明传输
	{
		sprintf((char*)Tran_Data,"%s",message);
		u2_printf("%s\r\n",Tran_Data);
		
		printf("Send：%s\r\n",Tran_Data);//显示发送的数据	
	}else if(LoRa_CFG.mode_sta == LORA_STA_Dire)//定向传输
	{
		
		addr = (u16)obj_addr;//目标地址
		chn = obj_chn;//目标信道
		
		date[i++] =(addr>>8)&0xff;//高位地址
		date[i++] = addr&0xff;//低位地址
		date[i] = chn;//无线信道
		
		for(i=0;i<Dire_DateLen;i++)//数据写到发送BUFF
		{
			date[3+i] = Dire_Date[i];
		}	
		for(i=0;i<(Dire_DateLen+3);i++)
		{
			while(USART_GetFlagStatus(USART2,USART_FLAG_TC)==RESET);//循环发送,直到发送完毕   
			USART_SendData(USART2,date[i]); 
		}	
		
        //将十六进制的数据转化为字符串打印在lcd_buff数组
		sprintf((char*)wlcd_buff,"%x %x %x %x %x %x %x %x",
				date[0],date[1],date[2],date[3],date[4],date[5],date[6],date[7]);
		
	    Dire_Date[4]++;//Dire_Date[4]数据更新
	}
			
}


int LoRa_SendData_Affirm(char* message)
{
	//1.发
	//2.等
	//3.检验
	
	int times = 3;
	int waittime = 50;
	while(times > 0){
		LoRa_SendData(message);
		while(waittime > 0){	//等待倒计时 
			waittime--;
			delay_ms(10);
			if(USART2_RX_STA&0X8000){ //接收完一次数据
			//检验目标子串
				USART2_RX_BUF[USART2_RX_STA&0X7FFF]=0;//添加结束符
				if(strstr((const char *)USART2_RX_BUF, (const char *)"RecOK") != NULL){
					memset((char*)USART2_RX_BUF,0x00,USART2_RX_STA&0X7FFF);//串口接收缓冲区清0
					
					alarmFlag = 0;
					return 0;
				}
				memset((char*)USART2_RX_BUF,0x00,USART2_RX_STA&0X7FFF);//串口接收缓冲区清0
			} 
		}
		#if 0
		delay_ms(300);
		
		u16 i = 0;
		u16 len=0;
		//有数据来了
		if(USART2_RX_STA&0x8000){
			len = USART2_RX_STA&0X7FFF;
			USART2_RX_BUF[len]=0;//添加结束符
			USART2_RX_STA=0;

			for(i=0;i<len;i++)
			{
				while(USART_GetFlagStatus(USART1,USART_FLAG_TC)==RESET); //循环发送,直到发送完毕   
				USART_SendData(USART1,USART2_RX_BUF[i]); 
			}
			if(LoRa_CFG.mode_sta==LORA_STA_Tran)//透明传输
			{	
				printf("%s",USART2_RX_BUF);//显示接收到的数据

			}
			
			if(strstr((const char *)USART2_RX_BUF, (const char *)"RecOK") != NULL){
				memset((char*)USART2_RX_BUF,0x00,len);//串口接收缓冲区清0
				return 0;
			}
			
			memset((char*)USART2_RX_BUF,0x00,len);//串口接收缓冲区清0
		}
		#endif
		times--;
	}
	
	printf("Send Failed");
	alarmFlag = -1;
	return -1;
}

u8 rlcd_buff[10]={0}; //LCD显示字符串缓冲区

int peopleNum = 0;
//可调灯 x1
volatile int lightStatus = 0, lightPWM = 0;
//风扇 x2
volatile int fanStatus1 = 0, fanSpeed1 = 0;
volatile int fanStatus2 = 0, fanSpeed2 = 0;
//步进电机 x2
volatile int motorStatus1 = 0, motorSpeed1 = 0, motorDirection1 = 0;
volatile int motorStatus2 = 0, motorSpeed2 = 0, motorDirection2 = 0;
//红外对射开关 x2
volatile int infraredStatus1 = 0;
volatile int infraredStatus2 = 0;
//蜂鸣器 x1
volatile int buzzerStatus = 0;
//LED灯 x4
volatile int LEDStatus1 = 0, LEDStatus2 = 0, LEDStatus3 = 0, LEDStatus4 = 0;
//温湿度传感器
volatile int h1 = 0, h2 = 0, t1 = 0, t2 = 0;
//光照传感器 x2
volatile int i1 = 0, i2 = 0;
//烟雾传感器 x2
volatile int s1 = 0, s2 = 0;
volatile int wifiConnectStatus  = 0;
volatile int serverConnectStatus = 0;

int wifi = 0;
int server = 0;

//Lora模块接收数据
int speed_flag = 0;//0关 1开
void LoRa_ReceData(void)
{
   // u16 i=0;
    u16 len=0;
   
	//有数据来了
	if(USART2_RX_STA&0x8000)
	{
		len = USART2_RX_STA&0X7FFF;

		if(LoRa_CFG.mode_sta==LORA_STA_Tran)//透明传输
		{	
			lora_at_response(1);//显示接收到的数据

		}else if(LoRa_CFG.mode_sta==LORA_STA_Dire)//定向传输
		{
			//将十六进制的数据转化为字符串打印在lcd_buff数组
			sprintf((char*)rlcd_buff,"%x %x %x %x %x",
			USART2_RX_BUF[0],USART2_RX_BUF[1],USART2_RX_BUF[2],USART2_RX_BUF[3],USART2_RX_BUF[4]);
				
			//Show_Str_Mid(10,270,rlcd_buff,16,240);//显示接收到的数据	
			lora_at_response(0);
		}
		
		/****************对数据的处理逻辑*********************/
		
		uint8_t TxetBuf[128];
			
		//光照强度
		int sensorFlag = 0;
		if(strstr((const char *)USART2_RX_BUF, (const char *)"i1") != NULL){
			i1 = (int) USER_GetSteeringEngineJsonValue(USART2_RX_BUF, "i1");  
			sensorFlag = 1;
		}
		else if(strstr((const char *)USART2_RX_BUF, (const char *)"i2") != NULL){
			i2 = (int)USER_GetSteeringEngineJsonValue(USART2_RX_BUF, "i2");
			sensorFlag = 2;
		}
		//湿度
		if(strstr((const char *)USART2_RX_BUF, (const char *)"h1") != NULL){
			h1 = (int)USER_GetSteeringEngineJsonValue(USART2_RX_BUF, "h1");
			sensorFlag = 1;
		}
		else if(strstr((const char *)USART2_RX_BUF, (const char *)"h2") != NULL){
			h2 = (int)USER_GetSteeringEngineJsonValue(USART2_RX_BUF, "h2");
			sensorFlag = 2;
		}
		//温度
		if(strstr((const char *)USART2_RX_BUF, (const char *)"t1") != NULL){
			t1 = (int)USER_GetSteeringEngineJsonValue(USART2_RX_BUF, "t1");
			sensorFlag = 1;
		}
		else if(strstr((const char *)USART2_RX_BUF, (const char *)"t2") != NULL){
			t2 = (int)USER_GetSteeringEngineJsonValue(USART2_RX_BUF, "t2");
			sensorFlag = 2;
		}
		//烟雾浓度
		if(strstr((const char *)USART2_RX_BUF, (const char *)"s1") != NULL){
			s1 = (int)USER_GetSteeringEngineJsonValue(USART2_RX_BUF, "s1");
			sensorFlag = 1;
		}
		else if(strstr((const char *)USART2_RX_BUF, (const char *)"s2") != NULL){
			s2 = (int)USER_GetSteeringEngineJsonValue(USART2_RX_BUF, "s2");
			sensorFlag = 2;
		}	
	
		memset((char*)USART2_RX_BUF,0x00,len);//串口接收缓冲区清0
	}

}


int lightLastTime = 0;
int lightFlag = 0;
int motorFlag = 0;
void Lora_task(void)
{
	LoRa_ReceData();
		
}

void Lora_Test(void)
{	
	//delay_ms(6000);
    delay_ms(1000);
	while(LoRa_Init())//初始化ATK-LORA-01模块 初始化为无写回模式
	{
		printf("未检测到模块!!!\r\n"); 	
		delay_ms(300);
		printf("                \r\n");
	}
	printf("检测到模块!!!\r\n");
    delay_ms(500); 	
	
	//1.进入配置模式
	LORA_MD0=1; //进入配置模式
	delay_ms(40);
	usart2_rx(0);//关闭串口接收
	Aux_Int(0);//关闭中断
	
	LoRa_CFG.addr = 0x5410;
	LoRa_CFG.chn = 0x10;
	LoRa_CFG.power = LORA_PW_20Bbm;
	LoRa_CFG.wlrate = LORA_RATE_19K2;
	LoRa_CFG.wltime = LORA_WLTIME_1S;
	LoRa_CFG.mode = LORA_MODE_GEN;
	LoRa_CFG.mode_sta = LORA_STA_Tran;//LORA_STA_Dire
	LoRa_CFG.bps = LORA_TTLBPS_115200;
	LoRa_CFG.parity = LORA_TTLPAR_8N1;	
	LoRa_Set();//自动打开usart2_rx(1);
}

