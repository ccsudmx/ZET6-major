#ifndef _RFID_H
#define _RFID_H

#define STATUS_OK			0x00
#define STATUS_ERR    0x01

#define UART3_RX_BUF_LEN 30
extern unsigned char Uart3RxBuf[];
extern unsigned char Uart3RxDataConut ;
extern unsigned char Rx3Flag;
unsigned char ReadId(void);
unsigned char Write_RFID(unsigned char addr,unsigned char * data,unsigned char len);
unsigned char Read_RFID(unsigned char addr,unsigned char len);
void RFID_task(void);
#endif

