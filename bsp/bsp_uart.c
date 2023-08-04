
#include "bsp_uart.h"
#include "debug_printf.h"
/* config */

#define USING_UART3


/* USART1 */
#define UART1_GPIO_TX        GPIO_Pin_9
#define UART1_GPIO_RX        GPIO_Pin_10
#define UART1_GPIO           GPIOA

/* USART2 */
#define UART2_GPIO_TX        GPIO_Pin_2
#define UART2_GPIO_RX        GPIO_Pin_3
#define UART2_GPIO           GPIOA

/* USART3_REMAP[1:0] = 00 */
#define UART3_GPIO_TX        GPIO_Pin_10
#define UART3_GPIO_RX        GPIO_Pin_11
#define UART3_GPIO           GPIOB

/* USART4 */
#define UART4_GPIO_TX        GPIO_Pin_10
#define UART4_GPIO_RX        GPIO_Pin_11
#define UART4_GPIO           GPIOC

typedef struct _uart_obj
{
    USART_TypeDef*	        uart_port;
    uint32_t                uart_rcc;
  	uint32_t				uart_def_baud;
    IRQn_Type               uart_irq_src;
    uint8_t                 irq_prio;
    
  GPIO_TypeDef*		    uart_pin_port;
	uint32_t                uart_pin_rcc;
	uint16_t				uart_tx_pin;
	uint8_t                 tx_pin_source;
	uint16_t				uart_rx_pin;
	uint8_t                 rx_pin_source;
    
		 kfifo_t				    uart_tx_fifo;
		 kfifo_t				    uart_rx_fifo;
    
    volatile uint8_t        flag_is_in_sending;
		uint8_t 				send_char;
		uint8_t 				recv_char;

}uart_obj_t;

#define ADD_UART_ITEM(uart,rcc_uart,baud,irq_prio,port,tx_pin,rx_pin,tx_size,tx_pbuf,rx_size,rx_pbuf) \
            {uart,RCC_##rcc_uart##Periph_##uart,baud,uart##_IRQn,irq_prio,port,RCC_APB2Periph_##port,GPIO_Pin_##tx_pin,GPIO_PinSource##tx_pin,GPIO_Pin_##rx_pin,GPIO_PinSource##rx_pin,\
            {NULL,tx_size,tx_pbuf,0,0},\
            {NULL,rx_size,rx_pbuf,0,0},\
            0,0,0}
           

#define UART3_TX_SIZE  128
#define UART3_RX_SIZE  128
static uint8_t u3_tx_buf[UART3_TX_SIZE] = {0};        
static uint8_t u3_rx_buf[UART3_RX_SIZE] = {0};        
//static uint8_t u4_tx_buf[UART3_TX_SIZE] = {0};        
//static uint8_t u4_rx_buf[UART3_RX_SIZE] = {0};     

            
uart_obj_t uart_tab[]=
{
     
    ADD_UART_ITEM(USART3,APB1,115200,3,GPIOB,10,11,UART3_TX_SIZE,u3_tx_buf,UART3_RX_SIZE,u3_rx_buf )
    
};

            
#define UART_OBJ uart_tab[uart_num]     

static void RCC_Configuration(void)
{
#if defined(RT_USING_UART1)
    /* Enable UART GPIO clocks */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
    /* Enable UART clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
#endif /* RT_USING_UART1 */

#if defined(RT_USING_UART2)
    /* Enable UART GPIO clocks */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
    /* Enable UART clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
#endif /* RT_USING_UART2 */

#if defined(USING_UART3)
    /* Enable UART GPIO clocks */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
    /* Enable UART clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
#endif /* USING_UART3 */

#if defined(RT_USING_UART4)
    /* Enable UART GPIO clocks */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);
    /* Enable UART clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
#endif /* RT_USING_UART4 */
}

static void GPIO_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;

