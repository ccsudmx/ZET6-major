#include "delay.h"
#include "usart2.h"
#include "stdarg.h"	 	 
#include "stdio.h"	 	 
#include "string.h"	 
#include "timer.h"
#include "lora_cfg.h"
#include "lora_app.h"

//串口接收缓存区 	
u8 USART2_RX_BUF[USART2_MAX_RECV_LEN]; 			//接收缓冲,最大USART2_MAX_RECV_LEN个字节.
u8 USART2_TX_BUF[USART2_MAX_SEND_LEN]; 			//发送缓冲,最大USART2_MAX_SEND_LEN字节

//通过判断接收连续2个字符之间的时间差不大于10ms来决定是不是一次连续的数据.
//如果2个字符接收间隔超过timer,则认为不是1次连续数据.也就是超过timer没有接收到
//任何数据,则表示此次接收完毕.
//接收到的数据状态
//[15]:0,没有接收到数据;1,接收到了一批数据.
//[14:0]:接收到的数据长度
vu16 USART2_RX_STA=0;   	

USART_InitTypeDef USART_InitStructure;
//初始化IO 串口2
//pclk1:PCLK1时钟频率(Mhz)
//bound:波特率	  
void usart2_init(u32 bound)
{  

	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); // GPIOA时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE); //串口2时钟使能

 	USART_DeInit(USART2);                           //复位串口2
   //USART2_TX   PA2-TX
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;      //PA2
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
    GPIO_Init(GPIOA, &GPIO_InitStructure);          //初始化PA2
   
    //USART2_RX	  PA3-RX
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;   //上拉输入
    GPIO_Init(GPIOA, &GPIO_InitStructure);          //初始化PA3
	
	USART_InitStructure.USART_BaudRate = bound;                     //波特率一般设置为9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;     //字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;          //一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;             //无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
	
	USART_Init(USART2, &USART_InitStructure); //初始化串口2
 
	USART_Cmd(USART2, ENABLE);                  //使能串口 
	
	//使能接收中断
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//开启中断   
	
	//设置中断优先级
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
	
	TIM5_Int_Init(99,7199);	//10ms中断
	USART2_RX_STA=0;		//清零
	TIM_Cmd(TIM5,DISABLE);	//关闭定时器5
}

//串口2,printf 函数
//确保一次发送数据不超过USART2_MAX_SEND_LEN字节
void u2_printf(char* fmt,...)  
{  
	u16 i,j; 
	va_list ap; 
	va_start(ap,fmt);
	vsprintf((char*)USART2_TX_BUF,fmt,ap);
	va_end(ap);
	i=strlen((const char*)USART2_TX_BUF);		//此次发送数据的长度
	for(j=0;j<i;j++)							//循环发送数据
	{
	  while(USART_GetFlagStatus(USART2,USART_FLAG_TC)==RESET); //循环发送,直到发送完毕   
	  USART_SendData(USART2,USART2_TX_BUF[j]); 
	} 
}

//串口2波特率和校验位配置
//bps:波特率（1200~115200）
//parity:校验位（无、偶、奇）
void usart2_set(u8 bps,u8 parity)
{
    static u32 bound=0;
	
	switch(bps)
	{
		case LORA_TTLBPS_1200:   bound=1200;     break;
		case LORA_TTLBPS_2400:   bound=2400;     break;
		case LORA_TTLBPS_4800:   bound=4800;     break;
		case LORA_TTLBPS_9600:   bound=9600;     break;
		case LORA_TTLBPS_19200:  bound=19200;    break;
		case LORA_TTLBPS_38400:  bound=38400;    break;
		case LORA_TTLBPS_57600:  bound=57600;    break;
		case LORA_TTLBPS_115200: bound=115200;   break;
	}
    
	USART_Cmd(USART2, DISABLE); //关闭串口 
	USART_InitStructure.USART_BaudRate = bound;
	USART_InitStructure.USART_StopBits = USART_StopBits_1; 
	
	if(parity==LORA_TTLPAR_8N1)//无校验
	{
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;    
		USART_InitStructure.USART_Parity = USART_Parity_No;
	}else if(parity==LORA_TTLPAR_8E1)//偶校验
	{
		USART_InitStructure.USART_WordLength = USART_WordLength_9b;    
		USART_InitStructure.USART_Parity = USART_Parity_Even;
	}else if(parity==LORA_TTLPAR_8O1)//奇校验
	{
		USART_InitStructure.USART_WordLength = USART_WordLength_9b;    
		USART_InitStructure.USART_Parity = USART_Parity_Odd;
	}
	USART_Init(USART2, &USART_InitStructure); //初始化串口2
    USART_Cmd(USART2, ENABLE); //使能串口 
	
}
 
//串口接收使能控制
//enable:0,关闭 1,打开
void usart2_rx(u8 enable)
{
	 USART_Cmd(USART2, DISABLE); //失能串口 
	
	 if(enable)
	 {
		 USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;//收发模式
	 }else
	 {
		 USART_InitStructure.USART_Mode = USART_Mode_Tx;//只发送 
	 }
	 
	 USART_Init(USART2, &USART_InitStructure); //初始化串口2
     USART_Cmd(USART2, ENABLE); //使能串口 
	
}













