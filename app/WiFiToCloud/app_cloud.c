/******************************************************************************
******************************************************************************
* @file    app/app_cloud.c 
* @author  zhao
* @version V1.0.0
* @date    2018.07.27
* @update  2018.07.27 
* @brief   �Ʒ������
******************************************************************************
******************************************************************************/

#include "WiFiToCloud.h"
#include "debug_printf.h"
#include "app_cloud.h"
#include "hal_core.h"
#include "lora_app.h"

#include "hardwareVariables.h"

uint8_t IpData[128];
uint8_t MQTTData[256];
/*******************************************************************
*������int8_t ESP8266_SendSensor(char *ApiTag, 
									uint32_t sensor, char *TimeStr)
*���ܣ����ʹ������ݵ�������
*���룺
	char *ApiTag0 ʵ������ǩ����
	uint32_t sensor0, ������ֵ
	char *ApiTag1 ʵ������ǩ����
	uint32_t sensor1, ������ֵ
	char *ApiTag2 ʵ������ǩ����
	uint32_t sensor2, ������ֵ
	char *TimeStr���ɼ�����ʱ��ʱ�䣬����yyyy-mm-dd hh:mm:ss��ʽ
*�����
		return = 0 ,sucess
		return < 0 ,error
*����˵����
*******************************************************************/
int8_t SE_SendSensor(char *ApiTag0, uint32_t Value0, char *TimeStr)
{
	uint8_t TryGo = 0;
	int8_t error = 0;
	uint8_t TrySend = 0;
	uint8_t TxetBuf[MAX_AT_TX_LEN];
	memset(TxetBuf,0x00,MAX_AT_TX_LEN);//��ջ���
	sprintf((char *)TxetBuf,"{\"t\":3,\"datatype\":2,\"datas\":{\"%s\":{\"%s\":%d}},\"msgid\":001}",
													ApiTag0,TimeStr,Value0);
	se_send_sensor:		
	//printf("%s\r\n",TxetBuf);////////////////////////////////////////////////////////////
	if(ESP8266_IpSend((char *)TxetBuf, strlen((char *)TxetBuf)) < 0)
	{//����ʧ��
		error=-1;
	
	}
	else
	{//���ͳɹ�
		for(TryGo = 0; TryGo<50; TryGo++)//���ȴ�ʱ��50*10=500ms
		{
			if(strstr((const char *)AT_RX_BUF, (const char *)"SEND OK") == NULL)//�����Ӧ״̬�Ƿ�Ϊ"�ϱ��ɹ�"
			{
				error = -1;
			
			}
			else
			{
				error = 0;
				break;
			}
			delay_ms(1000);
		}
	}
	if((error!=0)&&(TrySend<2))
	{
//	    DBG_B_INFO("�ط����������� %d ",TrySend+1);
	 	TrySend++;
		goto se_send_sensor;
	}
	return error;
}

/*******************************************************************
*������char *USER_GetJsonValue(char *cJson, char *Tag)
*���ܣ�jsonΪ�ַ������У���json��ʽ�е�Ŀ�����Tag��Ӧ��ֵ�ַ�����ȡ����
*���룺
		char *cJson json�ַ���
		char *Tag Ҫ�����Ķ����ǩ
*�����������ֵ���ַ�����ʽ����ʼ��ַ
*����˵�����û������ڴ˻����ϸ������չ�ú���������ֻ�Ǹ��򵥵�DEMO
*******************************************************************/
char *USER_GetJsonValue(char *cJson, char *Tag)
{
	char *target = NULL;
	static char temp[100];
	int8_t i=0;
	//int flagX = 0;
	
	memset(temp, 0x00, 128);
	sprintf(temp,"\"%s\":",Tag);
	target=strstr((const char *)cJson, (const char *)temp);
	if(target == NULL)
	{
		//printf("���ַ���\r\n");
		return NULL;
	}
	i=strlen((const char *)temp);
	target=target+i;
	memset(temp, 0x00, 128);
	
	if(*target == '"')// ������ַ���
	{
		i = 0;
		target++;
		while(1){
			if(*target != '\\' && *(target + 1) == '"'){
				temp[i] = *target;
				break;
			}
			temp[i] = *target;
			target++;
			i++;
			if(i >= 99)
				break;
		}
	}
	else {
		for(i=0; i<10; i++, target++)//��ֵ����10��λΪ�Ƿ�������2^32=4294967296
		{
			if((*target<='9')&&(*target>='0'))
			{
				temp[i]=*target;
			}
			else
			{
				break;
			}
		}
	}
	temp[i+1] = '\0';
	//printf("��ֵ=%s\r\n",temp);
	return (char *)temp;
}

