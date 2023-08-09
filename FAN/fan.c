#include "fan.h"
#include "delay.h"


int SendFlag = 0;




//��ʪ�ȴ�����
 u8 humidity = 0,temperature = 0;
//���մ����� x2
 int illumination = 0;
//�������� x2
 int smoke = 0;

//void demo()
//{
//	u16 led0pwmval=0;    
//	u8 dir=1;	
//	TIM1_PWM_Init(899,0);//����Ƶ��PWMƵ��=72000/(899+1)=80Khz
//						 //Ƶ��Խ��Խ��������˸������ռ�ձ���Ƶ�ʵĹ�ϵ��
//								//һ�����۾�����80Hz ����ˢ��Ƶ������ȫû����˸��
//								//�����ĵ��Ƶ���� 6-16kHZ֮��Ϊ��
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
//		//ռ�ձȼ��㣺led0pwmval/arr+1;  ���Ҳ����300/900
//		//printf("led0pwmval:%d\r\n",(int)led0pwmval);
//		TIM_SetCompare1(TIM1,300);	
//       break;		//ͨ���Ƚ�ֵ   �޸�TIM1_CCR1 ռ�ձ�
//	} 

//}

//PWM�����ʼ��
//arr���Զ���װֵ
//psc��ʱ��Ԥ��Ƶ��
//A8 A11
void TIM1_PWM_Init(u16 arr,u16 psc)
{  
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);// ʹ�ܶ�ʱ��1
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE);  //ʹ��GPIO����ʱ��ʹ��
	                                                                     	
/******************** 1 ************************/
   //���ø�����Ϊ�����������,���TIM1 CH1��PWM���岨��
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8; //TIM_CH1
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //�����������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
   //���ø�����Ϊ�����������,���TIM1 CH4��PWM���岨��
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11; //TIM_CH4
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //�����������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	
	TIM_TimeBaseStructure.TIM_Period = arr; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	 80KHZ
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ  ����Ƶ
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure); //����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ

 
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; //ѡ��ʱ��ģʽ:TIM�����ȵ���ģʽ2
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //�Ƚ����ʹ��
	TIM_OCInitStructure.TIM_Pulse = 0; //���ô�װ�벶��ȽϼĴ���������ֵ
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //�������:TIM����Ƚϼ��Ը�
	
	/******************** 2 ************************/
	TIM_OC1Init(TIM1, &TIM_OCInitStructure);  //����TIM_OCInitStruct��ָ���Ĳ�����ʼ������TIMx  ͨ��1
	TIM_OC4Init(TIM1, &TIM_OCInitStructure);  //����TIM_OCInitStruct��ָ���Ĳ�����ʼ������TIMx  ͨ��4
	
    TIM_CtrlPWMOutputs(TIM1,ENABLE);	//MOE �����ʹ��	

	/******************** 3 ************************/
	TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);  //CH1Ԥװ��ʹ��	 
	TIM_OC4PreloadConfig(TIM1, TIM_OCPreload_Enable);  //CH4Ԥװ��ʹ��	
	
	TIM_ARRPreloadConfig(TIM1, ENABLE); //ʹ��TIMx��ARR�ϵ�Ԥװ�ؼĴ���
	
	TIM_Cmd(TIM1, ENABLE);  //ʹ��TIM1
}


//�����жϲ������Ϊ1ms��CLK_INT=72��Ԥ��Ƶϵ�� = 1000
/**********************************************************
** ������: TIM2_Int_Init
** ��������:  ������ʱ������
** �������: ��
** �������: ��
** ˵������ʱʱ��=(Ԥ��Ƶ��+1��*������ֵ+1) /TIM5ʱ��(72MHz)����λ(s)
   �������ʱ��t=(7200*10000)/72000000s=1s
***********************************************************/
void TIM2_Config(u16 arr,u16 psc)
{
	TIM_TimeBaseInitTypeDef    TIM_TimeBaseStructure;
	NVIC_InitTypeDef           NVIC_InitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); //ʹ��TIM2ʱ��
	//����жϱ�־λ
  TIM_ClearITPendingBit(TIM2,TIM_IT_Update);//TIM_ClearFlag(TIM2,TIM_FLAG_Update);//����������ͬ
	TIM_TimeBaseStructure.TIM_Period = arr; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);  //����ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
	TIM_ITConfig(TIM2,TIM_IT_Update, ENABLE);     //ʹ��ָ����TIM2�ж�,��������ж�
 
	//�ж����ȼ�NVIC����
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;  //TIM2�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //��ռ���ȼ�0��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //�����ȼ�3��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQͨ����ʹ��
	NVIC_Init(&NVIC_InitStructure);
	
	TIM_Cmd(TIM2, ENABLE);     //ʹ�ܶ�ʱ��2
}


void TIM2_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
	{
			
		SendFlag++;
		TIM_ClearITPendingBit(TIM2,TIM_IT_Update);//��������жϱ�־λ
	}
}


