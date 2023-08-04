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
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "delay.h"
#include "WiFiToCloud.h"
#include "hardwareVariables.h"
#include "lora_app.h"

/*******************************************************************
*������int8_t ESP8266_SetStation(void)
*���ܣ�ESP8266����Ϊstationģʽ
*���룺��
*�����
		return = 0 ,sucess
		return < 0 ,error
*����˵����
*******************************************************************/
int8_t ESP8266_SetStation(void)
{
	ClrAtRxBuf();//��ջ���
	SendAtCmd((uint8_t *)AT_CWMODE,strlen(AT_CWMODE));
	delay_ms(100);
	if(strstr((const char *)AT_RX_BUF, (const char *)"OK") == NULL)
	{
		return -1;
	}
	return 0;
}

/*******************************************************************
*������int8_t ESP8266_SetAP(void)
*���ܣ�����ESP8266Ҫ���ӵ��ȵ����ƺ�����
*���룺char *wifi-�ȵ����� char *pwd-�ȵ�����
*�����
		return = 0 ,sucess
		return < 0 ,error
*����˵����
*******************************************************************/
int8_t ESP8266_SetAP(char *wifi, char *pwd)
{
	uint8_t AtCwjap[MAX_AT_TX_LEN];
	memset(AtCwjap, 0x00, MAX_AT_TX_LEN);//��ջ���
	ClrAtRxBuf();//��ջ���
	sprintf((char *)AtCwjap,"AT+CWJAP=\"%s\",\"%s\"",wifi, pwd);
//	printf("%s\r\n",AtCwjap);////////////////////////////////////////////////////////////
	SendAtCmd((uint8_t *)AtCwjap,strlen((const char *)AtCwjap));
	delay_ms(5500);
	if(strstr((const char *)AT_RX_BUF, (const char *)"OK") == NULL)
	{
		return -1;
	}
	return 0;
}

/*******************************************************************
*������int8_t ESP8266_SetStation(void)
*���ܣ�ESP8266����TCP����
*���룺
	char *IpAddr-IP��ַ�����磺120.77.58.34
	uint16_t port-�˿ںţ�ȡֵ0~65535
*�����
		return = 0 ,sucess
		return < 0 ,error
*����˵����
*******************************************************************/
int8_t ESP8266_IpStart(char *IpAddr, uint16_t port)
{
	uint8_t IpStart[MAX_AT_TX_LEN];
	memset(IpStart, 0x00, MAX_AT_TX_LEN);//��ջ���
	ClrAtRxBuf();//��ջ���
	
	/**************����MQTT_USER��Ϣ***************/
	//AT+MQTTUSERCFG=0,1,"�û�ID","�˺�","����",0,0,""
	                       //AT+MQTTUSERCFG=0,1,\"M3\",\"\",\"\",0,0,\"\"
	sprintf((char *)IpStart,"AT+MQTTUSERCFG=0,1,\"M3\",\"\",\"\",0,0,\"\"");
	SendAtCmd((uint8_t *)IpStart,strlen((const char *)IpStart));
	delay_ms(1500);
	printf("IpStart-USER_Config: %s\r\n", (char *)AT_RX_BUF);
	if(strstr((const char *)AT_RX_BUF, (const char *)"OK") == NULL)
	{
		/******�Ͽ�����*****/
		memset(IpStart, 0x00, MAX_AT_TX_LEN);//��ջ���
		ClrAtRxBuf();//��ջ���
		
		//AT+MQTTCLEAN=0
		sprintf((char *)IpStart,"AT+MQTTCLEAN=0");
		//printf("%s\r\n",IpStart);////////////////////////////////////////////////////////////
		SendAtCmd((uint8_t *)IpStart,strlen((const char *)IpStart));
		
		return -1;
	}
	
	/**************����MQTT����***************/
	memset(IpStart, 0x00, MAX_AT_TX_LEN);//��ջ���
	ClrAtRxBuf();//��ջ���
	
	//AT+MQTTCONN=0,"������IP",1883,0
	//AT+MQTTCONN=0,"192.168.43.245",1883,0
	sprintf((char *)IpStart,"AT+MQTTCONN=0,\"%s\",%d,0",IpAddr, port);
	//printf("%s\r\n",IpStart);////////////////////////////////////////////////////////////
	SendAtCmd((uint8_t *)IpStart,strlen((const char *)IpStart));
	delay_ms(1500);
	printf("IpStart-MQTT_Connect: %s\r\n", (char *)AT_RX_BUF);
	if(strstr((const char *)AT_RX_BUF, (const char *)"OK") == NULL)
	{
		return -2;
	}
	
	return 0;
}

