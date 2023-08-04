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
*函数：int8_t ESP8266_SetStation(void)
*功能：ESP8266设置为station模式
*输入：无
*输出：
		return = 0 ,sucess
		return < 0 ,error
*特殊说明：
*******************************************************************/
int8_t ESP8266_SetStation(void)
{
	ClrAtRxBuf();//清空缓存
	SendAtCmd((uint8_t *)AT_CWMODE,strlen(AT_CWMODE));
	delay_ms(100);
	if(strstr((const char *)AT_RX_BUF, (const char *)"OK") == NULL)
	{
		return -1;
	}
	return 0;
}

/*******************************************************************
*函数：int8_t ESP8266_SetAP(void)
*功能：设置ESP8266要连接的热点名称和密码
*输入：char *wifi-热点名称 char *pwd-热点密码
*输出：
		return = 0 ,sucess
		return < 0 ,error
*特殊说明：
*******************************************************************/
int8_t ESP8266_SetAP(char *wifi, char *pwd)
{
	uint8_t AtCwjap[MAX_AT_TX_LEN];
	memset(AtCwjap, 0x00, MAX_AT_TX_LEN);//清空缓存
	ClrAtRxBuf();//清空缓存
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
*函数：int8_t ESP8266_SetStation(void)
*功能：ESP8266建立TCP连接
*输入：
	char *IpAddr-IP地址，例如：120.77.58.34
	uint16_t port-端口号，取值0~65535
*输出：
		return = 0 ,sucess
		return < 0 ,error
*特殊说明：
*******************************************************************/
int8_t ESP8266_IpStart(char *IpAddr, uint16_t port)
{
	uint8_t IpStart[MAX_AT_TX_LEN];
	memset(IpStart, 0x00, MAX_AT_TX_LEN);//清空缓存
	ClrAtRxBuf();//清空缓存
	
	/**************配置MQTT_USER信息***************/
	//AT+MQTTUSERCFG=0,1,"用户ID","账号","密码",0,0,""
	                       //AT+MQTTUSERCFG=0,1,\"M3\",\"\",\"\",0,0,\"\"
	sprintf((char *)IpStart,"AT+MQTTUSERCFG=0,1,\"M3\",\"\",\"\",0,0,\"\"");
	SendAtCmd((uint8_t *)IpStart,strlen((const char *)IpStart));
	delay_ms(1500);
	printf("IpStart-USER_Config: %s\r\n", (char *)AT_RX_BUF);
	if(strstr((const char *)AT_RX_BUF, (const char *)"OK") == NULL)
	{
		/******断开连接*****/
		memset(IpStart, 0x00, MAX_AT_TX_LEN);//清空缓存
		ClrAtRxBuf();//清空缓存
		
		//AT+MQTTCLEAN=0
		sprintf((char *)IpStart,"AT+MQTTCLEAN=0");
		//printf("%s\r\n",IpStart);////////////////////////////////////////////////////////////
		SendAtCmd((uint8_t *)IpStart,strlen((const char *)IpStart));
		
		return -1;
	}
	
	/**************建立MQTT连接***************/
	memset(IpStart, 0x00, MAX_AT_TX_LEN);//清空缓存
	ClrAtRxBuf();//清空缓存
	
	//AT+MQTTCONN=0,"服务器IP",1883,0
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
*函数：int8_t ESP8266_IpSend(char *IpBuf, uint8_t len)
*功能：ESP8266发送数据
*输入：
		char *IpBuf-IP数据
		uint8_t len-数据长度
*输出：
		return = 0 ,sucess
		return < 0 ,error
*特殊说明：
*******************************************************************/
int8_t ESP8266_IpSend(char *IpBuf, uint8_t len)
{
	uint8_t TryGo = 0;
	int8_t error = 0;
	uint8_t IpSend[MAX_AT_TX_LEN];
	memset(IpSend, 0x00, MAX_AT_TX_LEN);//清空缓存
	ClrAtRxBuf();//清空缓存
	sprintf((char *)IpSend,"AT+CIPSEND=%d",len);
	//printf("%s\r\n",IpSend);////////////////////////////////////////////////////////////
	SendAtCmd((uint8_t *)IpSend,strlen((const char *)IpSend));
	delay_ms(3);
	if(strstr((const char *)AT_RX_BUF, (const char *)"OK") == NULL)
	{
		return -1;
	}
	ClrAtRxBuf();//清空缓存
	SendStrLen((uint8_t *)IpBuf, len);
	printf("IPsend: %s\r\n",IpBuf);////////////////////////////////////////////////////////////
	for(TryGo = 0; TryGo<60; TryGo++)//最多等待时间100*60=6000ms
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
*函数：int8_t ConnectToServer(void)
*功能：连接到服务器
*输入：无
*输出：
		return = 0 ,sucess
		return < 0 ,error
*特殊说明：
*******************************************************************/
int8_t ConnectToServer(char *DeviceID, char *SecretKey)
{
	uint8_t TryGo = 0;
	int8_t error = 0;
	uint8_t TxetBuf[MAX_AT_TX_LEN];
	memset(TxetBuf,0x00,MAX_AT_TX_LEN);//清空缓存
	for(TryGo = 0; TryGo<3; TryGo++)//设置WiFi通讯模块工作模式
	{
		if(ESP8266_SetStation() == 0)//设置WiFi通讯模块工作模式
		{
			error = 0;
			break;
		}
		else
		{
			error = -1;
		}
	}
	
	//修改--跳过设置站点模式失败的情况
	error = 0;
	
	if(error < 0)
	{
		return error;
	}
	for(TryGo = 0; TryGo<3; TryGo++)//设置热点名称和密码
	{
		if(ESP8266_SetAP((char *)WIFI_AP, (char *)WIFI_PWD) == 0)//设置热点名称和密码
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
	for(TryGo = 0; TryGo<3; TryGo++)//连接服务器
	{
		if(ESP8266_IpStart((char *)SERVER_PC_IP,SERVER_PC_PORT) == 0)//连接服务器IP地址，端口：120.77.58.34,8600
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
	
	for(TryGo = 0; TryGo<3; TryGo++)//订阅主题
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
	if(ESP8266_MQTT_Sub("M3") < 0)//发送数据
	{//发送失败
		error=-4;
		printf("数据发送失败\n\r");
	}
	else
	{//发送成功
		for(TryGo = 0; TryGo<50; TryGo++)//最多等待时间50*10=500ms
		{
			if(strstr((const char *)AT_RX_BUF, (const char *)"\"status\":0") != NULL)//检查响应状态是否为握手成功  原来是 == NULL
			{
				error = -5;
			}
			else
			{
				error = 0;
				
				printf("欧内的手好汉\n\r");
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
	memset(IpSend, 0x00, MAX_AT_TX_LEN);//清空缓存
	ClrAtRxBuf();//清空缓存
	/************设置为MQTT_Binary发送模式************/
	sprintf((char *)IpSend,"AT+MQTTPUBRAW=0,\"M3\",%d,%d,0", len, qos);
	//printf("%s\r\n",IpSend);////////////////////////////////////////////////////////////
	SendAtCmd((uint8_t *)IpSend,strlen((const char *)IpSend));
	delay_ms(100);
	//注意--如果断开连接或者怎么样，这一步ESP8266就会返回ERROR
	if(strstr((const char *)AT_RX_BUF, (const char *)"OK") == NULL)
	{
		printf("MQTT_Pub Fail:%s\r\n", AT_RX_BUF);
		MQTTSendErrorTimes++;
		return -1;
	}
	
	ClrAtRxBuf();//清空缓存
	
	//以二进制数据形式发送字符串
	SendAtCmd((uint8_t *)IpBuf, len);
	printf("MQTT_Pub: %s\r\n", IpBuf);
	if(qos == 0){
		return 0;
	}
	
	//等待qos回执处理
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
	memset(IpSend, 0x00, MAX_AT_TX_LEN);//清空缓存
	ClrAtRxBuf();//清空缓存
	
	
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
*函数：int8_t ESP8266_SendSensor(uint8_t sensor)
*功能：发送传感数据到服务器
*输入：
	uint8_t sensor, 传感数值
	char *TimeStr，采集数据时的时间，须是yyyy-mm-dd hh:mm:ss格式
*输出：
		return = 0 ,sucess
		return < 0 ,error
*特殊说明：
*******************************************************************/
int8_t ESP8266_SendSensor(uint8_t sensor, char *TimeStr)
{
	uint8_t TryGo = 0;
	int8_t error = 0;
	uint8_t TxetBuf[MAX_AT_TX_LEN];
	memset(TxetBuf,0x00,MAX_AT_TX_LEN);//清空缓存
	sprintf((char *)TxetBuf,"{\"t\":3,\"datatype\":1,\"datas\":{\"alarm\":{\"%s\":%d}},\"msgid\":001}",TimeStr,sensor);
	//printf("%s\r\n",TxetBuf);////////////////////////////////////////////////////////////
	if(ESP8266_IpSend((char *)TxetBuf, strlen((char *)TxetBuf)) < 0)
	{//发送失败
		error=-1;
	}
	else
	{//发送成功
		for(TryGo = 0; TryGo<50; TryGo++)//最多等待时间50*10=500ms
		{
			if(strstr((const char *)AT_RX_BUF, (const char *)"\"status\":0") == NULL)//检查响应状态是否为"上报成功"
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
*函数：uint8_t ESP8266_GetIpData(uint8_t *AtRxBuf, char *GetIpData)
*功能：解析服务器数据
*输入：
		uint8_t *AtRxBuf ，原始AT串口缓存
		char *GetIpData ，截取出来的Itcp/ip数据
*输出：返回收到的IP数据长度
*特殊说明：
	AT+CIPSEND=76
	{"t":3,"datatype":2,"datas":{"alarm":{"2018-06-19 18:15:02":0}},"msgid":001}
	+IPD,29:{"msgid":1,"status":0,"t":4}
*******************************************************************/
uint8_t ESP8266_GetIpData(uint8_t *AtRxBuf, char *GetIpData)
{
	char *Point = NULL;
	uint8_t len = 0;
	
	Point = strstr((const char *)AtRxBuf, (const char *)"+IPD,");
	if(Point != NULL)//检查模块收到TCP/IP数据包？
	{
		len=(Point[5]-'0')*10+Point[6]-'0';//这里IP数据长度不会超过100，故可以按“个位”和“十位”计算收到的IP数据长度
		Point = strstr((const char *)AtRxBuf, (const char *)":")+1;
		memcpy(GetIpData, Point, len);
		GetIpData[len]='\0';
		printf("收到IP数据：%s\r\n",GetIpData);///////////////////////////////////////////////////////
		printf("收到IP数据长度=%d\r\n",len);///////////////////////////////////////////////////
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
	
	//响应数据示例：+MQTTSUBRECV:0,"setM3",12,wuhudasima //主题 长度 内容
	
	if(strstr((const char *)AtRxBuf, (const char *)"setM3") == NULL){
		return -1;
	}
	
	//找到第主题setM3的位置
	point = strstr((const char *)AtRxBuf, (const char *)"setM3");
	if(point == NULL){
		return -2;
	}
	//找到length的起始位置
	lengthStartIndex = strstr((const char *)point, (const char *)",");
	if(lengthStartIndex == NULL){
		return -3;
	}
	lengthStartIndex++;
	//找到length结束位置
	lengthEndIndex = strstr((const char *)lengthStartIndex, (const char *)",");
	if(lengthEndIndex == NULL){
		return -4;
	}
	lenLength = lengthEndIndex - lengthStartIndex;
	//数据报起始位置
	dataIndex = lengthEndIndex + 1;
	
	switch(lenLength){
		case 1:len = lengthStartIndex[0]-'0';break;
		case 2:len = (lengthStartIndex[0]-'0')*10 + (lengthStartIndex[1]-'0');break;
		case 3:len = (lengthStartIndex[0]-'0')*100 + (lengthStartIndex[1]-'0')*10 + (lengthStartIndex[2]-'0');break;
		default: return -5;
	}
//	printf("收到数据为: %s\r\n", dataIndex);
//	printf("收到数据长度为: %d\r\n", len);
	
	
	memcpy(GetIpData, dataIndex, len);
	#if 0
	if(lenLength == 2)//检查模块收到TCP/IP数据包？
	{
		len=(Point[5]-'0')*10+Point[6]-'0';//这里IP数据长度不会超过100，故可以按“个位”和“十位”计算收到的IP数据长度
		Point = strstr((const char *)AtRxBuf, (const char *)":")+1;
		memcpy(GetIpData, Point, len);
		GetIpData[len]='\0';
		printf("收到IP数据：%s\r\n",GetIpData);///////////////////////////////////////////////////////
		printf("收到IP数据长度=%d\r\n",len);///////////////////////////////////////////////////
	}
	#endif
	
	return (len);
}


/*******************************************************************
*函数：void ESP8266_DataAnalysisProcess(char *RxBuf)
*功能：解析服务器数据
*输入：char *RxBuf 服务器下发数据
*输出：
*特殊说明：用户可以在此基础上改造和扩展该函数，这里只是个简单的DEMO
*******************************************************************/
void ESP8266_DataAnalysisProcess(char *RxBuf)
{
	if(strstr((const char *)RxBuf, (const char *)PING_REQ) != NULL)//心跳请求？
	{
		ESP8266_IpSend((char *)PING_RSP, strlen((const char *)PING_RSP));//响应心跳
	}
	else if(strstr((const char *)RxBuf, (const char *)"\"t\":5") != NULL)//命令请求？
	{
		if(strstr((const char *)RxBuf, (const char *)"\"apitag\":\"ctrl\"") != NULL)//开锁请求
		{
			if((strstr((const char *)RxBuf, (const char *)"\"data\":1") != NULL))//开锁
			{
				ESP8266_IpSend((char *)PING_RSP, strlen((const char *)PING_RSP));//响应心跳
			}
		}
		else if(strstr((const char *)RxBuf, (const char *)"\"apitag\":\"defense\"") != NULL)//布防/撤防请求
		{
			if((strstr((const char *)RxBuf, (const char *)"\"data\":1") != NULL))//布防
			{
				printf("布防！\r\n");
				;//...
				;//...
				;//...
			}
			else if((strstr((const char *)RxBuf, (const char *)"\"data\":0") != NULL))//撤防
			{
				printf("撤防！\r\n");
				;//...
				;//...
				;//...
			}
		}
	}
}
