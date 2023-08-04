/******************************************************************************
******************************************************************************
* @file    app/app_cloud.c 
* @author  zhao
* @version V1.0.0
* @date    2018.07.27
* @update  2018.07.27 
* @brief   云服务代码
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
*函数：int8_t ESP8266_SendSensor(char *ApiTag, 
									uint32_t sensor, char *TimeStr)
*功能：发送传感数据到服务器
*输入：
	char *ApiTag0 实体对象标签名称
	uint32_t sensor0, 传感数值
	char *ApiTag1 实体对象标签名称
	uint32_t sensor1, 传感数值
	char *ApiTag2 实体对象标签名称
	uint32_t sensor2, 传感数值
	char *TimeStr，采集数据时的时间，须是yyyy-mm-dd hh:mm:ss格式
*输出：
		return = 0 ,sucess
		return < 0 ,error
*特殊说明：
*******************************************************************/
int8_t SE_SendSensor(char *ApiTag0, uint32_t Value0, char *TimeStr)
{
	uint8_t TryGo = 0;
	int8_t error = 0;
	uint8_t TrySend = 0;
	uint8_t TxetBuf[MAX_AT_TX_LEN];
	memset(TxetBuf,0x00,MAX_AT_TX_LEN);//清空缓存
	sprintf((char *)TxetBuf,"{\"t\":3,\"datatype\":2,\"datas\":{\"%s\":{\"%s\":%d}},\"msgid\":001}",
													ApiTag0,TimeStr,Value0);
	se_send_sensor:		
	//printf("%s\r\n",TxetBuf);////////////////////////////////////////////////////////////
	if(ESP8266_IpSend((char *)TxetBuf, strlen((char *)TxetBuf)) < 0)
	{//发送失败
		error=-1;
	
	}
	else
	{//发送成功
		for(TryGo = 0; TryGo<50; TryGo++)//最多等待时间50*10=500ms
		{
			if(strstr((const char *)AT_RX_BUF, (const char *)"SEND OK") == NULL)//检查响应状态是否为"上报成功"
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
//	    DBG_B_INFO("重发传感器数据 %d ",TrySend+1);
	 	TrySend++;
		goto se_send_sensor;
	}
	return error;
}

/*******************************************************************
*函数：char *USER_GetJsonValue(char *cJson, char *Tag)
*功能：json为字符串序列，将json格式中的目标对象Tag对应的值字符串提取出来
*输入：
		char *cJson json字符串
		char *Tag 要操作的对象标签
*输出：返回数值的字符串形式的启始地址
*特殊说明：用户可以在此基础上改造和扩展该函数，这里只是个简单的DEMO
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
		//printf("空字符！\r\n");
		return NULL;
	}
	i=strlen((const char *)temp);
	target=target+i;
	memset(temp, 0x00, 128);
	
	if(*target == '"')// 如果是字符型
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
		for(i=0; i<10; i++, target++)//数值超过10个位为非法，由于2^32=4294967296
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
	//printf("数值=%s\r\n",temp);
	return (char *)temp;
}

/*******************************************************************
*函数：char *USER_GetSteeringEngineJsonValue(char *cJson, char *Tag)
*功能：json为字符串序列，将json格式中的目标对象Tag对应的值字符串转换为数值
*输入：
		char *cJson json字符串
		char *Tag 要操作的对象标签
*输出：返回数值
*特殊说明：用户可以在此基础上改造和扩展该函数，这里只是个简单的DEMO
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
		printf("空字符！\r\n");
		return NULL;
	}
	i=strlen((const char *)temp);
	target=target+i;
	memset(temp, 0x00, 128);
	for(i=0; i<10; i++, target++)//数值超过10个位为非法，由于2^32=4294967296
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
// printf("数值str=%s\r\n",temp);
//	printf("数值str=%s\r\n",target);
// len=strlen((const char *)target);
//	for(i=0; i<len;i++)
//    {
//	 printf("数值str[%d]=0x%x  \r\n",i,*(target+i));
//	}
	len=strlen((const char *)temp);
	switch(len)
	{
		case(1):value=temp[0]-'0';break;
		case(2):value=temp[1]-'0'+(temp[0]-'0')*10;break;
		case(3):value=temp[2]-'0'+(temp[0]-'0')*100+(temp[1]-'0')*10;break;
		default:break;
	}
	//printf("数值value=%d\r\n",value);
	return value;
}


/*******************************************************************
*函数：void USER_DataAnalysisProcess(char *RxBuf)
*功能：解析服务器数据
*输入：char *RxBuf 服务器下发数据
*输出：无
*特殊说明：用户可以在此基础上改造和扩展该函数，这里只是个简单的DEMO
*******************************************************************/
void USER_DataAnalysisProcess(char *RxBuf)
{
	char *cmdid = NULL;
	uint8_t TxetBuf[128];
	uint8_t number_down=0;
	#if 0
	//如果是数据库信息
	if(strstr((const char *)RxBuf, (const char *)"[") != NULL){
	
		
		return;
	}
	
	//lightAuto
	if(strstr((const char *)RxBuf, (const char *)"lightAuto") != NULL){
		int lightAuto = USER_GetSteeringEngineJsonValue((char *)RxBuf, (char *)"lightAuto");
	}
	
	//身份验证
	if(strstr((const char *)RxBuf, (const char *)"peopleCode") != NULL){
		if((int)USER_GetSteeringEngineJsonValue((char *)RxBuf, (char *)"peopleCode") == 0){
			identifyFlag = 0;
			printf("身份验证成功\r\n");
		}
		else{
			identifyFlag = -1;
			printf("身份验证失败\r\n");
		}
	}
	//闭馆开关
	if(strstr((const char *)RxBuf, (const char *)"alarm") != NULL){
		alarm = USER_GetSteeringEngineJsonValue((char *)RxBuf, (char *)"alarm");
		sprintf((char *)TxetBuf, "{\"alarm\":%d}", alarm);
 		ESP8266_MQTT_Pub((char *)TxetBuf, strlen((char *)TxetBuf),1);
	}
	
	//如果是硬件控制信息
	//模式
	if(strstr((const char *)RxBuf, (const char *)"auto") != NULL){
		int tempAuto = USER_GetSteeringEngineJsonValue((char *)RxBuf, (char *)"auto");
		if(autoMode != tempAuto){
			autoMode = tempAuto;
			allToJSON(1, (char*)TxetBuf,"auto");
			ESP8266_MQTT_Pub((char *)TxetBuf, strlen((char *)TxetBuf),1);
		}
	}
	//温度阈值
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
	//湿度阈值
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
	//光照阈值
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
	
	//烟雾阈值
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
	
	if(autoMode == 0) { //手动
		//可调灯
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
			if(LoRa_SendData_Affirm((char *)TxetBuf) == 0){//如果结点收到消息
				//MQTT数据回发
				allToJSON(2, (char*)TxetBuf,"lightStatus", "lightPWM");
				ESP8266_MQTT_Pub((char *)TxetBuf, strlen((char *)TxetBuf),1);
			}
			else {
				//还原数值
				lightStatus = tempLightStatus;
				lightPWM = tempLightPWM;
				printf("Light断开连接\r\n");
			}
		}
		
		//风扇
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
			if(LoRa_SendData_Affirm((char *)TxetBuf) == 0){//如果结点收到消息
				//MQTT数据回发
				allToJSON(2, (char*)TxetBuf,"fanStatus1", "fanSpeed1");
				ESP8266_MQTT_Pub((char *)TxetBuf, strlen((char *)TxetBuf),1);
			//	printf("设置成功!fanStatus1=%d,fanSpeed1=%d",fanStatus1,fanSpeed1);
			}
			else {
				//还原数值
				fanStatus1 = tempFanStatus1;
				fanSpeed1 = tempFanSpeed1;
				printf("FAN1断开连接\r\n");
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
			if(LoRa_SendData_Affirm((char *)TxetBuf) == 0){//如果结点收到消息
				//MQTT数据回发
				allToJSON(2, (char*)TxetBuf,"fanStatus2", "fanSpeed2");
				ESP8266_MQTT_Pub((char *)TxetBuf, strlen((char *)TxetBuf),1);
			}
			else {
				//还原数值
				fanStatus2 = tempFanStatus2;
				fanSpeed2 = tempFanSpeed2;
				printf("FAN2断开连接\r\n");
			}
		}
		
		//步进电机
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
			if(LoRa_SendData_Affirm((char *)TxetBuf) == 0){//如果结点收到消息
				//MQTT数据回发
				allToJSON(3, (char*)TxetBuf,"motorStatus1", "motorSpeed1", "motorDirection1");
				ESP8266_MQTT_Pub((char *)TxetBuf, strlen((char *)TxetBuf),1);
			}
			else {
				//还原数值
				motorStatus1 = tempMotorStatus1;
				motorSpeed1 = tempMotorSpeed1;
				motorDirection1 = tempMotorDirection1;
				printf("Motor1断开连接\r\n");
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
			if(LoRa_SendData_Affirm((char *)TxetBuf) == 0){//如果结点收到消息
				//MQTT数据回发
				allToJSON(3, (char*)TxetBuf,"motorStatus2", "motorSpeed2", "motorDirection2");
				ESP8266_MQTT_Pub((char *)TxetBuf, strlen((char *)TxetBuf),1);
			}
			else {
				//还原数值
				motorStatus2 = tempMotorStatus2;
				motorSpeed2 = tempMotorSpeed2;
				motorDirection2 = tempMotorDirection2;
				printf("Motor2断开连接\r\n");
			}
		}
		//蜂鸣器
		if(strstr((const char *)RxBuf, (const char *)"buzzerStatus") != NULL){
			buzzerStatus = USER_GetSteeringEngineJsonValue((char *)RxBuf, (char *)"buzzerStatus");
			//buzzerOpen();
		}
		//LED灯
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
*函数：void USER_Datatoplayer(char *RxBuf)
*功能：解析服务器数据 判断是否为语音播报执行器
*输入：char *RxBuf 服务器下发数据
*输出：无
*特殊说明：考生在该代码上补充。需获取需要语音播报额数据
           例如：char *RxBuf= {"apitag":"string_play","cmdid":73166,"data":"你好","t":5}       p_play="你好"      播报"你好"。
                 char *RxBuf= {"apitag":"string_play","cmdid":73166,"data":"谢谢","t":5}    p_play="谢谢"      播报"谢谢"。




/*
  * @brief  cloud_task 云服务进程
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

int sensorTimes = 0;//4s一次

int reconnectedTimes = 0;//掉线后每存10条数据后重连一次

int sTime = 0;
void cloud_task(void)
{
   static uint32_t lastTime;
   static int8_t erroCount=0;
   int8_t error=0;
   char str[50];
   if(F_AT_RX_FINISH)//串口4接收到数据
	 {	  // 接收到数据包
		// ESP8266_Get_MQTT_Data((uint8_t *)AT_RX_BUF, (char *)MQTTData);
		 //printf("AT_RX_BUF：%s", (char*)AT_RX_BUF);
		 if(strstr((const char *)AT_RX_BUF, (const char *)"+MQTTSUBRECV:0") != NULL){
		//	ESP8266_MQTT_Pub("{\"code\":0}", strlen("{\"code\":0}"),1);
			// printf("徐享%s",(char *)AT_RX_BUF);
			//USER_DataAnalysisProcess((char *)MQTTData);
			 USER_DataAnalysisProcess((char *)AT_RX_BUF);
		 }
		 
		 memset(MQTTData, 0x00, 256);
		 ClrAtRxBuf();
	 }
	 //10ms
	 if((uint32_t)(SYSTICK_get_time()-lastTime>=1000))//10S发送一次数据,备注：该时间不准确，大约10s
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
	  	 //RTC_Get();				 //更新时间   
	  	 //sprintf(str,"%d-%d-%d %d:%d:%d",calendar.w_year,calendar.w_month,calendar.w_date,calendar.hour,calendar.min,calendar.sec);//输出闹铃时间  
	   	
		 //error=SE_SendSensor((char *)"number_up", number_show, (char *)str);
		
	 	 ClrAtRxBuf();
	     // DBG_B_INFO("SE_SendSensor %d",error);
		 
		 
		 #if 0 //利用errorCount判断是否断开连接，时间为（判断一次时间==10）*（错误次数==5）= 50s
		 //判断是否和服务器失去连接
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
		if(erroCount > 6)//通讯失败次数超过4次 视为设备离线  //四次太敏感，先设置成六次
		{
			erroCount=0;
		//	buzzerOpen();
			server = 0;
			onlineCode = 0;
			DBG_B_INFO("断线重连");
			ESP8266_init();
		}
		#else //利用MQTTSendErrorTimes判断，时间为（判断一次时间==10）s
		//mqtt_pub 中发布失败 ErrorTimers ++
		if(MQTTSendErrorTimes > 1 || reconnectedTimes >= 10){//判断条件为（10s内发送失败错误次数大于1 或 存储离线数据次数>=10）注意，此处得设置的恰到好处，太大了就可能永远也无法触发重连，因为下面会重置为0
			int testTimes = 0;
			if(MQTTSendErrorTimes > 1){			
				for(testTimes = 0; testTimes < 3; testTimes++){
					 if(ESP8266_MQTT_Pub("{\"test\",0}", strlen("{\"test\",0}"),0) == 0){
						 break;
					 }
					 //新增延迟，给数据响应时间
					 delay_ms(10);
				}
			}
			if(testTimes >= 3 || reconnectedTimes >= 10){ // 如果本身就已经一次以上发送失败了，再连发3次还失败，那就认定为掉线 || 需要尝试回发离线数据了
			//	buzzerOpen();
				server = 0;
				onlineCode = 0;
				DBG_B_INFO("断线重连");
				//重连成功时会自动发送暂存的离线数据
				ESP8266_init();
			}
			reconnectedTimes = 0;
		}
		printf("MQTTSendErrorTimes = %d\r\n", MQTTSendErrorTimes);
		MQTTSendErrorTimes = 0;
		#endif
	}
	 if((uint32_t)(SYSTICK_get_time()-sTime>=100))//1s 记一次
	 {
	     sTime=SYSTICK_get_time();	 	 
		 //发送传感数据
		 if(sensorTimes >= 4){
			sensorTimes = 0;
			if(onlineCode == 0) {//记录离线数据
				recordOfflineMessage();
				reconnectedTimes++;
			}
			else if(onlineCode == 1){//发送在线数据
				char msg[128];
			
				//存储离线数据  i1  i2  h1  h2  t1  t2  s1  s2  infraredStatus1
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



