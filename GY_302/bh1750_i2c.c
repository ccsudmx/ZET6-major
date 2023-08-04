#if 0

#include "bh1750_i2c.h"

//BH1750дһ���ֽ�
//����ֵ	�ɹ���0		ʧ�ܣ���0 
uint8_t BH1750_Byte_Write(uint8_t data)
{
	IIC_Start();
	//����д��ַ
	IIC_Send_Byte(BH1750_Addr|0);
	if(IIC_Wait_Ack()==1)
		return 1;
	//���Ϳ�������
	IIC_Send_Byte(data);
	if(IIC_Wait_Ack()==1)
		return 2;
	IIC_Stop();
	return 0;
}

//BH1750��ȡ��������
//����ֵ �ɹ������ع���ǿ�� 	ʧ�ܣ�����0
uint16_t BH1750_Read_Measure(void)
{
	uint16_t receive_data=0; 
	IIC_Start();
	//���Ͷ���ַ
	IIC_Send_Byte(BH1750_Addr|1);
	if(IIC_Wait_Ack()==1)
		return 0;
	//��ȡ�߰�λ
	receive_data=IIC_Read_Byte(0); //0 or 1??????????
	IIC_Ack();
	//��ȡ�Ͱ�λ
	receive_data=(receive_data<<8)+IIC_Read_Byte(0);//0 or 1??????????
	IIC_NAck();
	IIC_Stop();
	return receive_data;	//���ض�ȡ��������
}


//BH1750s�ϵ�
void BH1750_Power_ON(void)
{
	BH1750_Byte_Write(POWER_ON);
}

//BH1750s�ϵ�
void BH1750_Power_OFF(void)
{
	BH1750_Byte_Write(POWER_OFF);
}

//BH1750��λ	�����ϵ�ʱ��Ч
void BH1750_RESET(void)
{
	BH1750_Byte_Write(MODULE_RESET);
}

//BH1750��ʼ��
void BH1750_Init(void)
{
	BH1750_Power_ON();	//BH1750s�ϵ�
	//BH1750_RESET();			//BH1750��λ
	BH1750_Byte_Write(Measure_Mode);
	delay_ms(120);
}

//��ȡ����ǿ��
float Light_Intensity(void)
{
	return (float)(BH1750_Read_Measure()/1.2f);
}

#endif
