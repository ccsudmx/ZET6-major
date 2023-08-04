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
*函数：void ClrAtRxBuf(void)
*功能：清空AT串口接收缓存相关寄存器
*输入：无
*输出：无
*特殊说明：无
*******************************************************************/
void ClrAtRxBuf(void)
{
	memset(AT_RX_BUF, 0x00, MAX_AT_RX_LEN);//清空缓存
	AT_RX_COUNT = 0;
	F_AT_RX_FINISH = 0;
}
/*******************************************************************
*函数：void SendAtCmd(uint8_t *string)
*功能：发送AT指令字符串
*输入：
		uint8_t *string 待发送的AT指令
		uint8_t len 长度
*输出：无
*特殊说明：无
*******************************************************************/
void SendAtCmd(uint8_t *string, uint8_t len)
{
	UART4_SendStrlen(string, len);
	UART4_SendStrlen((uint8_t *)"\r\n", 2);
}

/*******************************************************************
*函数：void SendStrLen(uint8_t *string, uint8_t len)
*功能：发送字符串到AT指令串口
*输入：
		uint8_t *string 待发送的字符串
		uint8_t len 长度

*输出：无
*特殊说明：无
*******************************************************************/
void SendStrLen(uint8_t *string, uint8_t len)
{
	UART4_SendStrlen(string, len);
}

void ESP8266_init(void)
{
	uint8_t TryCount=0;
	int8_t temp=0;
	//连接服务器
	for(TryCount=0; TryCount<4; TryCount++)
	{
		temp=ConnectToServer((char *)MY_DEVICE_ID, (char *)MY_SECRET_KEY);
		
		if(temp != 0)
		{
			DBG_R_E("Connect To Server ERROR %d ,  ",temp);
			switch(temp){
				case -1: 
					printf("设置站点模式失败\r\n");break;
				case -2:
					printf("连接WIFI失败\r\n");break;
				case -3:
					printf("连接PC服务器失败\r\n");break;
				case -4:
					printf("订阅主题失败\r\n");break;
				default:
					printf("Connect未知错误\r\n");
			}
		}
		else
		{
		  //xfs_voice_start((unsigned char *)"连接云平台成功", XFS_FORMAT_GBK, 1);
			server = 1;
			DBG_R_E("Connect To Server OK");
			ESP8266_MQTT_Pub("{\"q\":1}", strlen("{\"q\":1}"),1);
			int getIndex = 0;//因为存储的时候和发送的时候顺序是相反的，所以要借助这个变量来记录当前发送到第几条数据了
			while(onlineCode == 0 && offlineMessageCounts > 0){//发送离线数据 i1  i2  h1  h2  t1  t2  s1  s2  infraredStatus1
				//先赋值
				i1 = offlineMessage[getIndex][0];
				i2 = offlineMessage[getIndex][1];
				h1 = offlineMessage[getIndex][2];
				h2 = offlineMessage[getIndex][3];
				t1 = offlineMessage[getIndex][4];
				t2 = offlineMessage[getIndex][5];
				s1 = offlineMessage[getIndex][6];
				s2 = offlineMessage[getIndex][7];
				infraredStatus1 = offlineMessage[offlineMessageCounts][8];
				//再发送
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

