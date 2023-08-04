#include "lora_app.h"
#include "lora_ui.h"
#include "usart2.h"
#include "string.h"
#include "delay.h"
#include "stdio.h"

#include "hal_core.h"
#include "app_cloud.h"
#include "WiFiToCloud.h"
#include "hardWareVariables.h"

//�豸������ʼ��(�����豸������lora_cfg.h����)
_LoRa_CFG LoRa_CFG=
{
	.addr = LORA_ADDR,       //�豸��ַ
	.power = LORA_POWER,     //���书��
	.chn = LORA_CHN,         //�ŵ�
	.wlrate = LORA_RATE,     //��������
	.wltime = LORA_WLTIME,   //˯��ʱ��
	.mode = LORA_MODE,       //����ģʽ
	.mode_sta = LORA_STA,    //����״̬
	.bps = LORA_TTLBPS ,     //����������
	.parity = LORA_TTLPAR    //У��λ����
};

//ȫ�ֲ���
EXTI_InitTypeDef EXTI_InitStructure;
NVIC_InitTypeDef Lora_NVIC_InitStructure;

//�豸����ģʽ(���ڼ�¼�豸״̬)
u8 Lora_mode=0;//0:����ģʽ 1:����ģʽ 2:����ģʽ
//��¼�ж�״̬
static u8 Int_mode=0;//0:�ر� 1:������ 2:�½���

//AUX�ж�����
//mode:���õ�ģʽ 0:�ر� 1:������ 2:�½���
void Aux_Int(u8 mode)
{
    if(!mode)
	{
		EXTI_InitStructure.EXTI_LineCmd = DISABLE;//�ر��ж�
		Lora_NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;	
	}else
	{
		if(mode==1)
			 EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; //������
	    else if(mode==2)
			 EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;//�½���
		
		EXTI_InitStructure.EXTI_LineCmd = ENABLE;
		Lora_NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		
	}
	Int_mode = mode;//��¼�ж�ģʽ
	EXTI_Init(&EXTI_InitStructure);
	NVIC_Init(&Lora_NVIC_InitStructure);  
    
}

//LORA_AUX�жϷ�����
void EXTI4_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line4))
	{  
	   if(Int_mode==1)//������(����:��ʼ�������� ����:���ݿ�ʼ���)     
	   {
		  if(Lora_mode==1)//����ģʽ
		  {
			 USART2_RX_STA=0;//���ݼ�����0
		  }
		  Int_mode=2;//�����½��ش���
	   }
       else if(Int_mode==2)//�½���(����:�����ѷ����� ����:�����������)	
	   {
		  if(Lora_mode==1)//����ģʽ
		  {
			 USART2_RX_STA|=1<<15;//���ݼ���������
		  }else if(Lora_mode==2)//����ģʽ(�������ݷ������)
		  {
			 Lora_mode=1;//�������ģʽ
		  }
		  Int_mode=1;//���������ش���
	   }
       Aux_Int(Int_mode);//���������жϱ���
	   EXTI_ClearITPendingBit(EXTI_Line4); //���LINE4�ϵ��жϱ�־λ  
	}	
}

//LoRaģ���ʼ��
//����ֵ: 0,���ɹ�
//        1,���ʧ��
u8 LoRa_Init(void)
{
	 u8 retry=0;
	 u8 temp=1;
	
	 GPIO_InitTypeDef  GPIO_InitStructure;
		
	 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	//ʹ��PA�˿�ʱ��
	 RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);	//ʹ�ܸ��ù���ʱ��

     GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);//��ֹJTAG,�Ӷ�PA15��������ͨIOʹ��,����PA15��������ͨIO!!!	
	
	 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;	    		 //LORA_MD0
	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
	 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
	 GPIO_Init(GPIOA, &GPIO_InitStructure);	  				 //������� ��IO���ٶ�Ϊ50MHz
	
	 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;				 //LORA_AUX
	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; 		     //��������
	 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
	 GPIO_Init(GPIOA, &GPIO_InitStructure);					 //�����趨������ʼ��GPIOA.4
	
	 GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource4);
	
	   EXTI_InitStructure.EXTI_Line=EXTI_Line4;
  	 EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
  	 EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  //�����ش���
  	 EXTI_InitStructure.EXTI_LineCmd = DISABLE;              //�ж��߹ر�(�ȹرպ����ٴ�)
  	 EXTI_Init(&EXTI_InitStructure);//����EXTI_InitStruct��ָ���Ĳ�����ʼ������EXTI�Ĵ���
	
	  Lora_NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;			//LORA_AUX
  	 Lora_NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;//��ռ���ȼ�2�� 
  	 Lora_NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;		//�����ȼ�3
  	 Lora_NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE; //�ر��ⲿ�ж�ͨ���������ٴ򿪣�
   	 NVIC_Init(&Lora_NVIC_InitStructure); 
	 
	 LORA_MD0=0;
	 LORA_AUX=0;
	
	 while(LORA_AUX)//ȷ��LORAģ���ڿ���״̬��(LORA_AUX=0)
	 {
		 printf("ģ����æ,���Ե�!!\n"); 	
		 delay_ms(500);
	 }
	 usart2_init(115200);//��ʼ������2
	 
	 LORA_MD0=1;//����ATģʽ
	 delay_ms(40);
	 retry=3;
	 
	 while(retry--)
	 {
		 if(!lora_send_cmd("ATE0","OK",70)) //OK
		 {
			 temp=0;//���ɹ�
			 printf("���ɹ�");
			 break;
		 }	
	 }
	 if(retry==0) temp=1;//���ʧ��
	 return temp;
}

