#if 0

#include "bh1750_i2c.h"

//BH1750写一个字节
//返回值	成功：0		失败：非0 
uint8_t BH1750_Byte_Write(uint8_t data)
{
	IIC_Start();
	//发送写地址
	IIC_Send_Byte(BH1750_Addr|0);
	if(IIC_Wait_Ack()==1)
		return 1;
	//发送控制命令
	IIC_Send_Byte(data);
	if(IIC_Wait_Ack()==1)
		return 2;
	IIC_Stop();
	return 0;
}

//BH1750读取测量数据
//返回值 成功：返回光照强度 	失败：返回0
uint16_t BH1750_Read_Measure(void)
{
	uint16_t receive_data=0; 
	IIC_Start();
	//发送读地址
	IIC_Send_Byte(BH1750_Addr|1);
	if(IIC_Wait_Ack()==1)
		return 0;
	//读取高八位
	receive_data=IIC_Read_Byte(0); //0 or 1??????????
	IIC_Ack();
	//读取低八位
	receive_data=(receive_data<<8)+IIC_Read_Byte(0);//0 or 1??????????
	IIC_NAck();
	IIC_Stop();
	return receive_data;	//返回读取到的数据
}


//BH1750s上电
void BH1750_Power_ON(void)
{
	BH1750_Byte_Write(POWER_ON);
}

//BH1750s断电
void BH1750_Power_OFF(void)
{
	BH1750_Byte_Write(POWER_OFF);
}

//BH1750复位	仅在上电时有效
void BH1750_RESET(void)
{
	BH1750_Byte_Write(MODULE_RESET);
}

//BH1750初始化
void BH1750_Init(void)
{
	BH1750_Power_ON();	//BH1750s上电
	//BH1750_RESET();			//BH1750复位
	BH1750_Byte_Write(Measure_Mode);
	delay_ms(120);
}

//获取光照强度
float Light_Intensity(void)
{
	return (float)(BH1750_Read_Measure()/1.2f);
}

#endif