#if defined(RT_USING_UART1)
    /* Configure USART Rx/tx PIN */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin = UART1_GPIO_RX;
    GPIO_Init(UART1_GPIO, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = UART1_GPIO_TX;
    GPIO_Init(UART1_GPIO, &GPIO_InitStructure);
#endif /* RT_USING_UART1 */

#if defined(RT_USING_UART2)
    /* Configure USART Rx/tx PIN */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin = UART2_GPIO_RX;
    GPIO_Init(UART2_GPIO, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = UART2_GPIO_TX;
    GPIO_Init(UART2_GPIO, &GPIO_InitStructure);
#endif /* RT_USING_UART2 */

#if defined(USING_UART3)
    /* Configure USART Rx/tx PIN */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin = UART3_GPIO_RX;
    GPIO_Init(UART3_GPIO, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = UART3_GPIO_TX;
    GPIO_Init(UART3_GPIO, &GPIO_InitStructure);
#endif /* USING_UART3 */

#if defined(RT_USING_UART4)
    /* Configure USART Rx/tx PIN */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin = UART4_GPIO_RX;
    GPIO_Init(UART4_GPIO, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = UART4_GPIO_TX;
    GPIO_Init(UART4_GPIO, &GPIO_InitStructure);
#endif /* RT_USING_UART4 */
}



void bsp_uart_init(uint8_t uart_num)
{
    GPIO_InitTypeDef                                GPIO_InitStructure;//定义一个GPIO_InitTypeDef类型的结构体
    USART_InitTypeDef                               USART_InitStructure;
    NVIC_InitTypeDef                                NVIC_InitStructure;//定义一个NVIC_InitTypeDef类型的结构体
     if(UART_OBJ.uart_port == USART3)
    {
        RCC_APB2PeriphClockCmd(UART_OBJ.uart_rcc    , ENABLE);
    }
    else
    {
        RCC_APB1PeriphClockCmd(UART_OBJ.uart_rcc    , ENABLE);
    }
    
    RCC_AHBPeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
    GPIO_Init(UART_OBJ.uart_pin_port, &GPIO_InitStructure); 
   if(uart_num>=ARRY_ITEMS_NUM(uart_tab)){DBG_R_E("bsp_uart_init error! uart_num:%d the max:%d",uart_num,ARRY_ITEMS_NUM(uart_tab));}

    GPIO_InitStructure.GPIO_Pin                     = UART_OBJ.uart_rx_pin;//选择要控制的GPIOA引脚
    GPIO_InitStructure.GPIO_Mode                    = GPIO_Mode_IN_FLOATING;
	  GPIO_InitStructure.GPIO_Speed                   = GPIO_Speed_50MHz;
	  GPIO_Init(UART_OBJ.uart_pin_port, &GPIO_InitStructure); 
	
	  GPIO_InitStructure.GPIO_Pin 					=  UART_OBJ.uart_tx_pin;//选择要控制的GPIOA引脚

    GPIO_InitStructure.GPIO_Mode                    = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed                   = GPIO_Speed_50MHz;
	GPIO_Init(UART_OBJ.uart_pin_port, &GPIO_InitStructure); 



   USART_InitStructure.USART_BaudRate              = UART_OBJ.uart_def_baud;
   USART_InitStructure.USART_WordLength            = USART_WordLength_8b;
   USART_InitStructure.USART_StopBits              = USART_StopBits_1;
   USART_InitStructure.USART_Parity                = USART_Parity_No;
   USART_InitStructure.USART_HardwareFlowControl   = USART_HardwareFlowControl_None;
   USART_InitStructure.USART_Mode                  = USART_Mode_Rx | USART_Mode_Tx;

	NVIC_InitStructure.NVIC_IRQChannel              = UART_OBJ.uart_irq_src;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;

	NVIC_InitStructure.NVIC_IRQChannelCmd           = ENABLE;

  
    
    //if(UART_OBJ.uart_port == USART3)
    //{
    //   GPIO_PinAFConfig(UART_OBJ.uart_pin_port, UART_OBJ.tx_pin_source, GPIO_AF_4);  
    //    GPIO_PinAFConfig(UART_OBJ.uart_pin_port, UART_OBJ.rx_pin_source, GPIO_AF_4);  
    //}
    //else
    //{
    //    GPIO_PinAFConfig(UART_OBJ.uart_pin_port, UART_OBJ.tx_pin_source, GPIO_AF_1);  
    //     GPIO_PinAFConfig(UART_OBJ.uart_pin_port, UART_OBJ.rx_pin_source, GPIO_AF_1);  
    //}
    
	NVIC_Init(&NVIC_InitStructure);
    
	USART_Init      (UART_OBJ.uart_port, &USART_InitStructure);
	
	USART_ITConfig  (UART_OBJ.uart_port, USART_IT_RXNE, ENABLE);
	USART_ITConfig  (UART_OBJ.uart_port, USART_IT_PE  , ENABLE); //-----------
	USART_ITConfig  (UART_OBJ.uart_port, USART_IT_ERR , ENABLE); //-------------


    
	USART_Cmd(UART_OBJ.uart_port, ENABLE);
	DBG_B_INFO("uart3 初始化完成");
}

#if uart_tx_kfifo
int32_t bsp_uart_send(uint8_t uart_num,uint8_t* pbuf,int32_t len)
{
    int32_t real_len = 0;
	  if(len==0){return 0;}
    if(uart_num>=ARRY_ITEMS_NUM(uart_tab)){DBG_R_E("bsp_uart_init error! uart_num:%d the max:%d",uart_num,ARRY_ITEMS_NUM(uart_tab));}

    real_len = kfifo_push_in(&UART_OBJ.uart_tx_fifo,pbuf,len);

    if(UART_OBJ.flag_is_in_sending==0){UART_OBJ.flag_is_in_sending=1;}
    
    USART_ITConfig(UART_OBJ.uart_port, USART_IT_TXE, ENABLE);
	return real_len;
}
#else

void bsp_uart_send(u8 *buf,u8 len)   //发送数据函数，带发送长度
{
	u8 t;
	USART_SendData(USART3,1);
  	for(t=0;t<len;t++)		//循环发送数据
	{		   
		while(USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET);	  
		USART_SendData(USART3,buf[t]);
	}	 
 
	while(USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET);		
}



#endif


int32_t bsp_uart_read		(uint8_t uart_num,uint8_t* p_dest,int32_t len	)
{
	int32_t real_len = 0;
  if(uart_num>=ARRY_ITEMS_NUM(uart_tab)){DBG_R_E("bsp_uart_init error! uart_num:%d the max:%d",uart_num,ARRY_ITEMS_NUM(uart_tab));}
	real_len = kfifo_pull_out(&UART_OBJ.uart_rx_fifo,p_dest,len);
	return real_len;
}

void uart_irq_action(uint8_t uart_num)
{
	  DBG_B_INFO("uart_irq_action [%d]",uart_num);

    if( USART_GetITStatus(UART_OBJ.uart_port, USART_IT_TXE) != RESET)
    {
		    DBG_B_INFO("USART_IT_TXE");
        if(kfifo_pull_out(&UART_OBJ.uart_tx_fifo,&UART_OBJ.send_char,1))
        {
            USART_SendData(UART_OBJ.uart_port, UART_OBJ.send_char);				
        }
        else
        {
            UART_OBJ.flag_is_in_sending=0;
            USART_ITConfig(UART_OBJ.uart_port, USART_IT_TXE, DISABLE);
        }
    }
	
	if(USART_GetITStatus(UART_OBJ.uart_port, USART_IT_RXNE) != RESET)
	{
		DBG_B_INFO("USART_IT_RXNE");
		USART_ClearITPendingBit(UART_OBJ.uart_port, USART_IT_RXNE);
		UART_OBJ.recv_char = USART_ReceiveData(UART_OBJ.uart_port);
		kfifo_push_in(&UART_OBJ.uart_rx_fifo,&UART_OBJ.recv_char,1);
	}

	if (USART_GetFlagStatus(UART_OBJ.uart_port, USART_FLAG_ORE) == SET)  
	{  
	    DBG_B_INFO("USART_FLAG_ORE");
	    //USART_ClearITPendingBit(g_uart_init_tab[uart_port].uart_port,USART_IT_ORE);  
//        UART_OBJ.recv_char = USART_ReceiveData(UART_OBJ.uart_port);
//		kfifo_push_in(&UART_OBJ.uart_rx_fifo,&UART_OBJ.recv_char,1);
		USART_ClearFlag(UART_OBJ.uart_port,USART_FLAG_ORE);  //读SR
	}
    
    if(USART_GetFlagStatus(UART_OBJ.uart_port, USART_FLAG_PE) != RESET)
    {//?  @arg USART_IT_PE     : Parity Error interrupt
        DBG_B_INFO("USART_FLAG_PE");
        USART_ClearFlag(UART_OBJ.uart_port, USART_FLAG_PE);
    }
    
    if(USART_GetFlagStatus(UART_OBJ.uart_port, USART_FLAG_FE) != RESET)
    {//?  @arg USART_IT_PE     : Parity Error interrupt
        DBG_B_INFO("USART_FLAG_FE");
        USART_ClearFlag(UART_OBJ.uart_port, USART_FLAG_FE);
    }
    
    if(USART_GetFlagStatus(UART_OBJ.uart_port, USART_FLAG_NE) != RESET)
    {//?  @arg USART_IT_NE     : Noise Error interrupt
        DBG_B_INFO("USART_FLAG_NE");
        USART_ClearFlag(UART_OBJ.uart_port, USART_FLAG_NE);
    }
    
}

void USART2_IRQHandler(void)
{
    //uart_irq_action(1);
}
void USART3_IRQHandler(void)
{
	
	DBG_G_INFO("USART3_IRQHandler");
	uart_irq_action(0);

}



