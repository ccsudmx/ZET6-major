#ifndef _TIMER_H
#define _TIMER_H
#include "sys.h"
	

void TIM7_Int_Init(u16 arr,u16 psc);
void TIM4_Int_Init(u16 arr,u16 psc);
void TIM5_Int_Init(u16 arr,u16 psc);
void TIM5_SetARR(u16 period);
void TIM4_SetARR(u16 period);
#endif
