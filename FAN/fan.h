#ifndef __FAN_H__
#define __FAN_H__
#include "sys.h"


#define FAN 1
#define LIGHT 1
#define SENSOR 4 // 1 or 2

#if 0
#define MQ_2 1  //1 or 2
#define DHT11 1 //1 or 2
#define GY302 1 //1 or 2
#endif



//��ʪ�ȴ�����
extern int SendFlag;
extern  u8 humidity,temperature;
//���մ����� x2
extern  int illumination;
//�������� x2
extern  int smoke;

void TIM1_PWM_Init(u16 arr,u16 psc);
void TIM2_Config(u16 arr,u16 psc);

#endif