/*******************************************************************
*������char *USER_GetSteeringEngineJsonValue(char *cJson, char *Tag)
*���ܣ�jsonΪ�ַ������У���json��ʽ�е�Ŀ�����Tag��Ӧ��ֵ�ַ���ת��Ϊ��ֵ
*���룺
		char *cJson json�ַ���
		char *Tag Ҫ�����Ķ����ǩ
*�����������ֵ
*����˵�����û������ڴ˻����ϸ������չ�ú���������ֻ�Ǹ��򵥵�DEMO
*******************************************************************/
uint8_t USER_GetSteeringEngineJsonValue(char *cJson, char *Tag)
{
	char *target = NULL;
	//char *str = NULL;
	static char temp[10];
	uint8_t len=0;
	uint8_t value=0;
	int8_t i=0;
	
	memset(temp, 0x00, 128);
	sprintf(temp,"\"%s\":\"",Tag);
	target=strstr((const char *)cJson, (const char *)temp);
	if(target == NULL)
	{
		printf("���ַ���\r\n");
		return NULL;
	}
	i=strlen((const char *)temp);
	target=target+i;
	memset(temp, 0x00, 128);
	for(i=0; i<10; i++, target++)//��ֵ����10��λΪ�Ƿ�������2^32=4294967296
	{
		if((*target<='9')&&(*target>='0'))
		{
			temp[i]=*target;
		}
		else
		{
			break;
		}
	}
	//str=strstr((const char *)target, (const char *)"\",");
	
	temp[i+1] = '\0';
// printf("��ֵstr=%s\r\n",temp);
//	printf("��ֵstr=%s\r\n",target);
// len=strlen((const char *)target);
//	for(i=0; i<len;i++)
//    {
//	 printf("��ֵstr[%d]=0x%x  \r\n",i,*(target+i));
//	}
	len=strlen((const char *)temp);
	switch(len)
	{
		case(1):value=temp[0]-'0';break;
		case(2):value=temp[1]-'0'+(temp[0]-'0')*10;break;
		case(3):value=temp[2]-'0'+(temp[0]-'0')*100+(temp[1]-'0')*10;break;
		default:break;
	}
	//printf("��ֵvalue=%d\r\n",value);
	return value;
}