//Loraģ���������
void LoRa_Set(void)
{
	u8 sendbuf[20];
	u8 lora_addrh,lora_addrl=0;
	
	usart2_set(LORA_TTLBPS_115200,LORA_TTLPAR_8N1);//��������ģʽǰ����ͨ�Ų����ʺ�У��λ(115200 8λ���� 1λֹͣ ������У�飩
	usart2_rx(1);//��������2����
	
	while(LORA_AUX);//�ȴ�ģ�����
	LORA_MD0=1; //��������ģʽ
	delay_ms(40);
	Lora_mode=0;//���"����ģʽ"
	
	lora_addrh =  (LoRa_CFG.addr>>8)&0xff;
	lora_addrl = LoRa_CFG.addr&0xff;
	sprintf((char*)sendbuf,"AT+ADDR=%02x,%02x",lora_addrh,lora_addrl);//�����豸��ַ
	lora_send_cmd(sendbuf,"OK",50);
	sprintf((char*)sendbuf,"AT+WLRATE=%d,%d",LoRa_CFG.chn,LoRa_CFG.wlrate);//�����ŵ��Ϳ�������
	lora_send_cmd(sendbuf,"OK",50);
	sprintf((char*)sendbuf,"AT+TPOWER=%d",LoRa_CFG.power);//���÷��书��
	lora_send_cmd(sendbuf,"OK",50);
	sprintf((char*)sendbuf,"AT+CWMODE=%d",LoRa_CFG.mode);//���ù���ģʽ
	lora_send_cmd(sendbuf,"OK",50);
	sprintf((char*)sendbuf,"AT+TMODE=%d",LoRa_CFG.mode_sta);//���÷���״̬
	lora_send_cmd(sendbuf,"OK",50);
	sprintf((char*)sendbuf,"AT+WLTIME=%d",LoRa_CFG.wltime);//����˯��ʱ��
	lora_send_cmd(sendbuf,"OK",50);
	sprintf((char*)sendbuf,"AT+UART=%d,%d",LoRa_CFG.bps,LoRa_CFG.parity);//���ô��ڲ����ʡ�����У��λ
	lora_send_cmd(sendbuf,"OK",50);

	LORA_MD0=0;//�˳�����,����ͨ��
	delay_ms(40);
	while(LORA_AUX);//�ж��Ƿ����(ģ����������ò���)
	USART2_RX_STA=0;
	Lora_mode=1;//���"����ģʽ"
	usart2_set(LoRa_CFG.bps,LoRa_CFG.parity);//����ͨ��,����ͨ�Ŵ�������(�����ʡ�����У��λ)
	Aux_Int(1);//����LORA_AUX�������ж�	
	
}

u8 Dire_Date[]={0x11,0x22,0x33,0x44,0x55};//����������
u8 date[30]={0};//��������

#define Dire_DateLen sizeof(Dire_Date)/sizeof(Dire_Date[0])
extern u32 obj_addr;//��¼�û�����Ŀ���ַ
extern u8 obj_chn;//��¼�û�����Ŀ���ŵ�
u8 Tran_Data[200]={0};//͸������

