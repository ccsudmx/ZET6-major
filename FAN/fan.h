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

extern int SendFlag;
extern int timesFlag;

extern volatile int lightStatus,lightPWM,lightVal;
//���� x2
extern volatile int fanStatus1,fanSpeed1,fanVal1;
extern volatile int fanStatus2,fanSpeed2,fanVal2;

//��ʪ�ȴ�����
extern volatile u8 humidity,temperature;
//���մ����� x2
extern volatile int illumination;
//�������� x2
extern volatile int smoke;

void TIM1_PWM_Init(u16 arr,u16 psc);
void TIM2_Config(u16 arr,u16 psc);

#endif
