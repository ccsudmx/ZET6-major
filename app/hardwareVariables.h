#ifndef __HARDWARE_VARIABLES__
#define __HARDWARE_VARIABLES__

/******************************************************************/

//wifi模块 -- 此部分弃用，直接使用了wifi和sever两个变量名
extern volatile int wifiConnectStatus;
extern volatile int serverConnectStatus;


int getAllDataAttribute(char* s);
//解析的值都不加双引号
void allToJSON(int length, char* msg, ...);

//解析的值都加双引号
void allToJSON2(int length, char* msg, ...);

//往离线记录中存入一条数据
void recordOfflineMessage();
//从离线记录中取出第一条数据-----弃用
void readOfflineMessage();

//阈值
extern volatile int minT1,maxT1; //default
extern volatile int minT2,maxT2;
extern volatile int minH1,maxH1; //default
extern volatile int minH2,maxH2;
extern volatile int minI1,maxI1; //default
extern volatile int minI2,maxI2;
extern volatile int minS1,maxS1; //default
extern volatile int minS2,maxS2;
extern volatile int autoMode; //Mappedd to auto

//可调灯 x1
extern volatile int lightStatus,lightPWM;
//风扇 x2
extern volatile int fanStatus1,fanSpeed1;
extern volatile int fanStatus2,fanSpeed2;
//步进电机 x2
extern volatile int motorStatus1,motorSpeed1,motorDirection1;
extern volatile int motorStatus2,motorSpeed2,motorDirection2;
//红外对射开关 x2
extern volatile int infraredStatus1;
extern volatile int infraredStatus2;
//蜂鸣器 x1
extern volatile int buzzerStatus;
//LED灯 x4
extern volatile int LEDStatus1,LEDStatus2,LEDStatus3,LEDStatus4;
//温湿度传感器
extern volatile int h1,h2,t1,t2;
//光照传感器 x2
extern volatile int i1,i2;
//烟雾传感器 x2
extern volatile int s1,s2;

//警报
extern volatile int alarm;
extern volatile int alarmFlag;//0正常，-1异常
extern volatile int sAlarm, hAlarm;
//刷卡类型
extern volatile int idType;//1为peopleID 2为bookId 3为readerID（2、3统称RFID）
extern volatile int identifyFlag;//0通过，-1不通过

//MQTT发送失败次数
extern int MQTTSendErrorTimes;
//在线状态标志位
extern volatile int onlineCode;

//箱内外温差
extern volatile int TG;

//光照存储段
extern volatile int lightAuto;

//设备状态信息


#define OFFLINE_DEFAULT_COUNTS 100
//离线存储的数据消息条数
extern volatile int offlineMessageCounts;
//离线存储的数据消息 i1  i2  h1  h2  t1  t2  s1  s2  infraredStatus1
extern int offlineMessage[9][OFFLINE_DEFAULT_COUNTS];

#endif