/*******************************************************************
*������void USER_DataAnalysisProcess(char *RxBuf)
*���ܣ���������������
*���룺char *RxBuf �������·�����
*�������
*����˵�����û������ڴ˻����ϸ������չ�ú���������ֻ�Ǹ��򵥵�DEMO
*******************************************************************/
void USER_DataAnalysisProcess(char *RxBuf)
{
	char *cmdid = NULL;
	uint8_t TxetBuf[128];
	uint8_t number_down=0;
	#if 0
	//��������ݿ���Ϣ
	if(strstr((const char *)RxBuf, (const char *)"[") != NULL){
	
		
		return;
	}
	
	//lightAuto
	if(strstr((const char *)RxBuf, (const char *)"lightAuto") != NULL){
		int lightAuto = USER_GetSteeringEngineJsonValue((char *)RxBuf, (char *)"lightAuto");
	}
	
	//�����֤
	if(strstr((const char *)RxBuf, (const char *)"peopleCode") != NULL){
		if((int)USER_GetSteeringEngineJsonValue((char *)RxBuf, (char *)"peopleCode") == 0){
			identifyFlag = 0;
			printf("�����֤�ɹ�\r\n");
		}
		else{
			identifyFlag = -1;
			printf("�����֤ʧ��\r\n");
		}
	}
	//�չݿ���
	if(strstr((const char *)RxBuf, (const char *)"alarm") != NULL){
		alarm = USER_GetSteeringEngineJsonValue((char *)RxBuf, (char *)"alarm");
		sprintf((char *)TxetBuf, "{\"alarm\":%d}", alarm);
 		ESP8266_MQTT_Pub((char *)TxetBuf, strlen((char *)TxetBuf),1);
	}
	
	//�����Ӳ��������Ϣ
	//ģʽ
	if(strstr((const char *)RxBuf, (const char *)"auto") != NULL){
		int tempAuto = USER_GetSteeringEngineJsonValue((char *)RxBuf, (char *)"auto");
		if(autoMode != tempAuto){
			autoMode = tempAuto;
			allToJSON(1, (char*)TxetBuf,"auto");
			ESP8266_MQTT_Pub((char *)TxetBuf, strlen((char *)TxetBuf),1);
		}
	}
	//�¶���ֵ
	if(strstr((const char *)RxBuf, (const char *)"minT1") != NULL){
		int temp = USER_GetSteeringEngineJsonValue((char *)RxBuf, (char *)"minT1");
		if(minT1 != temp){
			minT1 = temp;
			allToJSON(1, (char*)TxetBuf,"minT1");
			ESP8266_MQTT_Pub((char *)TxetBuf, strlen((char *)TxetBuf),1);
		}
	}
	if(strstr((const char *)RxBuf, (const char *)"maxT1") != NULL){
		int temp = USER_GetSteeringEngineJsonValue((char *)RxBuf, (char *)"maxT1");
		if(maxT1 != temp){
			maxT1 = temp;
			allToJSON(1, (char*)TxetBuf,"maxT1");
			ESP8266_MQTT_Pub((char *)TxetBuf, strlen((char *)TxetBuf),1);
		}
	}
	if(strstr((const char *)RxBuf, (const char *)"minT2") != NULL){
		int temp = USER_GetSteeringEngineJsonValue((char *)RxBuf, (char *)"minT2");
		if(minT2 != temp){
			minT2 = temp;
			allToJSON(1, (char*)TxetBuf,"minT2");
			ESP8266_MQTT_Pub((char *)TxetBuf, strlen((char *)TxetBuf),1);
		}
	}
	if(strstr((const char *)RxBuf, (const char *)"maxT2") != NULL){
		int temp = USER_GetSteeringEngineJsonValue((char *)RxBuf, (char *)"maxT2");
		if(maxT2 != temp){
			maxT2 = temp;
			allToJSON(1, (char*)TxetBuf,"maxT2");
			ESP8266_MQTT_Pub((char *)TxetBuf, strlen((char *)TxetBuf),1);
		}
	}
	//ʪ����ֵ
	if(strstr((const char *)RxBuf, (const char *)"minH1") != NULL){
		int temp = USER_GetSteeringEngineJsonValue((char *)RxBuf, (char *)"minH1");
		if(minH1 != temp){
			minH1 = temp;
			allToJSON(1, (char*)TxetBuf,"minH1");
			ESP8266_MQTT_Pub((char *)TxetBuf, strlen((char *)TxetBuf),1);
		}
	}
	if(strstr((const char *)RxBuf, (const char *)"maxH1") != NULL){
		int temp = USER_GetSteeringEngineJsonValue((char *)RxBuf, (char *)"maxH1");
		if(maxH1 != temp){
			maxH1 = temp;
			allToJSON(1, (char*)TxetBuf,"maxH1");
			ESP8266_MQTT_Pub((char *)TxetBuf, strlen((char *)TxetBuf),1);
		}
	}
	if(strstr((const char *)RxBuf, (const char *)"minH2") != NULL){
		int temp = USER_GetSteeringEngineJsonValue((char *)RxBuf, (char *)"minH2");
		if(minH2 != temp){
			minH2 = temp;
			allToJSON(1, (char*)TxetBuf,"minH2");
			ESP8266_MQTT_Pub((char *)TxetBuf, strlen((char *)TxetBuf),1);
		}
	}
	if(strstr((const char *)RxBuf, (const char *)"maxH2") != NULL){
		int temp = USER_GetSteeringEngineJsonValue((char *)RxBuf, (char *)"maxH2");
		if(maxH2 != temp){
			maxH2 = temp;
			allToJSON(1, (char*)TxetBuf,"maxH2");
			ESP8266_MQTT_Pub((char *)TxetBuf, strlen((char *)TxetBuf),1);
		}
	}
	//������ֵ
	if(strstr((const char *)RxBuf, (const char *)"minI1") != NULL){
		int temp = USER_GetSteeringEngineJsonValue((char *)RxBuf, (char *)"minI1");
		if(minI1 != temp){
			minI1 = temp;
			allToJSON(1, (char*)TxetBuf,"minI1");
			ESP8266_MQTT_Pub((char *)TxetBuf, strlen((char *)TxetBuf),1);
		}
	}
	if(strstr((const char *)RxBuf, (const char *)"maxI1") != NULL){
		int temp = USER_GetSteeringEngineJsonValue((char *)RxBuf, (char *)"maxI1");
		if(maxI1 != temp){
			maxI1 = temp;
			allToJSON(1, (char*)TxetBuf,"maxI1");
			ESP8266_MQTT_Pub((char *)TxetBuf, strlen((char *)TxetBuf),1);
		}
	}
	if(strstr((const char *)RxBuf, (const char *)"minI2") != NULL){
		int temp = USER_GetSteeringEngineJsonValue((char *)RxBuf, (char *)"minI2");
		if(minI2 != temp){
			minI2 = temp;
			allToJSON(1, (char*)TxetBuf,"minI2");
			ESP8266_MQTT_Pub((char *)TxetBuf, strlen((char *)TxetBuf),1);
		}
	}
	if(strstr((const char *)RxBuf, (const char *)"maxI2") != NULL){
		int temp = USER_GetSteeringEngineJsonValue((char *)RxBuf, (char *)"maxI2");
		if(maxI2 != temp){
			maxI2 = temp;
			allToJSON(1, (char*)TxetBuf,"maxI2");
			ESP8266_MQTT_Pub((char *)TxetBuf, strlen((char *)TxetBuf),1);
		}
	}
	
	//������ֵ
	if(strstr((const char *)RxBuf, (const char *)"minS1") != NULL){
		int temp = USER_GetSteeringEngineJsonValue((char *)RxBuf, (char *)"minS1");
		if(minS1 != temp){
			minS1 = temp;
			allToJSON(1, (char*)TxetBuf,"minS1");
			ESP8266_MQTT_Pub((char *)TxetBuf, strlen((char *)TxetBuf),1);
		}
	}
	if(strstr((const char *)RxBuf, (const char *)"maxS1") != NULL){
		int temp = USER_GetSteeringEngineJsonValue((char *)RxBuf, (char *)"maxS1");
		if(maxS1 != temp){
			maxS1 = temp;
			allToJSON(1, (char*)TxetBuf,"maxS1");
			ESP8266_MQTT_Pub((char *)TxetBuf, strlen((char *)TxetBuf),1);
		}
	}
	if(strstr((const char *)RxBuf, (const char *)"minS2") != NULL){
		int temp = USER_GetSteeringEngineJsonValue((char *)RxBuf, (char *)"minS2");
		if(minS2 != temp){
			minS2 = temp;
			allToJSON(1, (char*)TxetBuf,"minS2");
			ESP8266_MQTT_Pub((char *)TxetBuf, strlen((char *)TxetBuf),1);
		}
	}
	if(strstr((const char *)RxBuf, (const char *)"maxS2") != NULL){
		int temp = USER_GetSteeringEngineJsonValue((char *)RxBuf, (char *)"maxS2");
		if(maxS2 != temp){
			maxS2 = temp;
			allToJSON(1, (char*)TxetBuf,"maxS2");
			ESP8266_MQTT_Pub((char *)TxetBuf, strlen((char *)TxetBuf),1);
		}
	}
	
	#endif
	
	if(autoMode == 0) { //�ֶ�
		//�ɵ���
		int sendFlag = 0;
		int lightFlag1[2] = {0,0};
		int tempLightStatus = lightStatus;
		int tempLightPWM = lightPWM;
		if(strstr((const char *)RxBuf, (const char *)"lightStatus") != NULL){
			lightStatus = USER_GetSteeringEngineJsonValue((char *)RxBuf, (char *)"lightStatus");
			
			lightFlag1[0] = 1;
			sendFlag++;
		}
		if(strstr((const char *)RxBuf, (const char *)"lightPWM") != NULL){
			lightPWM = USER_GetSteeringEngineJsonValue((char *)RxBuf, (char *)"lightPWM");
			lightFlag1[1] = 1;
			sendFlag++;
		}
		if(sendFlag > 0){
			sendFlag = 0;
			allToJSON2(2, (char*)TxetBuf,"lightStatus", "lightPWM");
			if(LoRa_SendData_Affirm((char *)TxetBuf) == 0){//�������յ���Ϣ
				//MQTT���ݻط�
				allToJSON(2, (char*)TxetBuf,"lightStatus", "lightPWM");
				ESP8266_MQTT_Pub((char *)TxetBuf, strlen((char *)TxetBuf),1);
			}
			else {
				//��ԭ��ֵ
				lightStatus = tempLightStatus;
				lightPWM = tempLightPWM;
				printf("Light�Ͽ�����\r\n");
			}
		}
		
		//����
		int tempFanStatus1 = fanStatus1;
		int tempFanSpeed1 = fanSpeed1;
		/******************Fan1******************/
		if(strstr((const char *)RxBuf, (const char *)"fanStatus1") != NULL){
			fanStatus1 = USER_GetSteeringEngineJsonValue((char *)RxBuf, (char *)"fanStatus1");
			sendFlag++;
		}
		if(strstr((const char *)RxBuf, (const char *)"fanSpeed1") != NULL){
			fanSpeed1 = USER_GetSteeringEngineJsonValue((char *)RxBuf, (char *)"fanSpeed1");
			sendFlag++;
			
		}
		if(sendFlag > 0){
			sendFlag = 0;
			allToJSON2(2, (char*)TxetBuf,"fanStatus1", "fanSpeed1");
			//LoRa_SendData((char *)TxetBuf);
			//LoRa_SendData((char *)RxBuf);
			if(LoRa_SendData_Affirm((char *)TxetBuf) == 0){//�������յ���Ϣ
				//MQTT���ݻط�
				allToJSON(2, (char*)TxetBuf,"fanStatus1", "fanSpeed1");
				ESP8266_MQTT_Pub((char *)TxetBuf, strlen((char *)TxetBuf),1);
			//	printf("���óɹ�!fanStatus1=%d,fanSpeed1=%d",fanStatus1,fanSpeed1);
			}
			else {
				//��ԭ��ֵ
				fanStatus1 = tempFanStatus1;
				fanSpeed1 = tempFanSpeed1;
				printf("FAN1�Ͽ�����\r\n");
			}
		}
		int tempFanStatus2 = fanStatus2;
		int tempFanSpeed2 = fanSpeed2;
		/******************Fan2******************/
		if(strstr((const char *)RxBuf, (const char *)"fanStatus2") != NULL){
			fanStatus2 = USER_GetSteeringEngineJsonValue((char *)RxBuf, (char *)"fanStatus2");
			sendFlag++;
		}
		if(strstr((const char *)RxBuf, (const char *)"fanSpeed2") != NULL){
			fanSpeed2 = USER_GetSteeringEngineJsonValue((char *)RxBuf, (char *)"fanSpeed2");
			sendFlag++;
		}
		if(sendFlag > 0){
			sendFlag = 0;
			allToJSON2(2, (char*)TxetBuf,"fanStatus2", "fanSpeed2");
			if(LoRa_SendData_Affirm((char *)TxetBuf) == 0){//�������յ���Ϣ
				//MQTT���ݻط�
				allToJSON(2, (char*)TxetBuf,"fanStatus2", "fanSpeed2");
				ESP8266_MQTT_Pub((char *)TxetBuf, strlen((char *)TxetBuf),1);
			}
			else {
				//��ԭ��ֵ
				fanStatus2 = tempFanStatus2;
				fanSpeed2 = tempFanSpeed2;
				printf("FAN2�Ͽ�����\r\n");
			}
		}
		
		//�������
		/******************Motor1******************/
		int tempMotorStatus1 = motorStatus1;
		int tempMotorSpeed1 = motorSpeed1;
		int tempMotorDirection1 = motorDirection1;
		int motorFlag1[3] = {0,0,0};
		if(strstr((const char *)RxBuf, (const char *)"motorStatus1") != NULL){
			motorStatus1 = USER_GetSteeringEngineJsonValue((char *)RxBuf, (char *)"motorStatus1");
			motorFlag1[0] = 1;
			sendFlag++;
		}
		if(strstr((const char *)RxBuf, (const char *)"motorSpeed1") != NULL){
			motorSpeed1 = USER_GetSteeringEngineJsonValue((char *)RxBuf, (char *)"motorSpeed1");
			motorFlag1[1] = 1;
			sendFlag++;
		}
		if(strstr((const char *)RxBuf, (const char *)"motorDirection1") != NULL){
			motorDirection1 = USER_GetSteeringEngineJsonValue((char *)RxBuf, (char *)"motorDirection1");
			motorFlag1[2] = 1;
			sendFlag++;
		}
		if(sendFlag > 0){
			sendFlag = 0;
			allToJSON2(3, (char*)TxetBuf,"motorStatus1", "motorSpeed1", "motorDirection1");
			if(LoRa_SendData_Affirm((char *)TxetBuf) == 0){//�������յ���Ϣ
				//MQTT���ݻط�
				allToJSON(3, (char*)TxetBuf,"motorStatus1", "motorSpeed1", "motorDirection1");
				ESP8266_MQTT_Pub((char *)TxetBuf, strlen((char *)TxetBuf),1);
			}
			else {
				//��ԭ��ֵ
				motorStatus1 = tempMotorStatus1;
				motorSpeed1 = tempMotorSpeed1;
				motorDirection1 = tempMotorDirection1;
				printf("Motor1�Ͽ�����\r\n");
			}
		}
		
		/******************Motor2******************/
		int tempMotorStatus2 = motorStatus2;
		int tempMotorSpeed2 = motorSpeed2;
		int tempMotorDirection2 = motorDirection2;
		int motorFlag2[3] = {0,0,0};
		if(strstr((const char *)RxBuf, (const char *)"motorStatus2") != NULL){
			motorStatus2 = USER_GetSteeringEngineJsonValue((char *)RxBuf, (char *)"motorStatus2");
			motorFlag2[0] = 1;
			sendFlag++;
		}
		if(strstr((const char *)RxBuf, (const char *)"motorSpeed2") != NULL){
			motorSpeed2 = USER_GetSteeringEngineJsonValue((char *)RxBuf, (char *)"motorSpeed2");
			motorFlag2[1] = 1;
			sendFlag++;
		}
		if(strstr((const char *)RxBuf, (const char *)"motorDirection2") != NULL){
			motorDirection2 = USER_GetSteeringEngineJsonValue((char *)RxBuf, (char *)"motorDirection2");
			motorFlag2[2] = 1;
			sendFlag++;
		}
		if(sendFlag > 0){
			sendFlag = 0;
			allToJSON2(3, (char*)TxetBuf,"motorStatus2", "motorSpeed2", "motorDirection2");
			if(LoRa_SendData_Affirm((char *)TxetBuf) == 0){//�������յ���Ϣ
				//MQTT���ݻط�
				allToJSON(3, (char*)TxetBuf,"motorStatus2", "motorSpeed2", "motorDirection2");
				ESP8266_MQTT_Pub((char *)TxetBuf, strlen((char *)TxetBuf),1);
			}
			else {
				//��ԭ��ֵ
				motorStatus2 = tempMotorStatus2;
				motorSpeed2 = tempMotorSpeed2;
				motorDirection2 = tempMotorDirection2;
				printf("Motor2�Ͽ�����\r\n");
			}
		}
		//������
		if(strstr((const char *)RxBuf, (const char *)"buzzerStatus") != NULL){
			buzzerStatus = USER_GetSteeringEngineJsonValue((char *)RxBuf, (char *)"buzzerStatus");
			//buzzerOpen();
		}
		//LED��
		//int tempLEDStatus1,tempLEDStatus2,tempLEDStatus3,tempLEDStatus4;
		if(strstr((const char *)RxBuf, (const char *)"LEDStatus1") != NULL){
			sendFlag++;
			LEDStatus1 = USER_GetSteeringEngineJsonValue((char *)RxBuf, (char *)"LEDStatus1");
		}
		if(strstr((const char *)RxBuf, (const char *)"LEDStatus2") != NULL){
			sendFlag++;
			LEDStatus2 = USER_GetSteeringEngineJsonValue((char *)RxBuf, (char *)"LEDStatus2");
		}
		if(strstr((const char *)RxBuf, (const char *)"LEDStatus3") != NULL){
			sendFlag++;
			LEDStatus3 = USER_GetSteeringEngineJsonValue((char *)RxBuf, (char *)"LEDStatus3");
		}
		if(strstr((const char *)RxBuf, (const char *)"LEDStatus4") != NULL){
			sendFlag++;
			LEDStatus4 = USER_GetSteeringEngineJsonValue((char *)RxBuf, (char *)"LEDStatus4");
		}
		if(sendFlag > 0){
			sendFlag = 0;
			allToJSON(4, (char*)TxetBuf,"LEDStatus1", "LEDStatus2", "LEDStatus3", "LEDStatus4");
			ESP8266_MQTT_Pub((char *)TxetBuf, strlen((char *)TxetBuf),1);
		}
	}
	
	
	
}


