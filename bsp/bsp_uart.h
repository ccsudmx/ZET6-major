#ifndef __BSP_UART_H__
#define __BSP_UART_H__

#include "stm32f10x.h"
#include "stm32f10x_usart.h"

#include "kfifo.h"
#define ARRY_ITEMS_NUM(n) (sizeof(n)/sizeof(n[0]))
void bsp_uart_init(uint8_t uart_num);

#define uart_tx_kfifo  1
#if uart_tx_kfifo

int32_t bsp_uart_send       (uint8_t uart_port,uint8_t* pbuf,int32_t len);
#else
void bsp_uart_send(u8 *buf,u8 len) ;  //发送数据函数，带发送长度

#endif

int32_t bsp_uart_read		(uint8_t uart_port,uint8_t* p_dest,int32_t len);

#endif //__BSP_UART_H__
