

#ifndef RTC_H
#define RTC_H
#include "stdint.h"
#include "stm32f10x.h"

typedef void (*RTC_CALLBACK_FUNC)(uint32_t rtc_counter_val);
typedef struct 
{
	vu8 hour;
	vu8 min;
	vu8 sec;			
	vu16 w_year;
	vu8  w_month;
	vu8  w_date;
	vu8  week;		 
}_calendar_obj;					 
extern _calendar_obj calendar;	//ии?иди▓?ив11им?

void rtc_init(RTC_CALLBACK_FUNC cbp);
void Time_Adjust(uint16_t syear,uint8_t smon, uint8_t sday, uint8_t hour, uint8_t minute, uint8_t second);
u8 RTC_Get(void);

#endif
