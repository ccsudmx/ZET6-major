#include "lora_app.h"
#include "lora_ui.h"
#include "usart3.h"
#include "string.h"
#include "delay.h"
#include "stdio.h"
#include "fan.h"
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
NVIC_InitTypeDef NVIC_InitStructure;

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
		NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;	
	}else
	{
		if(mode==1)
			 EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; //������
	    else if(mode==2)
			 EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;//�½���
		
		EXTI_InitStructure.EXTI_LineCmd = ENABLE;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		
	}
	Int_mode = mode;//��¼�ж�ģʽ
	EXTI_Init(&EXTI_InitStructure);
	NVIC_Init(&NVIC_InitStructure);  
    
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
			 USART3_RX_STA=0;//���ݼ�����0
		  }
		  Int_mode=2;//�����½��ش���
	   }
       else if(Int_mode==2)//�½���(����:�����ѷ����� ����:�����������)	
	   {
		  if(Lora_mode==1)//����ģʽ
		  {
			 USART3_RX_STA|=1<<15;//���ݼ���������
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
	
	 NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;			//LORA_AUX
  	 NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;//��ռ���ȼ�2�� 
  	 NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;		//�����ȼ�3
  	 NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE; //�ر��ⲿ�ж�ͨ���������ٴ򿪣�
   	 NVIC_Init(&NVIC_InitStructure); 
	 
	 LORA_MD0=0;
	 LORA_AUX=0;
	
	 while(LORA_AUX)//ȷ��LORAģ���ڿ���״̬��(LORA_AUX=0)
	 {
		 printf("ģ����æ,���Ե�!!\n"); 	
		 delay_ms(500);
	 }
	 usart3_init(115200);//��ʼ������3
	 
	 LORA_MD0=1;//����ATģʽ
	 delay_ms(40);
	 retry=3;
	 
	 while(retry--)
	 {
		 if(!lora_send_cmd("ATE0","OK",70)) //OK
		 {
			 temp=0;//���ɹ�
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
	
	usart3_set(LORA_TTLBPS_115200,LORA_TTLPAR_8N1);//��������ģʽǰ����ͨ�Ų����ʺ�У��λ(115200 8λ���� 1λֹͣ ������У�飩
	usart3_rx(1);//��������3����
	
	while(LORA_AUX);//�ȴ�ģ�����
	LORA_MD0=1; //��������ģʽ
	delay_ms(40);
	Lora_mode=0;//���"����ģʽ"
	
	lora_addrh =  (LoRa_CFG.addr>>8)&0xff;
	lora_addrl = LoRa_CFG.addr&0xff;
    sprintf((char*)sendbuf,"AT+CGMR?");//�����豸��ַ
	lora_send_cmd(sendbuf,"OK",50);
   
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
	USART3_RX_STA=0;
	Lora_mode=1;//���"����ģʽ"
	usart3_set(LoRa_CFG.bps,LoRa_CFG.parity);//����ͨ��,����ͨ�Ŵ�������(�����ʡ�����У��λ)
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
  u16 addr;
	u8 chn;
	u16 i=0; 
	
	Lora_mode=2;//���"����״̬"
	
	if(LoRa_CFG.mode_sta == LORA_STA_Tran)//͸������
	{
		sprintf((char*)Tran_Data,"%s",message);
		u3_printf("%s\r\n",Tran_Data);

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
			while(USART_GetFlagStatus(USART3,USART_FLAG_TC)==RESET);//ѭ������,ֱ���������   
			USART_SendData(USART3,date[i]); 
		}	
		
        //��ʮ�����Ƶ�����ת��Ϊ�ַ�����ӡ��lcd_buff����
		sprintf((char*)wlcd_buff,"%x %x %x %x %x %x %x %x",
				date[0],date[1],date[2],date[3],date[4],date[5],date[6],date[7]);
		
	    Dire_Date[4]++;//Dire_Date[4]���ݸ���
		
	}
			
}

u8 rlcd_buff[10]={0}; //LCD��ʾ�ַ���������



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
	//printf("ȡֵ�ڲ�%s %s %s %s",cJson,Tag,USART3_RX_BUF,temp);
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
 //printf("��ֵstr=%s\r\n",temp);
	//printf("��ֵstr=%s\r\n",target);
 len=strlen((const char *)target);
	for(i=0; i<len;i++)
    {
	// printf("��ֵstr[%d]=0x%x  \r\n",i,*(target+i));
	}
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
volatile int recFlag=0;

//Loraģ���������
void LoRa_ReceData(void)
{
//    u16 i=0;
        u16 len=0;
		int count=0;
		u8 msg[1024];
    
	if(USART3_RX_STA&0x8000)
	{  		
			
			while(USART3_RX_BUF[count]!='\0')
			{
			    msg[count]=USART3_RX_BUF[count];
				  count++;
			
			}
			count++;
			msg[count]='\0';
			printf("�Ѿ����յ�������\r\n");
//		len = USART3_RX_STA&0X7FFF;
//		USART3_RX_BUF[len]=0;//��ӽ�����
   	//	USART3_RX_STA=0;

//		for(i=0;i<len;i++)
//		{
//			while(USART_GetFlagStatus(USART1,USART_FLAG_TC)==RESET); //ѭ������,ֱ���������   
//			USART_SendData(USART1,USART3_RX_BUF[i]); 
//		}
		if(LoRa_CFG.mode_sta==LORA_STA_Tran)//͸������
		{	
			lora_at_response(1);//��ʾ���յ�������
		}
		int recFlag = 0;
		//���ݽ���
		//arr+1~0
		//ռ�ձȼ��㣺fan0pwmval/arr+1;  ���Ҳ����5500/7200
		//FANռ�ձȴ���%50���Ͳ�ת��
		//LIGHTռ�ձȿ��Դ�%0~%100
		//FAN1
		if(strstr((const char *)USART3_RX_BUF, (const char *)"fanStatus1") != NULL){
		
			//printf("�ս���msg=%s\r\n",msg);
			recFlag++;
			fanStatus1 = (int) USER_GetSteeringEngineJsonValue((char *)msg, (char *)"fanStatus1");  
			//char temp[20];
			//sprintf(temp,"%d,%d",fanStatus1,fanSpeed1);
			//printf("%s",temp);
			//printf("������%s",USART3_RX_BUF);
			if(fanStatus1 == 0){
				TIM_SetCompare1(TIM1,100*72);
				LoRa_SendData("FAN1 OFF\r\n");
				printf("FAN1 OFF\r\n");
		    
			}
			else if(fanStatus1 == 1){
				TIM_SetCompare1(TIM1,fanVal1);
				LoRa_SendData("FAN1 ON\r\n");
				printf("FAN1 ON\r\n");
			}
		}
		if(strstr((const char *)msg, (const char *)"fanSpeed1") != NULL){
			recFlag++;
			fanSpeed1 = USER_GetSteeringEngineJsonValue((char *)msg, (char *)"fanSpeed1");
			
			fanVal1 = (int)(100 - fanSpeed1)*72;
			if(fanStatus1 == 1)
				TIM_SetCompare1(TIM1,fanVal1);	  //ͨ���Ƚ�ֵ   �޸�TIM1_CCR1 ռ�ձ�
			
			printf("FAN1 = %d%%\r\n", fanSpeed1);
		}
		
		//FAN2
		if(strstr((const char *)USART3_RX_BUF, (const char *)"fanStatus2") != NULL){
			recFlag++;
			fanStatus2 = USER_GetSteeringEngineJsonValue((char *)USART3_RX_BUF, (char *)"fanStatus2");
			
			if(fanStatus2 == 0){
				TIM_SetCompare4(TIM1,100*72);
				printf("FAN2 OFF\r\n");
			}
			else if(fanStatus2 == 1){
				TIM_SetCompare4(TIM1,fanVal2);
				printf("FAN2 ON\r\n");
			}
		}
		if(strstr((const char *)USART3_RX_BUF, (const char *)"fanSpeed2") != NULL){
			recFlag++;
			fanSpeed2 = USER_GetSteeringEngineJsonValue((char *)USART3_RX_BUF, (char *)"fanSpeed2");
			
			fanVal2 = (int)(100 - fanSpeed2)*72;
			
			if(fanStatus2 == 1)
				TIM_SetCompare4(TIM1,fanVal2);	  //ͨ���Ƚ�ֵ   �޸�TIM1_CCR1 ռ�ձ�
			printf("FAN2 = %d%%\r\n", fanSpeed2);
		}
		
		if(recFlag > 0){
			//ȷ����Ϣ�ط�
			LoRa_SendData("FanRecOK");
			recFlag=0;
		}

		if(strstr((const char *)msg, (const char *)"lightStatus") != NULL){
			recFlag++;
			lightStatus = USER_GetSteeringEngineJsonValue((char *)msg, (char *)"lightStatus");
			
			if(lightStatus == 0){
				TIM_SetCompare4(TIM1,100*72);
				printf("LIGHT OFF\r\n");
			}
			else if(lightStatus == 1){
				TIM_SetCompare4(TIM1,lightVal);
				printf("LIGHT ON\r\n");
			}
		}
		if(strstr((const char *)msg, (const char *)"lightPWM") != NULL){
			recFlag++;
			lightPWM = USER_GetSteeringEngineJsonValue((char *)msg, (char *)"lightPWM");
			
			lightVal = (int)(100 - lightPWM)*72;
			
			if(lightStatus == 1)
				TIM_SetCompare4(TIM1,lightVal);	  //ͨ���Ƚ�ֵ   �޸�TIM1_CCR1 ռ�ձ�
			printf("LIGHT = %d%%\r\n", lightPWM);
		}
		if(recFlag > 0){
			//ȷ����Ϣ�ط�
			LoRa_SendData("LightRecOK");
		}
		
	
		memset((char*)USART3_RX_BUF,0x00,len);//���ڽ��ջ�������0
		memset((char *)msg,0x00,count);
		count=0;
	}


	
}

#if 0
//���ͺͽ��մ���
void LoRa_Process(void)
{
	u8 key=0;
	u8 t=0;
		
 DATA:
	Process_ui();//������ʾ
	LoRa_Set();//LoRa����(�������������ô��ڲ�����Ϊ115200) 
	while(1)
	{
		
		key = KEY_Scan(0);
		
		if(key==KEY0_PRES)
		{
			if(LoRa_CFG.mode_sta==LORA_STA_Dire)//���Ƕ�����,���������Ŀ���ַ���ŵ�����
			{
				usart3_rx(0);//�رմ��ڽ���
				Aux_Int(0);//�ر��ж�
				Dire_Set();//��������Ŀ���ַ���ŵ�
				goto DATA;
			}
		}else if(key==WKUP_PRES)//�������˵�ҳ��
		{
			LORA_MD0=1; //��������ģʽ
	        delay_ms(40);
			usart3_rx(0);//�رմ��ڽ���
			Aux_Int(0);//�ر��ж�
			break;
		}
		else if(key==KEY1_PRES)//��������
		{
			  if(!LORA_AUX&&(LoRa_CFG.mode!=LORA_MODE_SLEEP))//�����ҷ�ʡ��ģʽ
			  {
				  Lora_mode=2;//���"����״̬"
				  LoRa_SendData();//��������    
			  }
		}	
		//���ݽ���
		//usart3_rx(1);
		LoRa_ReceData();
		
		t++;
		if(t==20)
		{
			t=0;
			LED1=~LED1;
		}			
		delay_ms(10);		
   }
	
}

#endif

void Lora_Test(void)
{
	delay_ms(6000);
	while(LoRa_Init())//��ʼ��ATK-LORA-01ģ�� ��ʼ��Ϊ��д��ģʽ
	{
		printf("δ��⵽ģ��!!!\r\n"); 	
		delay_ms(300);
	}
	printf("��⵽ģ��!!!\r\n");
  delay_ms(500); 	
	
	//1.��������ģʽ
	LORA_MD0=1; //��������ģʽ
	delay_ms(40);
	usart3_rx(0);//�رմ��ڽ���
	Aux_Int(0);//�ر��ж�
	
	LoRa_CFG.addr = 0x5410; // 0x5410
	LoRa_CFG.chn = 0x10; //0x20
	LoRa_CFG.power = LORA_PW_20Bbm;
	LoRa_CFG.wlrate = LORA_RATE_19K2;
	LoRa_CFG.wltime = LORA_WLTIME_1S;
	LoRa_CFG.mode = LORA_MODE_GEN;
	LoRa_CFG.mode_sta = LORA_STA_Tran;//LORA_STA_Dire
	LoRa_CFG.bps = LORA_TTLBPS_115200;
	LoRa_CFG.parity = LORA_TTLPAR_8N1;	
	LoRa_Set();

}
