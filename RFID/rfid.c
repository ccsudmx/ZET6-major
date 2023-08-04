

#include "stm32f10x.h"
#include "usart3.h"
#include "usart.h"
#include "rfid.h"
#include "stdio.h"
#include "usart.h"

unsigned char Uart3RxBuf[UART3_RX_BUF_LEN];
unsigned char Uart3RxDataConut = 0;
unsigned char Rx3Flag = 0;

unsigned char Cmd_Read_Id[8] = {0x01,0x08,0xa1,0x20,0x00,0x00,0x00,0x00};
unsigned char Cmd_Read_Block[8]	= {0x01,0x08,0xa3,0x20,0x00,0x00,0x00,0x00};
unsigned char Cmd_Write_Block[23] = {0x01,0x17,0xa4,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

unsigned char WBlockData[16] = {0x11,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f};
//CircularBuffer *Uart2_Circular_Buffer;
unsigned char Cmd_Write_RFID[0x0B]={0x03, 0x0B, 0xC5, 0x20, 0x05, 0x12, 0x34, 0x56, 0x78, 0x09, 0x16};
 //                          命令类型  包长度  命令  设备地址  起始地址  数据长度   保留  校验和
unsigned char Cmd_Read_RFID[]={0x02,   0x08,   0xB5,  0x20,    0x03,      0x06,     0x00,  0x65};

//延时，10000000大约为1S
void Delay(__IO unsigned int nCount)
{
  for (; nCount != 0; nCount--);
}

void Uart3_Send_Data(unsigned char *buf,unsigned char num)
{
	unsigned char i;
	for(i=0;i<num;i++)
	{ 
	 	USART_SendData(USART3, buf[i]);
	 	while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
	}	
}

unsigned char RxCheckSum(unsigned char *ptr,unsigned char len)
{
	unsigned char i;
	unsigned char checksum;
	checksum = 0;
	for(i=0;i<(len-1);i++)
	{
		   checksum ^= ptr[i];
	}
	checksum = ~checksum;
	if(ptr[len-1] == checksum)
		return 	STATUS_OK;
	else 
		return 	STATUS_ERR;
}

void TxCheckSum(unsigned char *ptr,unsigned char len)
{
	unsigned char i;
	unsigned char checksum;
	checksum = 0;
	for(i=0;i<(len-1);i++)
	{
		   checksum ^= ptr[i];
	}
	checksum = ~checksum;
	ptr[len-1] = checksum;
}
//ReadId（）:读IC卡ID号（卡号）
//参数：*idout，读取的卡号保存到它所指向的存储空间
//返回值：0：成功读取卡号，1：读卡号失败
unsigned char ReadId(void)
{
	unsigned char status;
	unsigned char i;
	unsigned char idout[6];
	Cmd_Read_Id[5] = 0x01;//开启蜂鸣器提示
	//Cmd_Read_Id[5] = 0x00;//关闭蜂鸣器提示
	TxCheckSum(Cmd_Read_Id,Cmd_Read_Id[1]);		//计算校验和
	Uart3_Send_Data(Cmd_Read_Id,Cmd_Read_Id[1]);		 //发送读卡号ID命令
	Delay(2000000);//等待模块返回数据，大于150MS
 	if(Rx3Flag == 1)
 	{	
		Rx3Flag = 0;
		status = RxCheckSum(Uart3RxBuf,Uart3RxBuf[1]);//对接收到的数据校验
		if(status != STATUS_OK)  //判断校验和是否正确
		{
			return STATUS_ERR;
		}
		status = Uart3RxBuf[4];
		if(status != STATUS_OK)	//判断是否正确的读到卡
		{
		 	return STATUS_ERR;
		}
		if((Uart3RxBuf[0] == 0x01)&&(Uart3RxBuf[2] == 0xa1))//判断是否为读卡号返回的数据包
            {  
                printf("IDCard: ");
			for(i=0;i<6;i++)//获取卡号ID，6字节		 
			{
				//idout[i] = Uart3RxBuf[i+5];//从数组的第5个字节开始为卡号，长度为6字节
                while(USART_GetFlagStatus(USART1,USART_FLAG_TC)==RESET);
                
                if(Uart3RxBuf[i+5]/16<10)
                printf("%c",Uart3RxBuf[i+5]/16+48);
                else  printf("%c",Uart3RxBuf[i+5]/16+55); 
                
                if(Uart3RxBuf[i+5]%16<10)
                  printf("%c ",Uart3RxBuf[i+5]%16+48);
                else  printf("%c ",Uart3RxBuf[i+5]%16+55);
                                                
			}
      
        printf("\n");
			return STATUS_OK;		 //成功返回0
		}
 	} 
	return STATUS_ERR;			//失败返回1
}


//ReadId（）:读IC卡数据块
//参数：*idout，读取的数据保存到它所指向的存储空间
//参数：block，块号
//返回值：0：成功读取，1：读读取失败
unsigned char ReadDataFromBlock(unsigned char *dataout,unsigned char block)
{
	unsigned char status;
	unsigned char i;
	Cmd_Read_Block[4] = block;
	Cmd_Read_Block[5] = 0x01;//开启蜂鸣器提示
//	Cmd_Read_Block[5] = 0x00;//关闭蜂鸣器提示
	TxCheckSum(Cmd_Read_Block,Cmd_Read_Block[1]);	//数据校验
	Uart3_Send_Data(Cmd_Read_Block,Cmd_Read_Block[1]);		 //发送读数据块命令
	Delay(2000000);//等待模块返回数据，大于150MS
 	if(Rx3Flag == 1)
 	{	
		Rx3Flag = 0;
		status = RxCheckSum(Uart3RxBuf,Uart3RxBuf[1]);//对接收到的数据校验
		if(status != STATUS_OK)		 //判断校验和是否正确
		{
			return 	STATUS_ERR;
		}
		status = Uart3RxBuf[4];		//获取返回包状态
		if(status != STATUS_OK)	//判断是否正确的读到卡
		{
			return STATUS_ERR;
		}
		if((Uart3RxBuf[0] == 0x01)&&(Uart3RxBuf[2] == 0xa3))//判断是否为读块数据返回的数据包
		{
			for(i=0;i<16;i++)//获取块数据，16字节	，一个数据块的大小为16字节	 
			{
				dataout[i] = Uart3RxBuf[i+5];//从数组的第5个字节开始为数据，长度为16字节
			}
			return STATUS_OK;		 //成功返回0
		}
	}
	return STATUS_ERR;			//失败返回1
}
//ReadId（）:写数据到指定的数据块
//参数：*idout，指向要写入数据的缓冲区
//参数：block，块号
//返回值：0：写入成功，1：写入失败
unsigned char WriteDataToBlock(unsigned char *datain,unsigned char block)
{
	unsigned char status;
	unsigned char i;
	Cmd_Write_Block[4] = block;
	for(i=0;i<16;i++)
	{
		Cmd_Write_Block[6+i] = datain[i];
	}
	TxCheckSum(Cmd_Write_Block,Cmd_Write_Block[1]);	//数据校验
	Uart3_Send_Data(Cmd_Write_Block,Cmd_Write_Block[1]);		 //发送写命令
	Delay(2000000);//等待模块返回数据，大于150MS
 	if(Rx3Flag == 1)
 	{	
		Rx3Flag = 0;
		status = RxCheckSum(Uart3RxBuf,Uart3RxBuf[1]);//对返回的数据进行校验
		if(status != STATUS_OK) //判断校验是否通过
		{
			return STATUS_ERR;
		}
		status = Uart3RxBuf[4];
		if(status != STATUS_OK) //判断校验是否通过
		{
			return STATUS_ERR;
		}
		if((Uart3RxBuf[0] == 0x01)&&(Uart3RxBuf[2] == 0xa4))//判断是否为写块数据返回的数据包
		{
				return STATUS_OK;		 //成功返回0
		}
 	} 
	return STATUS_ERR;			//失败返回1
}

unsigned char Read_RFID(unsigned char addr,unsigned char len)
{
  unsigned char status;
  unsigned char i;
    Cmd_Read_RFID[4]=addr;
    Cmd_Read_RFID[5]=len;
	TxCheckSum(Cmd_Read_RFID,Cmd_Read_RFID[1]);		//计算校验和
	Uart3_Send_Data(Cmd_Read_RFID,Cmd_Read_RFID[1]);		 //发送读用户数据命令
	Delay(2000000);//等待模块返回数据，大于150MS
 	if(Rx3Flag == 1)
 	{	
        
		Rx3Flag = 0;
		status = RxCheckSum(Uart3RxBuf,Uart3RxBuf[1]);//对接收到的数据校验
		if(status != STATUS_OK)  //判断校验和是否正确
		{
			return STATUS_ERR;
		}
		status = Uart3RxBuf[4];
		if(status != STATUS_OK)	//判断是否正确的读到卡
		{
		 	return STATUS_ERR;
		}
		if((Uart3RxBuf[0] == 0x02)&&(Uart3RxBuf[2] == 0xB5))//判断是否为读RFID用户信息返回的数据包
		{
			
			for(i=0;i<Cmd_Read_RFID[5];i++)   //将读取的RFID打印到串口
		{
			while(USART_GetFlagStatus(USART1,USART_FLAG_TC)==RESET); 
			USART_SendData(USART1,Uart3RxBuf[i+5]+'0'); 
		}
			return STATUS_OK;		 //成功返回0
		}
 	} 
	return STATUS_ERR;	
}
unsigned char Write_RFID(unsigned char addr,unsigned char * data,unsigned char len)
{ 
    unsigned char status;
	unsigned char i;
    unsigned char cmd[22]={0x03, 0x0B, 0xC5, 0x20};
	cmd[4] =addr;//地址
    cmd[1]=6+len;//长度
	for(i=0;i<len;i++)
	{
		cmd[5+i] = data[i];
	}
	TxCheckSum(cmd,cmd[1]);	//数据校验
	Uart3_Send_Data(cmd,cmd[1]);	 //发送写命令
	Delay(2000000);//等待模块返回数据，大于150MS
 	if(Rx3Flag == 1)
 	{	
		Rx3Flag = 0;
		status = RxCheckSum(Uart3RxBuf,Uart3RxBuf[1]);//对返回的数据进行校验
		if(status != STATUS_OK) //判断校验是否通过
		{
			return STATUS_ERR;
		}
		status = Uart3RxBuf[4];
		if(status != STATUS_OK) //判断校验是否通过
		{
			return STATUS_ERR;
		}
		if((Uart3RxBuf[0] == 0x03)&&(Uart3RxBuf[2] == 0xc5))//判断是否为写块数据返回的数据包
		{        
             for(i=0;i<len;i++)   //将写入的数据打印到串口
		{
			while(USART_GetFlagStatus(USART1,USART_FLAG_TC)==RESET); 
			USART_SendData(USART1,cmd[i+5]+'0'); 
		}
				return STATUS_OK;		 //成功返回0
		}
 	} 
	return STATUS_ERR;			//失败返回1
}
void RFID_task(void)
{   
   // unsigned char data[5]={1,2,3,4,5};
   // Write_RFID(0x01,data,5);
   // Read_RFID(0x01,5);
    
    ReadId();

}
