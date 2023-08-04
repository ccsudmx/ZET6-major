/******************************************************************************
******************************************************************************
* @file    app/hal_core.c 
* @author  zhao
* @version V1.0.0
* @date    2018.8.1
* @update  2018.8.2
* @brief   hal_core
******************************************************************************
******************************************************************************/


/**************include*************************/

#include "hal_core.h"
#include "debug_printf.h"


/**************code*************************/


/*
  * @brief  NVIC_Priority_Group_Configuration
  * @param  none
  * @note        
  * @Date:  2017.7.10
  * @updatge:2017.7.10
  * @author:zhao
  * @return:void
*/
void NVIC_Priority_Group_Configuration(void)
{	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
}



/*
  * @brief  rtc_update_notify
  * @param  uint32_t rtc_counter_val  rtc������ֵ
  * @note        
  * @Date:   2017.8.5
  * @updatge:2017.8.5
  * @author:zhao
  * @return:void
*/
uint32_t rtc_second, is_rtc_update;
void rtc_update_notify(uint32_t rtc_counter_val)
{
	is_rtc_update = 1;
	rtc_second = rtc_counter_val;
}

/*
  * @brief  hal_init
  * @param  void
  * @note    1.��ʼ������    
  * @Date:   2017.8.5
  * @author: zhao
  * @return: int32_t
*/
int32_t	hal_init(void)
{

	//LF��Ƶ����ʼ����ͨ�� �������ͨ�ţ�
	//LF125K_init();
    //buzzer
	//buzzerInit();
	//������ʼ��
	//�����л�LFģʽ��д��/֧����
	//˫���л�LCD��ʾҳ
	//�ж�һ��ֻ����һ��
	//bsp_key_init(BUTTON_2, change_status_task, change_page_task);//2�Ű���
   // bsp_key_init1();//1�Ű���
	
	//�����
	//HC595_GpioInit();
	
	
	//�̵�����ʼ��
	//STM32_GPIOx_Init(RYLAY1_Init);
	//STM32_GPIOx_Init(RYLAY2_Init);
	
	//P_FAN = OFF;//���Ʒ��ȼĴ���
	//P_LAMP = OFF;//���Ƽ��ȵƼĴ���
	
	return 0;
}


