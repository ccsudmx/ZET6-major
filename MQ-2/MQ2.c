#include "MQ2.h"
#include "stm32f10x.h"
#include "delay.h"
void ADC_Pin_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA
						| RCC_APB2Periph_ADC1,ENABLE);
	
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
	GPIO_Init(GPIOA,&GPIO_InitStruct);
	
RCC_ADCCLKConfig(RCC_PCLK2_Div8) ;                                  
ADC_DeInit(ADC1);                                                                      
ADC_DMACmd(ADC1, DISABLE);                                                

ADC_InitTypeDef ADC_InitStuctrue;                                              
ADC_InitStuctrue.ADC_Mode = ADC_Mode_Independent;         
ADC_InitStuctrue.ADC_ScanConvMode = DISABLE;                  
ADC_InitStuctrue.ADC_ContinuousConvMode = DISABLE;         
ADC_InitStuctrue.ADC_ExternalTrigConv=ADC_ExternalTrigConv_None; 
ADC_InitStuctrue.ADC_DataAlign=ADC_DataAlign_Right;         
ADC_InitStuctrue.ADC_NbrOfChannel = 1;                                   
ADC_Init(ADC1,&ADC_InitStuctrue);                                        

ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_1Cycles5);
ADC_Cmd(ADC1,ENABLE);                                                       
ADC_ResetCalibration(ADC1);                                                    
while(ADC_GetResetCalibrationStatus(ADC1));                         
ADC_StartCalibration(ADC1);                                                   
while(ADC_GetCalibrationStatus(ADC1));                                 
}



u16 ADC_Trans(void)
{
	u16 adc_value = 0;
	u8 i = 0;
	
	for(i = 0; i < 10; i++)
	{ 
		//开始转换
		ADC_SoftwareStartConvCmd(ADC1,ENABLE);

		//转换是否结束
		while(ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC) != SET);
		
		adc_value = adc_value + ADC_GetConversionValue(ADC1);//读ADC中的值
	}
	delay_ms(10);
	return adc_value / 10;//取样50次的平均值
}