/*******************************************************************
*������int8_t ESP8266_IpSend(char *IpBuf, uint8_t len)
*���ܣ�ESP8266��������
*���룺
		char *IpBuf-IP����
		uint8_t len-���ݳ���
*�����
		return = 0 ,sucess
		return < 0 ,error
*����˵����
*******************************************************************/
int8_t ESP8266_IpSend(char *IpBuf, uint8_t len)
{
	uint8_t TryGo = 0;
	int8_t error = 0;
	uint8_t IpSend[MAX_AT_TX_LEN];
	memset(IpSend, 0x00, MAX_AT_TX_LEN);//��ջ���
	ClrAtRxBuf();//��ջ���
	sprintf((char *)IpSend,"AT+CIPSEND=%d",len);
	//printf("%s\r\n",IpSend);////////////////////////////////////////////////////////////
	SendAtCmd((uint8_t *)IpSend,strlen((const char *)IpSend));
	delay_ms(3);
	if(strstr((const char *)AT_RX_BUF, (const char *)"OK") == NULL)
	{
		return -1;
	}
	ClrAtRxBuf();//��ջ���
	SendStrLen((uint8_t *)IpBuf, len);
	printf("IPsend: %s\r\n",IpBuf);////////////////////////////////////////////////////////////
	for(TryGo = 0; TryGo<60; TryGo++)//���ȴ�ʱ��100*60=6000ms
	{
		if(strstr((const char *)AT_RX_BUF, (const char *)"SEND OK") == NULL)
		{
			error = -2;
		}
		else
		{
			error = 0;
			break;
		}
		delay_ms(100);
	}
	return error;
}

