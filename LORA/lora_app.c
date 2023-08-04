#include "lora_app.h"
#include "lora_ui.h"
#include "usart3.h"
#include "string.h"
#include "delay.h"
#include "stdio.h"
#include "fan.h"
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
  u16 addr;
	u8 chn;
	u16 i=0; 
	
	Lora_mode=2;//标记"发送状态"
	
	if(LoRa_CFG.mode_sta == LORA_STA_Tran)//透明传输
	{
		sprintf((char*)Tran_Data,"%s",message);
		u3_printf("%s\r\n",Tran_Data);

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
			while(USART_GetFlagStatus(USART3,USART_FLAG_TC)==RESET);//循环发送,直到发送完毕   
			USART_SendData(USART3,date[i]); 
		}	
		
        //将十六进制的数据转化为字符串打印在lcd_buff数组
		sprintf((char*)wlcd_buff,"%x %x %x %x %x %x %x %x",
				date[0],date[1],date[2],date[3],date[4],date[5],date[6],date[7]);
		
	    Dire_Date[4]++;//Dire_Date[4]数据更新
		
	}
			
}

u8 rlcd_buff[10]={0}; //LCD显示字符串缓冲区



/*******************************************************************
*函数：char *USER_GetSteeringEngineJsonValue(char *cJson, char *Tag)
*功能：json为字符串序列，将json格式中的目标对象Tag对应的值字符串转换为数值
*输入：
		char *cJson json字符串
		char *Tag 要操作的对象标签
*输出：返回数值
*特殊说明：用户可以在此基础上改造和扩展该函数，这里只是个简单的DEMO
*******************************************************************/

uint8_t USER_GetSteeringEngineJsonValue(char *cJson, char *Tag)
{
	char *target = NULL;
	//char *str = NULL;
	static char temp[10];
	uint8_t len=0;
	uint8_t value=0;
	int8_t i=0;
	
	memset(temp, 0x00, 128);
	sprintf(temp,"\"%s\":\"",Tag);
	//printf("取值内部%s %s %s %s",cJson,Tag,USART3_RX_BUF,temp);
	target=strstr((const char *)cJson, (const char *)temp);
	if(target == NULL)
	{
		printf("空字符！\r\n");
		return NULL;
	}
	i=strlen((const char *)temp);
	target=target+i;
	memset(temp, 0x00, 128);
	for(i=0; i<10; i++, target++)//数值超过10个位为非法，由于2^32=4294967296
	{
		if((*target<='9')&&(*target>='0'))
		{
			temp[i]=*target;
		}
		else
		{
			break;
		}
	}
	//str=strstr((const char *)target, (const char *)"\",");
	
	temp[i+1] = '\0';
 //printf("数值str=%s\r\n",temp);
	//printf("数值str=%s\r\n",target);
 len=strlen((const char *)target);
	for(i=0; i<len;i++)
    {
	// printf("数值str[%d]=0x%x  \r\n",i,*(target+i));
	}
	len=strlen((const char *)temp);
	switch(len)
	{
		case(1):value=temp[0]-'0';break;
		case(2):value=temp[1]-'0'+(temp[0]-'0')*10;break;
		case(3):value=temp[2]-'0'+(temp[0]-'0')*100+(temp[1]-'0')*10;break;
		default:break;
	}
	
	
	//printf("数值value=%d\r\n",value);
	return value;
}
volatile int recFlag=0;

