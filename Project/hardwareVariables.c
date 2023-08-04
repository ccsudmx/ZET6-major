#include "stdarg.h"
#include "string.h"
#include "stdio.h"
#include "hardwareVariables.h"
#include "stdlib.h"

//阈值
volatile int minT1, maxT1; //default
volatile int minT2, maxT2;
volatile int minH1, maxH1; //default
volatile int minH2, maxH2;
volatile int minI1, maxI1; //default
volatile int minI2, maxI2;
volatile int minS1,maxS1; //default
volatile int minS2,maxS2;
volatile int autoMode; //Mappedd to auto


//警报
volatile int alarm = 1;
volatile int alarmFlag = 0;
//刷卡类型
volatile int idType = 0;
volatile int identifyFlag = -1;

//MQTT发送失败次数
int MQTTSendErrorTimes = 0;
//在线状态标志位
volatile int onlineCode = 0;
//离线存储的数据消息条数
volatile int offlineMessageCounts = 0;
//离线存储的数据消息
int offlineMessage[9][100];

//光照存储段
volatile int lightAuto = 0;
//箱内外温差
volatile int TG = 0;

volatile int sAlarm = 0;
volatile int hAlarm = 0;

int getAllDataAttribute(char* s)
{
	if (strcmp(s, "minT1") == 0)	  //温度阈值
		return minT1;
	else if (strcmp(s,"maxT1") == 0)
		return maxT1;
	else if (strcmp(s,"minT2") == 0)
		return minT2;
	else if (strcmp(s,"maxT2") == 0)
		return maxT2;
	else if (strcmp(s,"minH1") == 0)  //湿度阈值
		return minH1;
	else if (strcmp(s,"maxH1") == 0)
		return maxH1;
	else if (strcmp(s,"minH2") == 0)
		return minH2;
	else if (strcmp(s,"maxH2") == 0)
		return maxH2;
	else if (strcmp(s,"minI1") == 0)  //光照阈值
		return minI1;
	else if (strcmp(s,"maxI1") == 0)
		return maxI1;
	else if (strcmp(s,"minI2") == 0)
		return minI2;
	else if (strcmp(s,"maxI2") == 0)
		return maxI2;
	else if (strcmp(s,"minS1") == 0)  //烟雾阈值
		return minS1;
	else if (strcmp(s,"maxS1") == 0)
		return maxS1;
	else if (strcmp(s,"minS2") == 0)
		return minS2;
	else if (strcmp(s,"maxS2") == 0)
		return maxS2;
	else if (strcmp(s,"auto") == 0)        //模式阈值
		return autoMode;
	else if (strcmp(s,"lightStatus") == 0)     //可调灯
		return lightStatus;
	else if (strcmp(s,"lightPWM") == 0)
		return lightPWM;
	else if (strcmp(s,"fanStatus1") == 0)      //风扇
		return fanStatus1;
	else if (strcmp(s,"fanSpeed1") == 0)
		return fanSpeed1;
	else if (strcmp(s,"fanStatus2") == 0)
		return fanStatus2;
	else if (strcmp(s,"fanSpeed2") == 0)
		return fanSpeed2;
	else if (strcmp(s,"motorStatus1") == 0)    //步进电机
		return motorStatus1;
	else if (strcmp(s,"motorSpeed1") == 0)
		return motorSpeed1;
	else if (strcmp(s,"motorDirection1") == 0)
		return motorDirection1;
	else if (strcmp(s,"motorStatus2") == 0)
		return motorStatus2;
	else if (strcmp(s,"motorSpeed2") == 0)
		return motorSpeed2;
	else if (strcmp(s,"motorDirection2") == 0)
		return motorDirection2;
	else if (strcmp(s,"infraredStatus1") == 0) //红外传感器
		return infraredStatus1;
	else if (strcmp(s,"infraredStatus2") == 0)
		return infraredStatus2;
	else if (strcmp(s,"buzzerStatus") == 0)    //蜂鸣器
		return buzzerStatus;
	else if (strcmp(s,"LEDStatus1") == 0)      //LED灯
		return LEDStatus1;
	else if (strcmp(s,"LEDStatus2") == 0)
		return LEDStatus2;
	else if (strcmp(s,"LEDStatus3") == 0)
		return LEDStatus3;
	else if (strcmp(s,"LEDStatus4") == 0)
		return LEDStatus4;
	else if (strcmp(s,"h1") == 0)			   //湿度传感器
		return h1;
	else if (strcmp(s,"h2") == 0)
		return h2;
	else if (strcmp(s,"t1") == 0)			   //温度传感器
		return t1;
	else if (strcmp(s,"t2") == 0)
		return t2;
	else if (strcmp(s,"i1") == 0)			   //光照传感器
		return i1;
	else if (strcmp(s,"i2") == 0)
		return i2;
	else if (strcmp(s,"s1") == 0)			   //烟雾传感器
		return s1;
	else if (strcmp(s,"s2") == 0)
		return s2;
	else if (strcmp(s, "onlineCode") == 0)
		return onlineCode;

	return -999;
}

void allToJSON(int length, char* msg, ...)
{
	va_list ap;
	char* p, * sval;
	int index = 0;
	char strCell[25];

	va_start(ap, msg);

	sprintf(msg, "{");
	while (length > 0) {
		sval = va_arg(ap, char*); 
		sprintf(strCell, "\"%s\":%d,", sval, getAllDataAttribute(sval));
		strcat(msg, strCell);
		length--;
	}
	int endIndex = strlen(msg);
	*(msg + endIndex -1) = '}';

	va_end(ap);
}

void allToJSON2(int length, char* msg, ...)
{
	va_list ap;
	char* p, * sval;
	int index = 0;
	char strCell[25];

	va_start(ap, msg);

	sprintf(msg, "{");
	while (length > 0) {
		sval = va_arg(ap, char*); 
		sprintf(strCell, "\"%s\":\"%d\",", sval, getAllDataAttribute(sval));
		strcat(msg, strCell);
		length--;
	}
	int endIndex = strlen(msg);
	*(msg + endIndex -1) = '}';

	va_end(ap);
}



void demo()
{
	char s[256];

	allToJSON(4, s, "minT1", "maxT1", "motorDirection2", "motorDirection2");
	printf("%s", s);
}
char vName[9][16] = {"i1", "i2", "h1", "h2", "t1", "t2", "s1", "s2", "infraredStatus1"};
void recordOfflineMessage()
{
	if(onlineCode == 0 && offlineMessageCounts < OFFLINE_DEFAULT_COUNTS){ //存储离线数据  i1  i2  h1  h2  t1  t2  s1  s2  infraredStatus1
		
		printf("存储离线数据 %d \r\n", offlineMessageCounts);
		
		for(int i = 0; i < 9; i++){
			offlineMessage[offlineMessageCounts][i] = getAllDataAttribute(vName[i]);
		}
		offlineMessageCounts++;
	}
}
//弃用
void readOfflineMessage()
{
	#if 0
	if(onlineCode == 0 && offlineMessageCounts > 0){ //存储离线数据  i1  i2  h1  h2  t1  t2  s1  s2  infraredStatus1
		for(int i = 0; i < 9; i++){
			offlineMessage[offlineMessageCounts][i] = getAllDataAttribute(vName[i]);
		}
		offlineMessageCounts++;
	}
	#endif
}