u8 wlcd_buff[10]={0}; //LCD��ʾ�ַ���������
//Loraģ�鷢������
void LoRa_SendData(char* message)
{      
	static u8 num=0;
    u16 addr;
	u8 chn;
	u16 i=0; 
		
	Lora_mode=2;//���"����״̬"
	if(LoRa_CFG.mode_sta == LORA_STA_Tran)//͸������
	{
		sprintf((char*)Tran_Data,"%s",message);
		u2_printf("%s\r\n",Tran_Data);
		
		printf("Send��%s\r\n",Tran_Data);//��ʾ���͵�����	
	}else if(LoRa_CFG.mode_sta == LORA_STA_Dire)//������
	{
		
		addr = (u16)obj_addr;//Ŀ���ַ
		chn = obj_chn;//Ŀ���ŵ�
		
		date[i++] =(addr>>8)&0xff;//��λ��ַ
		date[i++] = addr&0xff;//��λ��ַ
		date[i] = chn;//�����ŵ�
		
		for(i=0;i<Dire_DateLen;i++)//����д������BUFF
		{
			date[3+i] = Dire_Date[i];
		}	
		for(i=0;i<(Dire_DateLen+3);i++)
		{
			while(USART_GetFlagStatus(USART2,USART_FLAG_TC)==RESET);//ѭ������,ֱ���������   
			USART_SendData(USART2,date[i]); 
		}	
		
        //��ʮ�����Ƶ�����ת��Ϊ�ַ�����ӡ��lcd_buff����
		sprintf((char*)wlcd_buff,"%x %x %x %x %x %x %x %x",
				date[0],date[1],date[2],date[3],date[4],date[5],date[6],date[7]);
		
	    Dire_Date[4]++;//Dire_Date[4]���ݸ���
	}
			
}


int LoRa_SendData_Affirm(char* message)
{
	//1.��
	//2.��
	//3.����
	
	int times = 3;
	int waittime = 50;
	while(times > 0){
		LoRa_SendData(message);
		while(waittime > 0){	//�ȴ�����ʱ 
			waittime--;
			delay_ms(10);
			if(USART2_RX_STA&0X8000){ //������һ������
			//����Ŀ���Ӵ�
				USART2_RX_BUF[USART2_RX_STA&0X7FFF]=0;//��ӽ�����
				if(strstr((const char *)USART2_RX_BUF, (const char *)"RecOK") != NULL){
					memset((char*)USART2_RX_BUF,0x00,USART2_RX_STA&0X7FFF);//���ڽ��ջ�������0
					
					alarmFlag = 0;
					return 0;
				}
				memset((char*)USART2_RX_BUF,0x00,USART2_RX_STA&0X7FFF);//���ڽ��ջ�������0
			} 
		}
		#if 0
		delay_ms(300);
		
		u16 i = 0;
		u16 len=0;
		//����������
		if(USART2_RX_STA&0x8000){
			len = USART2_RX_STA&0X7FFF;
			USART2_RX_BUF[len]=0;//��ӽ�����
			USART2_RX_STA=0;

			for(i=0;i<len;i++)
			{
				while(USART_GetFlagStatus(USART1,USART_FLAG_TC)==RESET); //ѭ������,ֱ���������   
				USART_SendData(USART1,USART2_RX_BUF[i]); 
			}
			if(LoRa_CFG.mode_sta==LORA_STA_Tran)//͸������
			{	
				printf("%s",USART2_RX_BUF);//��ʾ���յ�������

			}
			
			if(strstr((const char *)USART2_RX_BUF, (const char *)"RecOK") != NULL){
				memset((char*)USART2_RX_BUF,0x00,len);//���ڽ��ջ�������0
				return 0;
			}
			
			memset((char*)USART2_RX_BUF,0x00,len);//���ڽ��ջ�������0
		}
		#endif
		times--;
	}
	
	printf("Send Failed");
	alarmFlag = -1;
	return -1;
}

u8 rlcd_buff[10]={0}; //LCD��ʾ�ַ���������

int peopleNum = 0;
//�ɵ��� x1
volatile int lightStatus = 0, lightPWM = 0;
//���� x2
volatile int fanStatus1 = 0, fanSpeed1 = 0;
volatile int fanStatus2 = 0, fanSpeed2 = 0;
//������� x2
volatile int motorStatus1 = 0, motorSpeed1 = 0, motorDirection1 = 0;
volatile int motorStatus2 = 0, motorSpeed2 = 0, motorDirection2 = 0;
//������俪�� x2
volatile int infraredStatus1 = 0;
volatile int infraredStatus2 = 0;
//������ x1
volatile int buzzerStatus = 0;
//LED�� x4
volatile int LEDStatus1 = 0, LEDStatus2 = 0, LEDStatus3 = 0, LEDStatus4 = 0;
//��ʪ�ȴ�����
volatile int h1 = 0, h2 = 0, t1 = 0, t2 = 0;
//���մ����� x2
volatile int i1 = 0, i2 = 0;
//�������� x2
volatile int s1 = 0, s2 = 0;
volatile int wifiConnectStatus  = 0;
volatile int serverConnectStatus = 0;

int wifi = 0;
int server = 0;

