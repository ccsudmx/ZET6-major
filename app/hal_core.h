/******************************************************************************
******************************************************************************
* @file    app/hal_core.h 
* @author  zhao
* @version V1.0.0
* @date    2018.07.17
* @update  2018.07.17 
* @brief   app core 
******************************************************************************
******************************************************************************/
#ifndef __HAL_CORE_H__
#define __HAL_CORE_H__

/*******************继电器模块接线*********************/
//RYLAY1-IN
#define RYLAY1_PIN_PORT		GPIOC
#define RYLAY1_PIN_PORT_CLK	RCC_APB2Periph_GPIOC
#define RYLAY1_PIN			GPIO_Pin_3
#define RYLAY1_Init			RYLAY1_PIN_PORT_CLK,\
							RYLAY1_PIN_PORT,\
							RYLAY1_PIN,GPIO_Speed_10MHz,GPIO_Mode_Out_PP
#define RYLAY1				PCout(3)

//RYLAY2-IN
#define RYLAY2_PIN_PORT		GPIOC
#define RYLAY2_PIN_PORT_CLK	RCC_APB2Periph_GPIOC
#define RYLAY2_PIN			GPIO_Pin_2
#define RYLAY2_Init			RYLAY2_PIN_PORT_CLK,\
							RYLAY2_PIN_PORT,\
							RYLAY2_PIN,GPIO_Speed_10MHz,GPIO_Mode_Out_PP
#define RYLAY2				PCout(2)

#define P_FAN		RYLAY1//控制风扇寄存器
#define P_LAMP		RYLAY2//控制加热灯寄存器
#define P_LAMP_STA	GPIO_ReadOutputDataBit(RYLAY2_PIN_PORT, RYLAY2_PIN)
#define	ON			1	//开启
#define	OFF			0	//关闭

/*******************PCF8591-I2C总线*********************/
#define PCF8591_I2C_SDA_PIN_PORT		GPIOB
#define PCF8591_I2C_SDA_PIN_PORT_CLK	RCC_APB2Periph_GPIOB
#define PCF8591_I2C_SDA_PIN				GPIO_Pin_7
#define PCF8591_I2C_SDA_OUT_Init		PCF8591_I2C_SDA_PIN_PORT_CLK,\
										PCF8591_I2C_SDA_PIN_PORT,\
										PCF8591_I2C_SDA_PIN,GPIO_Speed_2MHz,GPIO_Mode_Out_PP
#define PCF8591_I2C_SDA_OUT				PBout(7)
#define PCF8591_I2C_SDA_IN_Init			PCF8591_I2C_SDA_PIN_PORT_CLK,\
										PCF8591_I2C_SDA_PIN_PORT,\
										PCF8591_I2C_SDA_PIN,GPIO_Speed_2MHz,GPIO_Mode_IPU
#define PCF8591_I2C_SDA_IN				PBin(7)

#define PCF8591_I2C_SCL_PIN_PORT		GPIOB
#define PCF8591_I2C_SCL_PIN_PORT_CLK	RCC_APB2Periph_GPIOB
#define PCF8591_I2C_SCL_PIN				GPIO_Pin_6
#define PCF8591_I2C_SCL_Init			PCF8591_I2C_SCL_PIN_PORT_CLK,\
										PCF8591_I2C_SCL_PIN_PORT,\
										PCF8591_I2C_SCL_PIN,GPIO_Speed_2MHz,GPIO_Mode_Out_PP
#define PCF8591_I2C_SCL					PBout(6)


#ifdef __cplusplus  /* C++支持 */
	extern "C"{
#endif
#include "systick.h"
#include "timer.h"
#include "sys.h"
#include "usart.h"	
#include "bsp_uart4.h"
#include "usart3.h"
#include "bsp_timer3.h"

/***********include app**********************/
#include "app_cloud.h"
int32_t	hal_init(void);
/*===========================================================================*/
#ifdef __cplusplus  /* C++支持 */
	}
#endif
/*===========================================================================*/
#endif

