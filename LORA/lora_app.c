#include "lora_app.h"
#include "lora_ui.h"
#include "usart3.h"
#include "string.h"
#include "delay.h"
#include "stdio.h"
#include "fan.h"
#include "my_json.h"
 Json Fan=
{
	.name="FAN1",
	.status=0,
	.value=0,
	
};
Json tep={
  .name="TH",
  .status=1,
  .value=0,
   

};
Json time={
    .name="TIME",
    .hours=0,
    .min=0,
    .sec=0   

};

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


u8 Tran_Data[200]={0};//͸������


//Loraģ�鷢������
void LoRa_SendData(char* message)
{   	
   

	
	Lora_mode=2;//���"����״̬"
	
	if(LoRa_CFG.mode_sta == LORA_STA_Tran)//͸������
	{
		sprintf((char*)Tran_Data,"%s",message);
		u3_printf("%s",Tran_Data);

		printf("Send��%s\r\n",Tran_Data);//��ʾ���͵�����	
	}

			
}

int TH=0;

//Loraģ���������
void LoRa_ReceData(void)
{

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
            
		USART3_RX_BUF[USART3_RX_STA&0X7FFF]=0;//��ӽ�����
        USART3_RX_STA=0;
	
		if(strstr((const char *)msg, (const char *)"FAN1") != NULL){
		    
		       JSON_LORA((char *)msg,&Fan);
            if(Fan.status)
            {
			   TIM_SetCompare1(TIM1,(100-Fan.value)*72);
			   LoRa_SendData("FAN1 ON!");
				                         
            }
            else {
              TIM_SetCompare1(TIM1,100*72);
              LoRa_SendData("FAN1 OFF\r\n");
                  }
                  printf("FAN1_NEW_State: %s:%d:%d\r\n",Fan.name,Fan.status,Fan.value);
                
			}
        
        if(strstr((const char *)msg,(const char *) "TH")!=NULL)
        {
            
           JSON_LORA((char *)msg,&tep);
           TH=tep.value;
        
        
        }
        if(strstr((const char *)msg,(const char *)"TIME")!=NULL)
        {
           
              Json_time((char *)msg,&time);
        
        
        }
      
		
	
		memset((char*)USART3_RX_BUF,0x00,USART3_MAX_RECV_LEN);//���ڽ��ջ�������0
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