//Loraģ���������
int speed_flag = 0;//0�� 1��
void LoRa_ReceData(void)
{
   // u16 i=0;
    u16 len=0;
   
	//����������
	if(USART2_RX_STA&0x8000)
	{
		len = USART2_RX_STA&0X7FFF;

		if(LoRa_CFG.mode_sta==LORA_STA_Tran)//͸������
		{	
			lora_at_response(1);//��ʾ���յ�������

		}else if(LoRa_CFG.mode_sta==LORA_STA_Dire)//������
		{
			//��ʮ�����Ƶ�����ת��Ϊ�ַ�����ӡ��lcd_buff����
			sprintf((char*)rlcd_buff,"%x %x %x %x %x",
			USART2_RX_BUF[0],USART2_RX_BUF[1],USART2_RX_BUF[2],USART2_RX_BUF[3],USART2_RX_BUF[4]);
				
			//Show_Str_Mid(10,270,rlcd_buff,16,240);//��ʾ���յ�������	
			lora_at_response(0);
		}
		
		/****************�����ݵĴ����߼�*********************/
		
		uint8_t TxetBuf[128];
			
		//����ǿ��
		int sensorFlag = 0;
		if(strstr((const char *)USART2_RX_BUF, (const char *)"i1") != NULL){
			i1 = (int) USER_GetSteeringEngineJsonValue(USART2_RX_BUF, "i1");  
			sensorFlag = 1;
		}
		else if(strstr((const char *)USART2_RX_BUF, (const char *)"i2") != NULL){
			i2 = (int)USER_GetSteeringEngineJsonValue(USART2_RX_BUF, "i2");
			sensorFlag = 2;
		}
		//ʪ��
		if(strstr((const char *)USART2_RX_BUF, (const char *)"h1") != NULL){
			h1 = (int)USER_GetSteeringEngineJsonValue(USART2_RX_BUF, "h1");
			sensorFlag = 1;
		}
		else if(strstr((const char *)USART2_RX_BUF, (const char *)"h2") != NULL){
			h2 = (int)USER_GetSteeringEngineJsonValue(USART2_RX_BUF, "h2");
			sensorFlag = 2;
		}
		//�¶�
		if(strstr((const char *)USART2_RX_BUF, (const char *)"t1") != NULL){
			t1 = (int)USER_GetSteeringEngineJsonValue(USART2_RX_BUF, "t1");
			sensorFlag = 1;
		}
		else if(strstr((const char *)USART2_RX_BUF, (const char *)"t2") != NULL){
			t2 = (int)USER_GetSteeringEngineJsonValue(USART2_RX_BUF, "t2");
			sensorFlag = 2;
		}
		//����Ũ��
		if(strstr((const char *)USART2_RX_BUF, (const char *)"s1") != NULL){
			s1 = (int)USER_GetSteeringEngineJsonValue(USART2_RX_BUF, "s1");
			sensorFlag = 1;
		}
		else if(strstr((const char *)USART2_RX_BUF, (const char *)"s2") != NULL){
			s2 = (int)USER_GetSteeringEngineJsonValue(USART2_RX_BUF, "s2");
			sensorFlag = 2;
		}	
	
		memset((char*)USART2_RX_BUF,0x00,len);//���ڽ��ջ�������0
	}

}


int lightLastTime = 0;
int lightFlag = 0;
int motorFlag = 0;
void Lora_task(void)
{
	LoRa_ReceData();
		
}

void Lora_Test(void)
{	
	//delay_ms(6000);
    delay_ms(1000);
	while(LoRa_Init())//��ʼ��ATK-LORA-01ģ�� ��ʼ��Ϊ��д��ģʽ
	{
		printf("δ��⵽ģ��!!!\r\n"); 	
		delay_ms(300);
		printf("                \r\n");
	}
	printf("��⵽ģ��!!!\r\n");
    delay_ms(500); 	
	
	//1.��������ģʽ
	LORA_MD0=1; //��������ģʽ
	delay_ms(40);
	usart2_rx(0);//�رմ��ڽ���
	Aux_Int(0);//�ر��ж�
	
	LoRa_CFG.addr = 0x5410;
	LoRa_CFG.chn = 0x10;
	LoRa_CFG.power = LORA_PW_20Bbm;
	LoRa_CFG.wlrate = LORA_RATE_19K2;
	LoRa_CFG.wltime = LORA_WLTIME_1S;
	LoRa_CFG.mode = LORA_MODE_GEN;
	LoRa_CFG.mode_sta = LORA_STA_Tran;//LORA_STA_Dire
	LoRa_CFG.bps = LORA_TTLBPS_115200;
	LoRa_CFG.parity = LORA_TTLPAR_8N1;	
	LoRa_Set();//�Զ���usart2_rx(1);
}

