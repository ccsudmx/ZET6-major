#ifndef _RFID_H
#define _RFID_H
#define STATUS_OK			0x00
#define STATUS_ERR    0x01

#define UART1_RX_BUF_LEN  255
#define UART3_RX_BUF_LEN 255
extern unsigned char Uart1RxBuf[20];
extern unsigned char Uart1RxDataConut;
extern unsigned char Rx1Flag;
extern unsigned char Uart3RxBuf[UART3_RX_BUF_LEN];
extern unsigned char Uart3RxDataConut ;
extern unsigned char Rx3Flag;


#endif