/*******************************************************************
*������int8_t ConnectToServer(void)
*���ܣ����ӵ�������
*���룺��
*�����
		return = 0 ,sucess
		return < 0 ,error
*����˵����
*******************************************************************/
int8_t ConnectToServer(char *DeviceID, char *SecretKey)
{
	uint8_t TryGo = 0;
	int8_t error = 0;
	uint8_t TxetBuf[MAX_AT_TX_LEN];
	memset(TxetBuf,0x00,MAX_AT_TX_LEN);//��ջ���
	for(TryGo = 0; TryGo<3; TryGo++)//����WiFiͨѶģ�鹤��ģʽ
	{
		if(ESP8266_SetStation() == 0)//����WiFiͨѶģ�鹤��ģʽ
		{
			error = 0;
			break;
		}
		else
		{
			error = -1;
		}
	}
	
	//�޸�--��������վ��ģʽʧ�ܵ����
	error = 0;
	
	if(error < 0)
	{
		return error;
	}
	for(TryGo = 0; TryGo<3; TryGo++)//�����ȵ����ƺ�����
	{
		if(ESP8266_SetAP((char *)WIFI_AP, (char *)WIFI_PWD) == 0)//�����ȵ����ƺ�����
		{
			error = 0;
			break;
		}
		else
		{
			error = -2;
		}
	}
	if(error < 0)
	{
		return error;
	}
	wifi = 1;
	for(TryGo = 0; TryGo<3; TryGo++)//���ӷ�����
	{
		if(ESP8266_IpStart((char *)SERVER_PC_IP,SERVER_PC_PORT) == 0)//���ӷ�����IP��ַ���˿ڣ�120.77.58.34,8600
		{
			error = 0;
			break;
		}
		else
		{
			error = -3;
		}
	}
	if(error < 0)
	{
		return error;
	}
	
	//sprintf((char *)TxetBuf,"{\"t\":1,\"device\":\"%s\",\"key\":\"%s\",\"ver\":\"v0.0.0.0\"}",DeviceID,SecretKey);

	//printf("%s\r\n",TxetBuf);////////////////////////////////////////////////////////////
    int flag=0;int flag1=0;
	
	for(TryGo = 0; TryGo<3; TryGo++)//��������
	{
		if(ESP8266_MQTT_Sub("setM3") == 0)
		{
			flag = 1;
			
		}
       if(ESP8266_MQTT_Sub("setM4") == 0)
		{
			flag1= 1;
			break;
		}
        if(flag==1&&flag1==1)
        {
           error=0;
            break;
        
        }
		else
		{
			error = -4;
		}
	}
	if(error < 0)
	{
		return error;
	}

	#if 0
	if(ESP8266_MQTT_Sub("M3") < 0)//��������
	{//����ʧ��
		error=-4;
		printf("���ݷ���ʧ��\n\r");
	}
	else
	{//���ͳɹ�
		for(TryGo = 0; TryGo<50; TryGo++)//���ȴ�ʱ��50*10=500ms
		{
			if(strstr((const char *)AT_RX_BUF, (const char *)"\"status\":0") != NULL)//�����Ӧ״̬�Ƿ�Ϊ���ֳɹ�  ԭ���� == NULL
			{
				error = -5;
			}
			else
			{
				error = 0;
				
				printf("ŷ�ڵ��ֺú�\n\r");
				break;
			}
			delay_ms(10);
		}
	}
	#endif
	
	return error;
}

int8_t ESP8266_MQTT_Pub(char *IpBuf, uint8_t len, uint8_t qos)
{
	uint8_t TryGo = 0;
	int8_t error = 0;
	uint8_t IpSend[MAX_AT_TX_LEN];
	memset(IpSend, 0x00, MAX_AT_TX_LEN);//��ջ���
	ClrAtRxBuf();//��ջ���
	/************����ΪMQTT_Binary����ģʽ************/
	sprintf((char *)IpSend,"AT+MQTTPUBRAW=0,\"M3\",%d,%d,0", len, qos);
	//printf("%s\r\n",IpSend);////////////////////////////////////////////////////////////
	SendAtCmd((uint8_t *)IpSend,strlen((const char *)IpSend));
	delay_ms(100);
	//ע��--����Ͽ����ӻ�����ô������һ��ESP8266�ͻ᷵��ERROR
	if(strstr((const char *)AT_RX_BUF, (const char *)"OK") == NULL)
	{
		printf("MQTT_Pub Fail:%s\r\n", AT_RX_BUF);
		MQTTSendErrorTimes++;
		return -1;
	}
	
	ClrAtRxBuf();//��ջ���
	
	//�Զ�����������ʽ�����ַ���
	SendAtCmd((uint8_t *)IpBuf, len);
	printf("MQTT_Pub: %s\r\n", IpBuf);
	if(qos == 0){
		return 0;
	}
	
	//�ȴ�qos��ִ����
	delay_ms(100);
	if(strstr((const char *)AT_RX_BUF, (const char *)"OK") == NULL)
	{
		MQTTSendErrorTimes++;
		return -2;
	}
	return error;
}

int8_t ESP8266_MQTT_Sub(char *topic)
{
	//uint8_t TryGo = 0;
	int8_t error = 0;
	uint8_t IpSend[MAX_AT_TX_LEN];
	memset(IpSend, 0x00, MAX_AT_TX_LEN);//��ջ���
	ClrAtRxBuf();//��ջ���
	
	
	//AT+MQTTSUB=0,M3",0
	sprintf((char *)IpSend,"AT+MQTTSUB=0,\"%s\",1", topic);
	//printf("%s\r\n",IpSend);////////////////////////////////////////////////////////////
	SendAtCmd((uint8_t *)IpSend,strlen((const char *)IpSend));
	delay_ms(1500);
	printf("MQTT_Sub_Ack: %s\r\n",AT_RX_BUF);
	if(strstr((const char *)AT_RX_BUF, (const char *)"OK") == NULL)
	{
		return -4;
	}
	
	return error;
}



