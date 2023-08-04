/*************************************************************************
#    FileName: bsp_systick.h
#      Author: Allen
#       Email: qiurenguo@gmail.com
#    HomePage: Allen
#       Brief: 
#  LastChange: 2014-05-09 11:22:02
*************************************************************************/
/* Define to prevent recursive inclusion -------------------------------*/
#ifndef SYSTICK_H
#define SYSTICK_H

/* Includes ------------------------------------------------------------*/
/* Exported types ------------------------------------------------------*/
/* Exported constants --------------------------------------------------*/
/* Exported define -----------------------------------------------------*/
/* Exported variables --------------------------------------------------*/
extern volatile long jiffies;
/* Exported functions prototypes ---------------------------------------*/

void SYSTICK_init(void);
extern void SYSTICK_Configuration (void);
extern void SYSTICK_Delay10ms(long i);
extern void SYSTICK_ClearSystickVal (void);
extern long SYSTICK_GetVal(void);
extern long SYSTICK_get_time(void);


//extern void delay_ms(int ms);
//extern void delay_us(int us);

#endif 
/*********************************END OF FILE****************************/


