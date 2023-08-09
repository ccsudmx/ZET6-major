#include "fan.h"
#include "delay.h"


int SendFlag = 0;




//温湿度传感器
 u8 humidity = 0,temperature = 0;
//光照传感器 x2
 int illumination = 0;
//烟雾传感器 x2
 int smoke = 0;

//void demo()
//{
//	u16 led0pwmval=0;    
//	u8 dir=1;	
//	TIM1_PWM_Init(899,0);//不分频。PWM频率=72000/(899+1)=80Khz
//						 //频率越高越不容易闪烁（下是占空比与频率的关系）
//								//一般人眼睛对于80Hz 以上刷新频率则完全没有闪烁感
//								//正常的电机频率在 6-16kHZ之间为好
//						 //https://blog.csdn.net/as480133937/article/details/103439546/
//	
//   	while(1)
//	{

// 		delay_ms(10);	 
//		if(dir)led0pwmval++;
//		else led0pwmval--;	
//		
// 		if(led0pwmval>900)dir=0;
//		if(led0pwmval==0)dir=1;	   	
//		
//		//1000~0
//		//占空比计算：led0pwmval/arr+1;  最大也就是300/900
//		//printf("led0pwmval:%d\r\n",(int)led0pwmval);
//		TIM_SetCompare1(TIM1,300);	
//       break;		//通道比较值   修改TIM1_CCR1 占空比
//	} 

//}

//PWM输出初始化
//arr：自动重装值
//psc：时钟预分频数
//A8 A11
void TIM1_PWM_Init(u16 arr,u16 psc)
{  
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);// 使能定时器1
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE);  //使能GPIO外设时钟使能
	                                                                     	
/******************** 1 ************************/
   //设置该引脚为复用输出功能,输出TIM1 CH1的PWM脉冲波形
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8; //TIM_CH1
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //复用推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
   //设置该引脚为复用输出功能,输出TIM1 CH4的PWM脉冲波形
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11; //TIM_CH4
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //复用推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	
	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	 80KHZ
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值  不分频
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位

 
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; //选择定时器模式:TIM脉冲宽度调制模式2
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
	TIM_OCInitStructure.TIM_Pulse = 0; //设置待装入捕获比较寄存器的脉冲值
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //输出极性:TIM输出比较极性高
	
	/******************** 2 ************************/
	TIM_OC1Init(TIM1, &TIM_OCInitStructure);  //根据TIM_OCInitStruct中指定的参数初始化外设TIMx  通道1
	TIM_OC4Init(TIM1, &TIM_OCInitStructure);  //根据TIM_OCInitStruct中指定的参数初始化外设TIMx  通道4
	
    TIM_CtrlPWMOutputs(TIM1,ENABLE);	//MOE 主输出使能	

	/******************** 3 ************************/
	TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);  //CH1预装载使能	 
	TIM_OC4PreloadConfig(TIM1, TIM_OCPreload_Enable);  //CH4预装载使能	
	
	TIM_ARRPreloadConfig(TIM1, ENABLE); //使能TIMx在ARR上的预装载寄存器
	
	TIM_Cmd(TIM1, ENABLE);  //使能TIM1
}


//设置中断产生间隔为1ms，CLK_INT=72，预分频系数 = 1000
/**********************************************************
** 函数名: TIM2_Int_Init
** 功能描述:  基本定时器配置
** 输入参数: 无
** 输出参数: 无
** 说明：定时时间=(预分频数+1）*（计数值+1) /TIM5时钟(72MHz)，单位(s)
   这里溢出时间t=(7200*10000)/72000000s=1s
***********************************************************/
void TIM2_Config(u16 arr,u16 psc)
{
	TIM_TimeBaseInitTypeDef    TIM_TimeBaseStructure;
	NVIC_InitTypeDef           NVIC_InitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); //使能TIM2时钟
	//清除中断标志位
  TIM_ClearITPendingBit(TIM2,TIM_IT_Update);//TIM_ClearFlag(TIM2,TIM_FLAG_Update);//两者作用相同
	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);  //根据指定的参数初始化TIMx的时间基数单位
	TIM_ITConfig(TIM2,TIM_IT_Update, ENABLE);     //使能指定的TIM2中断,允许更新中断
 
	//中断优先级NVIC设置
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;  //TIM2中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //先占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //从优先级3级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);
	
	TIM_Cmd(TIM2, ENABLE);     //使能定时器2
}


void TIM2_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
	{
			
		SendFlag++;
		TIM_ClearITPendingBit(TIM2,TIM_IT_Update);//清除更新中断标志位
	}
}


