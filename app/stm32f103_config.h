/******************************************************************************
*
*
*
*
*
*
*
*
*
*
******************************************************************************/

#ifndef		_stm32f103_config_h_
#define		_stm32f103_config_h_

/******************************************************************************
                               外部函数头文件
                   应用到不同的外设头文件请在这里修改即可                        
******************************************************************************/

#include "sys.h"
#include "stm32f10x.h"

#include <string.h>	//内存操作相关函数库
#include <math.h>	//数学运算相关函数库
#include <stdio.h>	//printf相关
#include <stdbool.h>//布尔运算相关

#include "delay.h"
#include "hal_usart1.h"
#include "hal_uart4.h"
#include "hal_iic.h"
#include "hal_iic1.h"
#include "hal_key.h"
#include "hal_timer2.h"
#include "hal_buzzer.h"



#endif //_stm32f103_config_h_

