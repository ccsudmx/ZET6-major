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
/**********************************************************
	Modify History: qiuzhb make, 2016-8-21
**********************************************************/
#include <string.h>
#include <stdint.h>
#include "debug_printf.h"
#include "delay.h"
#include "bsp_uart4.h"
#include "WiFiToCloud.h"
#include "HAL_WiFiToCloud.h"
#include "lora_app.h"

/*******************************************************************
*������void ClrAtRxBuf(void)
*���ܣ����AT���ڽ��ջ�����ؼĴ���
*���룺��
*�������
*����˵������
*******************************************************************/
void ClrAtRxBuf(void)
{
	memset(AT_RX_BUF, 0x00, MAX_AT_RX_LEN);//��ջ���
	AT_RX_COUNT = 0;
	F_AT_RX_FINISH = 0;
}
/*******************************************************************
*������void SendAtCmd(uint8_t *string)
*���ܣ�����ATָ���ַ���
*���룺
		uint8_t *string �����͵�ATָ��
		uint8_t len ����
*�������
*����˵������
*******************************************************************/
void SendAtCmd(uint8_t *string, uint8_t len)
{
	UART4_SendStrlen(string, len);
	UART4_SendStrlen((uint8_t *)"\r\n", 2);
}

/*******************************************************************
*������void SendStrLen(uint8_t *string, uint8_t len)
*���ܣ������ַ�����ATָ���
*���룺
		uint8_t *string �����͵��ַ���
		uint8_t len ����

*�������
*����˵������
*******************************************************************/
void SendStrLen(uint8_t *string, uint8_t len)
{
	UART4_SendStrlen(string, len);
}

void ESP8266_init(void)
{
	uint8_t TryCount=0;
	int8_t temp=0;
	//���ӷ�����
	for(TryCount=0; TryCount<4; TryCount++)
	{
		temp=ConnectToServer((char *)MY_DEVICE_ID, (char *)MY_SECRET_KEY);
		
		if(temp != 0)
		{
			DBG_R_E("Connect To Server ERROR %d ,  ",temp);
			switch(temp){
				case -1: 
					printf("����վ��ģʽʧ��\r\n");break;
				case -2:
					printf("����WIFIʧ��\r\n");break;
				case -3:
					printf("����PC������ʧ��\r\n");break;
				case -4:
					printf("��������ʧ��\r\n");break;
				default:
					printf("Connectδ֪����\r\n");
			}
		}
		else
		{
		  //xfs_voice_start((unsigned char *)"������ƽ̨�ɹ�", XFS_FORMAT_GBK, 1);
			server = 1;
			DBG_R_E("Connect To Server OK");
			ESP8266_MQTT_Pub("{\"q\":1}", strlen("{\"q\":1}"),1);
			int getIndex = 0;//��Ϊ�洢��ʱ��ͷ��͵�ʱ��˳�����෴�ģ�����Ҫ���������������¼��ǰ���͵��ڼ���������
			while(onlineCode == 0 && offlineMessageCounts > 0){//������������ i1  i2  h1  h2  t1  t2  s1  s2  infraredStatus1
				//�ȸ�ֵ
				i1 = offlineMessage[getIndex][0];
				i2 = offlineMessage[getIndex][1];
				h1 = offlineMessage[getIndex][2];
				h2 = offlineMessage[getIndex][3];
				t1 = offlineMessage[getIndex][4];
				t2 = offlineMessage[getIndex][5];
				s1 = offlineMessage[getIndex][6];
				s2 = offlineMessage[getIndex][7];
				infraredStatus1 = offlineMessage[offlineMessageCounts][8];
				//�ٷ���
				char msg[128];
				allToJSON(10, msg, 
					"i1", "i2",
					"h1", "h2", 
					"t1", "t2", 
					"s1", "s2",
					"infraredStatus1", "onlineCode");
				ESP8266_MQTT_Pub(msg, strlen(msg),1);
				
				offlineMessageCounts--;
				getIndex++;
			}
			onlineCode = 1;
			//buzzerClose();
		  break;
		}
	}
}