/*******************************************************************
*������int8_t ESP8266_SendSensor(uint8_t sensor)
*���ܣ����ʹ������ݵ�������
*���룺
	uint8_t sensor, ������ֵ
	char *TimeStr���ɼ�����ʱ��ʱ�䣬����yyyy-mm-dd hh:mm:ss��ʽ
*�����
		return = 0 ,sucess
		return < 0 ,error
*����˵����
*******************************************************************/
int8_t ESP8266_SendSensor(uint8_t sensor, char *TimeStr)
{
	uint8_t TryGo = 0;
	int8_t error = 0;
	uint8_t TxetBuf[MAX_AT_TX_LEN];
	memset(TxetBuf,0x00,MAX_AT_TX_LEN);//��ջ���
	sprintf((char *)TxetBuf,"{\"t\":3,\"datatype\":1,\"datas\":{\"alarm\":{\"%s\":%d}},\"msgid\":001}",TimeStr,sensor);
	//printf("%s\r\n",TxetBuf);////////////////////////////////////////////////////////////
	if(ESP8266_IpSend((char *)TxetBuf, strlen((char *)TxetBuf)) < 0)
	{//����ʧ��
		error=-1;
	}
	else
	{//���ͳɹ�
		for(TryGo = 0; TryGo<50; TryGo++)//���ȴ�ʱ��50*10=500ms
		{
			if(strstr((const char *)AT_RX_BUF, (const char *)"\"status\":0") == NULL)//�����Ӧ״̬�Ƿ�Ϊ"�ϱ��ɹ�"
			{
				error = -1;
			}
			else
			{
				error = 0;
				break;
			}
			delay_ms(10);
		}
	}
	return error;
}

/*******************************************************************
*������uint8_t ESP8266_GetIpData(uint8_t *AtRxBuf, char *GetIpData)
*���ܣ���������������
*���룺
		uint8_t *AtRxBuf ��ԭʼAT���ڻ���
		char *GetIpData ����ȡ������Itcp/ip����
*����������յ���IP���ݳ���
*����˵����
	AT+CIPSEND=76
	{"t":3,"datatype":2,"datas":{"alarm":{"2018-06-19 18:15:02":0}},"msgid":001}
	+IPD,29:{"msgid":1,"status":0,"t":4}
*******************************************************************/
uint8_t ESP8266_GetIpData(uint8_t *AtRxBuf, char *GetIpData)
{
	char *Point = NULL;
	uint8_t len = 0;
	
	Point = strstr((const char *)AtRxBuf, (const char *)"+IPD,");
	if(Point != NULL)//���ģ���յ�TCP/IP���ݰ���
	{
		len=(Point[5]-'0')*10+Point[6]-'0';//����IP���ݳ��Ȳ��ᳬ��100���ʿ��԰�����λ���͡�ʮλ�������յ���IP���ݳ���
		Point = strstr((const char *)AtRxBuf, (const char *)":")+1;
		memcpy(GetIpData, Point, len);
		GetIpData[len]='\0';
		printf("�յ�IP���ݣ�%s\r\n",GetIpData);///////////////////////////////////////////////////////
		printf("�յ�IP���ݳ���=%d\r\n",len);///////////////////////////////////////////////////
	}
	
	return (len);
}

