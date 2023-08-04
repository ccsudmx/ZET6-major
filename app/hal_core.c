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
  * @param  uint32_t rtc_counter_val  rtc计数器值
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
  * @note    1.初始化函数    
  * @Date:   2017.8.5
  * @author: zhao
  * @return: int32_t
*/
int32_t	hal_init(void)
{

	//LF低频卡初始化（通过 与其进行通信）
	//LF125K_init();
    //buzzer
	//buzzerInit();
	//按键初始化
	//单机切换LF模式（写卡/支付）
	//双击切换LCD显示页
	//中断一次只能用一个
	//bsp_key_init(BUTTON_2, change_status_task, change_page_task);//2号按键
   // bsp_key_init1();//1号按键
	
	//数码管
	//HC595_GpioInit();
	
	
	//继电器初始化
	//STM32_GPIOx_Init(RYLAY1_Init);
	//STM32_GPIOx_Init(RYLAY2_Init);
	
	//P_FAN = OFF;//控制风扇寄存器
	//P_LAMP = OFF;//控制加热灯寄存器
	
	return 0;
}