/*******************************************************************
*������void USER_Datatoplayer(char *RxBuf)
*���ܣ��������������� �ж��Ƿ�Ϊ��������ִ����
*���룺char *RxBuf �������·�����
*�������
*����˵���������ڸô����ϲ��䡣���ȡ��Ҫ��������������
           ���磺char *RxBuf= {"apitag":"string_play","cmdid":73166,"data":"���","t":5}       p_play="���"      ����"���"��
                 char *RxBuf= {"apitag":"string_play","cmdid":73166,"data":"лл","t":5}    p_play="лл"      ����"лл"��




/*
  * @brief  cloud_task �Ʒ������
  * @param  none
  * @note           
  * @Date:2018.7.27
  * @author:zhao
  * @return:none
*/
extern volatile char number_write;
extern volatile char number_show;
int sendTypeFlag = 0;
int openTimes = 0;//10s

int alarmTimes = 0;//8s

int sensorTimes = 0;//4sһ��

int reconnectedTimes = 0;//���ߺ�ÿ��10�����ݺ�����һ��

int sTime = 0;
void cloud_task(void)
{
   static uint32_t lastTime;
   static int8_t erroCount=0;
   int8_t error=0;
   char str[50];
   if(F_AT_RX_FINISH)//����4���յ�����
	 {	  // ���յ����ݰ�
		// ESP8266_Get_MQTT_Data((uint8_t *)AT_RX_BUF, (char *)MQTTData);
		 //printf("AT_RX_BUF��%s", (char*)AT_RX_BUF);
		 if(strstr((const char *)AT_RX_BUF, (const char *)"+MQTTSUBRECV:0") != NULL){
		//	ESP8266_MQTT_Pub("{\"code\":0}", strlen("{\"code\":0}"),1);
			// printf("����%s",(char *)AT_RX_BUF);
			//USER_DataAnalysisProcess((char *)MQTTData);
			 USER_DataAnalysisProcess((char *)AT_RX_BUF);
		 }
		 
		 memset(MQTTData, 0x00, 256);
		 ClrAtRxBuf();
	 }
	 //10ms
	 if((uint32_t)(SYSTICK_get_time()-lastTime>=1000))//10S����һ������,��ע����ʱ�䲻׼ȷ����Լ10s
	 {
		 char msg[200];
		
		 //if(sendTypeFlag == 0){
			// sendTypeFlag = 1;
			 allToJSON(4, msg, 
			 "lightStatus", "lightPWM",
			 "fanStatus1", "fanSpeed1"
			);
		 //}
//		 else if(sendTypeFlag == 1){
//			 sendTypeFlag = 0;
//			 allToJSON(17, msg, 
//			 "minT1", "maxT1",
//			 "minT2", "maxT2", 
//			 "minH1","maxH1", 
//			 "minH2", "maxH2",
//			 "minI1","maxI1", 
//			 "minI2","maxI2",
//			 "minS1","maxS1",
//			 "minS2","maxS2",
//			 "auto");
//		 }
		 error = ESP8266_MQTT_Pub(msg, strlen(msg),1);
	     lastTime=SYSTICK_get_time();
	  	 //RTC_Get();				 //����ʱ��   
	  	 //sprintf(str,"%d-%d-%d %d:%d:%d",calendar.w_year,calendar.w_month,calendar.w_date,calendar.hour,calendar.min,calendar.sec);//�������ʱ��  
	   	
		 //error=SE_SendSensor((char *)"number_up", number_show, (char *)str);
		
	 	 ClrAtRxBuf();
	     // DBG_B_INFO("SE_SendSensor %d",error);
		 
		 
		 #if 0 //����errorCount�ж��Ƿ�Ͽ����ӣ�ʱ��Ϊ���ж�һ��ʱ��==10��*���������==5��= 50s
		 //�ж��Ƿ�ͷ�����ʧȥ����
	    if(error != 0)
	    {
			erroCount++;
	//	 DBG_B_INFO("erroCount++  %d",erroCount);
		}
		else
		{
			erroCount=0;
	//		DBG_B_INFO("erroCount=0 %d",erroCount);
		}
		if(erroCount > 6)//ͨѶʧ�ܴ�������4�� ��Ϊ�豸����  //�Ĵ�̫���У������ó�����
		{
			erroCount=0;
		//	buzzerOpen();
			server = 0;
			onlineCode = 0;
			DBG_B_INFO("��������");
			ESP8266_init();
		}
		#else //����MQTTSendErrorTimes�жϣ�ʱ��Ϊ���ж�һ��ʱ��==10��s
		//mqtt_pub �з���ʧ�� ErrorTimers ++
		if(MQTTSendErrorTimes > 1 || reconnectedTimes >= 10){//�ж�����Ϊ��10s�ڷ���ʧ�ܴ����������1 �� �洢�������ݴ���>=10��ע�⣬�˴������õ�ǡ���ô���̫���˾Ϳ�����ԶҲ�޷�������������Ϊ���������Ϊ0
			int testTimes = 0;
			if(MQTTSendErrorTimes > 1){			
				for(testTimes = 0; testTimes < 3; testTimes++){
					 if(ESP8266_MQTT_Pub("{\"test\",0}", strlen("{\"test\",0}"),0) == 0){
						 break;
					 }
					 //�����ӳ٣���������Ӧʱ��
					 delay_ms(10);
				}
			}
			if(testTimes >= 3 || reconnectedTimes >= 10){ // ���������Ѿ�һ�����Ϸ���ʧ���ˣ�������3�λ�ʧ�ܣ��Ǿ��϶�Ϊ���� || ��Ҫ���Իط�����������
			//	buzzerOpen();
				server = 0;
				onlineCode = 0;
				DBG_B_INFO("��������");
				//�����ɹ�ʱ���Զ������ݴ����������
				ESP8266_init();
			}
			reconnectedTimes = 0;
		}
		printf("MQTTSendErrorTimes = %d\r\n", MQTTSendErrorTimes);
		MQTTSendErrorTimes = 0;
		#endif
	}
	 if((uint32_t)(SYSTICK_get_time()-sTime>=100))//1s ��һ��
	 {
	     sTime=SYSTICK_get_time();	 	 
		 //���ʹ�������
		 if(sensorTimes >= 4){
			sensorTimes = 0;
			if(onlineCode == 0) {//��¼��������
				recordOfflineMessage();
				reconnectedTimes++;
			}
			else if(onlineCode == 1){//������������
				char msg[128];
			
				//�洢��������  i1  i2  h1  h2  t1  t2  s1  s2  infraredStatus1
				allToJSON(10, msg, 
					"i1", "i2",
					"h1", "h2", 
					"t1", "t2", 
					"s1", "s2",
					"infraredStatus1", "onlineCode");
				ESP8266_MQTT_Pub(msg, strlen(msg),1);
			}
		 }
		 else{
			 sensorTimes++;
		 }
	 }
}