uint8_t ESP8266_Get_MQTT_Data(uint8_t *AtRxBuf, char *GetIpData)
{
	char *point = NULL;
	char *lengthStartIndex = NULL;
	char *lengthEndIndex = NULL;
	char *dataIndex = NULL;
	uint8_t len = 0;
	uint8_t lenLength = 0;
	
	//��Ӧ����ʾ����+MQTTSUBRECV:0,"setM3",12,wuhudasima //���� ���� ����
	
	if(strstr((const char *)AtRxBuf, (const char *)"setM3") == NULL){
		return -1;
	}
	
	//�ҵ�������setM3��λ��
	point = strstr((const char *)AtRxBuf, (const char *)"setM3");
	if(point == NULL){
		return -2;
	}
	//�ҵ�length����ʼλ��
	lengthStartIndex = strstr((const char *)point, (const char *)",");
	if(lengthStartIndex == NULL){
		return -3;
	}
	lengthStartIndex++;
	//�ҵ�length����λ��
	lengthEndIndex = strstr((const char *)lengthStartIndex, (const char *)",");
	if(lengthEndIndex == NULL){
		return -4;
	}
	lenLength = lengthEndIndex - lengthStartIndex;
	//���ݱ���ʼλ��
	dataIndex = lengthEndIndex + 1;
	
	switch(lenLength){
		case 1:len = lengthStartIndex[0]-'0';break;
		case 2:len = (lengthStartIndex[0]-'0')*10 + (lengthStartIndex[1]-'0');break;
		case 3:len = (lengthStartIndex[0]-'0')*100 + (lengthStartIndex[1]-'0')*10 + (lengthStartIndex[2]-'0');break;
		default: return -5;
	}
//	printf("�յ�����Ϊ: %s\r\n", dataIndex);
//	printf("�յ����ݳ���Ϊ: %d\r\n", len);
	
	
	memcpy(GetIpData, dataIndex, len);
	#if 0
	if(lenLength == 2)//���ģ���յ�TCP/IP���ݰ���
	{
		len=(Point[5]-'0')*10+Point[6]-'0';//����IP���ݳ��Ȳ��ᳬ��100���ʿ��԰�����λ���͡�ʮλ�������յ���IP���ݳ���
		Point = strstr((const char *)AtRxBuf, (const char *)":")+1;
		memcpy(GetIpData, Point, len);
		GetIpData[len]='\0';
		printf("�յ�IP���ݣ�%s\r\n",GetIpData);///////////////////////////////////////////////////////
		printf("�յ�IP���ݳ���=%d\r\n",len);///////////////////////////////////////////////////
	}
	#endif
	
	return (len);
}


/*******************************************************************
*������void ESP8266_DataAnalysisProcess(char *RxBuf)
*���ܣ���������������
*���룺char *RxBuf �������·�����
*�����
*����˵�����û������ڴ˻����ϸ������չ�ú���������ֻ�Ǹ��򵥵�DEMO
*******************************************************************/
void ESP8266_DataAnalysisProcess(char *RxBuf)
{
	if(strstr((const char *)RxBuf, (const char *)PING_REQ) != NULL)//��������
	{
		ESP8266_IpSend((char *)PING_RSP, strlen((const char *)PING_RSP));//��Ӧ����
	}
	else if(strstr((const char *)RxBuf, (const char *)"\"t\":5") != NULL)//��������
	{
		if(strstr((const char *)RxBuf, (const char *)"\"apitag\":\"ctrl\"") != NULL)//��������
		{
			if((strstr((const char *)RxBuf, (const char *)"\"data\":1") != NULL))//����
			{
				ESP8266_IpSend((char *)PING_RSP, strlen((const char *)PING_RSP));//��Ӧ����
			}
		}
		else if(strstr((const char *)RxBuf, (const char *)"\"apitag\":\"defense\"") != NULL)//����/��������
		{
			if((strstr((const char *)RxBuf, (const char *)"\"data\":1") != NULL))//����
			{
				printf("������\r\n");
				;//...
				;//...
				;//...
			}
			else if((strstr((const char *)RxBuf, (const char *)"\"data\":0") != NULL))//����
			{
				printf("������\r\n");
				;//...
				;//...
				;//...
			}
		}
	}
}
