#ifndef __HARDWARE_VARIABLES__
#define __HARDWARE_VARIABLES__

/******************************************************************/

//wifiģ�� -- �˲������ã�ֱ��ʹ����wifi��sever����������
extern volatile int wifiConnectStatus;
extern volatile int serverConnectStatus;


int getAllDataAttribute(char* s);
//������ֵ������˫����
void allToJSON(int length, char* msg, ...);

//������ֵ����˫����
void allToJSON2(int length, char* msg, ...);

//�����߼�¼�д���һ������
void recordOfflineMessage();
//�����߼�¼��ȡ����һ������-----����
void readOfflineMessage();

//��ֵ
extern volatile int minT1,maxT1; //default
extern volatile int minT2,maxT2;
extern volatile int minH1,maxH1; //default
extern volatile int minH2,maxH2;
extern volatile int minI1,maxI1; //default
extern volatile int minI2,maxI2;
extern volatile int minS1,maxS1; //default
extern volatile int minS2,maxS2;
extern volatile int autoMode; //Mappedd to auto

//�ɵ��� x1
extern volatile int lightStatus,lightPWM;
//���� x2
extern volatile int fanStatus1,fanSpeed1;
extern volatile int fanStatus2,fanSpeed2;
//������� x2
extern volatile int motorStatus1,motorSpeed1,motorDirection1;
extern volatile int motorStatus2,motorSpeed2,motorDirection2;
//������俪�� x2
extern volatile int infraredStatus1;
extern volatile int infraredStatus2;
//������ x1
extern volatile int buzzerStatus;
//LED�� x4
extern volatile int LEDStatus1,LEDStatus2,LEDStatus3,LEDStatus4;
//��ʪ�ȴ�����
extern volatile int h1,h2,t1,t2;
//���մ����� x2
extern volatile int i1,i2;
//�������� x2
extern volatile int s1,s2;

//����
extern volatile int alarm;
extern volatile int alarmFlag;//0������-1�쳣
extern volatile int sAlarm, hAlarm;
//ˢ������
extern volatile int idType;//1ΪpeopleID 2ΪbookId 3ΪreaderID��2��3ͳ��RFID��
extern volatile int identifyFlag;//0ͨ����-1��ͨ��

//MQTT����ʧ�ܴ���
extern int MQTTSendErrorTimes;
//����״̬��־λ
extern volatile int onlineCode;

//�������²�
extern volatile int TG;

//���մ洢��
extern volatile int lightAuto;

//�豸״̬��Ϣ


#define OFFLINE_DEFAULT_COUNTS 100
//���ߴ洢��������Ϣ����
extern volatile int offlineMessageCounts;
//���ߴ洢��������Ϣ i1  i2  h1  h2  t1  t2  s1  s2  infraredStatus1
extern int offlineMessage[9][OFFLINE_DEFAULT_COUNTS];

#endif
