#if 0
#include "i2c_bh1750.h"
/*
��������: IIC���߳�ʼ��
Ӳ������:
        SCL-PB3
        SDA-PA15
*/
u8  BUF[3];
 
void BH1750_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOB, ENABLE );	
	   
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;   //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB,GPIO_Pin_6|GPIO_Pin_7); 	//PB6,PB7 �����
	//printf("BH1750 IIC_Init OK�� \n");
}
 
/**************************************
��ʼ�ź�
**************************************/
void BH1750_Start(void)
{
	IIC_OUTPUT_MODE_SET(); //�������ģʽ
	IIC_SDA_OUT = 1;       //����������
	IIC_SCL = 1;           //����ʱ����
	delay_us(5);            //��ʱ
	IIC_SDA_OUT = 0;       //�����½���
	delay_us(5);            //��ʱ
	IIC_SCL = 0;           //����ʱ����
}
 
/*
��������: ֹͣ�ź�
*/
void BH1750_Stop(void)
{
	IIC_OUTPUT_MODE_SET(); 	//�������ģʽ
	IIC_SDA_OUT = 0;        //����������
	IIC_SCL = 1;            //����ʱ����
	delay_us(5);             //��ʱ
	IIC_SDA_OUT = 1;        //����������
	delay_us(5);             //��ʱ
}
 
/**************************************
����Ӧ���ź�
��ڲ���:ack (0:ACK 1:NAK)
**************************************/
void BH1750_SendACK(char ack)
{
	IIC_OUTPUT_MODE_SET(); 	  //�������ģʽ
	IIC_SCL = 0;              //����ʱ����
	if(ack) IIC_SDA_OUT = 1; 
	else IIC_SDA_OUT = 0;     //дӦ���ź�
	delay_us(2);               //��ʱ
	IIC_SCL = 1;              //����ʱ����
	delay_us(5);               //��ʱ
	IIC_SCL = 0;              //����ʱ����
}
 
/**************************************
����Ӧ���ź�
**************************************/
char BH1750_RecvACK(void)
{
	u8 cnt=0;
	IIC_INPUT_MODE_SET();//����ģʽ
	IIC_SDA_OUT=1; 
	delay_us(1);
	IIC_SCL=1;
	delay_us(1);
	while(IIC_SDA_IN)//�ȴ�Ӧ��
	{
		cnt++;
		delay_us(1);
		if(cnt>=250)//�ȴ�ʱ�����������ֹͣ�źţ�����1����ʾ����Ӧ��ʧ��
		{
			BH1750_Stop();
			//printf("N0 ACK\n");
			return 1;
		}
	}
	IIC_SCL=0;//Ӧ��ɹ�����SCL���
	return 0;
}
 
/**************************************
��IIC���߷���һ���ֽ�����
**************************************/
void BH1750_SendByte(u8 data)
{
	u8 i;
	IIC_OUTPUT_MODE_SET();
	IIC_SCL = 0;//����ʱ����, ����׼������
	delay_us(2);//��ʱ
	for (i=0;i<8;i++)//�Ӹ�λ��ʼһλһλ�ش���
	{
		if(data&0x80) IIC_SDA_OUT=1;//�����ݿ�
		else IIC_SDA_OUT=0;
		data <<= 1;//�Ƴ����ݵ����λ
		IIC_SCL = 1; //����ʱ����,��������
		delay_us(2);//��ʱ
		IIC_SCL = 0;//����ʱ���ߣ����ݷ������
		delay_us(2);//��ʱ
	}
}
/**************************************
��IIC���߽���һ���ֽ�����
**************************************/
u8 BH1750_RecvByte(void)
{
	u8 i;
	u8 data = 0;
	IIC_INPUT_MODE_SET();//ʹ���ڲ�����,׼����ȡ����,
	for (i=0;i<8;i++)//8λ������
	{
		IIC_SCL=0;//��ʱ����Ϊ�ͣ�׼����������λ
		delay_us(2);//ʱ�ӵ͵�ƽ���ڴ���4.7��s
		IIC_SCL=1;//��ʱ����Ϊ��ʹ��������������Ч��������ʼ�����ݣ��ӻ������ٸı������ˣ����ı�SDA�ĵ�ƽ
		data <<= 1;
		IIC_SCL = 1;//����ʱ����
		if(IIC_SDA_IN) data|=0x01;//������               
		delay_us(1);//��ʱ
	}
	IIC_SCL=0;
	return data;
}
 
void Single_Write_BH1750(u8 REG_Address)
{
	BH1750_Start();//��ʼ�ź�
	BH1750_SendByte(SlaveAddress);//�����豸��ַ+д�ź�
	BH1750_RecvACK();
	BH1750_SendByte(REG_Address);//�ڲ��Ĵ�����ַ����ο�����pdf22ҳ 
	BH1750_RecvACK();
	BH1750_Stop();//����ֹͣ�ź�
}
/*********************************************************
��������BH1750�ڲ�����
*********************************************************/
void Multiple_Read_BH1750(void)
{   
  BH1750_Start();//��ʼ�ź�
  BH1750_SendByte(SlaveAddress+1);//�����豸��ַ+���ź�
	BH1750_RecvACK();
	BUF[0] = BH1750_RecvByte();//BUF[0]�洢0x32��ַ�е�����
	BH1750_SendACK(0);
	BUF[1] = BH1750_RecvByte();//BUF[0]�洢0x32��ַ�е�����
	BH1750_SendACK(1);
	BH1750_Stop();//ֹͣ�ź�
	delay_ms(5);
}
 
/*********************************************************
�������ݲ��ϳ�ʵ�ʹ�ǿֵ
*********************************************************/
float Get_Sunlight_Value(void)
{
	int dis_data=0;
	float temp;
	u8 i=0;
	Single_Write_BH1750(0x01);// power on
	Single_Write_BH1750(0x10);// H- resolution mode
	delay_ms(180);//��ʱ180ms
	Multiple_Read_BH1750();//�����������ݣ��洢��BUF��  
	for(i=0;i<3;i++)
	//printf("0X%X ",BUF[i]);
	dis_data=BUF[0];
	dis_data=(dis_data<<8)+BUF[1];//�ϳ����� 
	temp=(float)dis_data/1.2;
	return temp;
	//printf("Sunlight=%0.2f lx\n",temp);
}

#endif