//Lora模块接收数据
void LoRa_ReceData(void)
{
//    u16 i=0;
        u16 len=0;
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
			printf("已经接收到数据了\r\n");
//		len = USART3_RX_STA&0X7FFF;
//		USART3_RX_BUF[len]=0;//添加结束符
   	//	USART3_RX_STA=0;

//		for(i=0;i<len;i++)
//		{
//			while(USART_GetFlagStatus(USART1,USART_FLAG_TC)==RESET); //循环发送,直到发送完毕   
//			USART_SendData(USART1,USART3_RX_BUF[i]); 
//		}
		if(LoRa_CFG.mode_sta==LORA_STA_Tran)//透明传输
		{	
			lora_at_response(1);//显示接收到的数据
		}
		int recFlag = 0;
		//数据解析
		//arr+1~0
		//占空比计算：fan0pwmval/arr+1;  最大也就是5500/7200
		//FAN占空比大于%50差不多就不转了
		//LIGHT占空比可以从%0~%100
		//FAN1
		if(strstr((const char *)USART3_RX_BUF, (const char *)"fanStatus1") != NULL){
		
			//printf("刚进来msg=%s\r\n",msg);
			recFlag++;
			fanStatus1 = (int) USER_GetSteeringEngineJsonValue((char *)msg, (char *)"fanStatus1");  
			//char temp[20];
			//sprintf(temp,"%d,%d",fanStatus1,fanSpeed1);
			//printf("%s",temp);
			//printf("出来后%s",USART3_RX_BUF);
			if(fanStatus1 == 0){
				TIM_SetCompare1(TIM1,100*72);
				LoRa_SendData("FAN1 OFF\r\n");
				printf("FAN1 OFF\r\n");
		    
			}
			else if(fanStatus1 == 1){
				TIM_SetCompare1(TIM1,fanVal1);
				LoRa_SendData("FAN1 ON\r\n");
				printf("FAN1 ON\r\n");
			}
		}
		if(strstr((const char *)msg, (const char *)"fanSpeed1") != NULL){
			recFlag++;
			fanSpeed1 = USER_GetSteeringEngineJsonValue((char *)msg, (char *)"fanSpeed1");
			
			fanVal1 = (int)(100 - fanSpeed1)*72;
			if(fanStatus1 == 1)
				TIM_SetCompare1(TIM1,fanVal1);	  //通道比较值   修改TIM1_CCR1 占空比
			
			printf("FAN1 = %d%%\r\n", fanSpeed1);
		}
		
		//FAN2
		if(strstr((const char *)USART3_RX_BUF, (const char *)"fanStatus2") != NULL){
			recFlag++;
			fanStatus2 = USER_GetSteeringEngineJsonValue((char *)USART3_RX_BUF, (char *)"fanStatus2");
			
			if(fanStatus2 == 0){
				TIM_SetCompare4(TIM1,100*72);
				printf("FAN2 OFF\r\n");
			}
			else if(fanStatus2 == 1){
				TIM_SetCompare4(TIM1,fanVal2);
				printf("FAN2 ON\r\n");
			}
		}
		if(strstr((const char *)USART3_RX_BUF, (const char *)"fanSpeed2") != NULL){
			recFlag++;
			fanSpeed2 = USER_GetSteeringEngineJsonValue((char *)USART3_RX_BUF, (char *)"fanSpeed2");
			
			fanVal2 = (int)(100 - fanSpeed2)*72;
			
			if(fanStatus2 == 1)
				TIM_SetCompare4(TIM1,fanVal2);	  //通道比较值   修改TIM1_CCR1 占空比
			printf("FAN2 = %d%%\r\n", fanSpeed2);
		}
		
		if(recFlag > 0){
			//确认消息回发
			LoRa_SendData("FanRecOK");
			recFlag=0;
		}

		if(strstr((const char *)msg, (const char *)"lightStatus") != NULL){
			recFlag++;
			lightStatus = USER_GetSteeringEngineJsonValue((char *)msg, (char *)"lightStatus");
			
			if(lightStatus == 0){
				TIM_SetCompare4(TIM1,100*72);
				printf("LIGHT OFF\r\n");
			}
			else if(lightStatus == 1){
				TIM_SetCompare4(TIM1,lightVal);
				printf("LIGHT ON\r\n");
			}
		}
		if(strstr((const char *)msg, (const char *)"lightPWM") != NULL){
			recFlag++;
			lightPWM = USER_GetSteeringEngineJsonValue((char *)msg, (char *)"lightPWM");
			
			lightVal = (int)(100 - lightPWM)*72;
			
			if(lightStatus == 1)
				TIM_SetCompare4(TIM1,lightVal);	  //通道比较值   修改TIM1_CCR1 占空比
			printf("LIGHT = %d%%\r\n", lightPWM);
		}
		if(recFlag > 0){
			//确认消息回发
			LoRa_SendData("LightRecOK");
		}
		
	
		memset((char*)USART3_RX_BUF,0x00,len);//串口接收缓冲区清0
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
