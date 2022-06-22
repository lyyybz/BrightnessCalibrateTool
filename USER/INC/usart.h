#ifndef __USART_H__
#define __USART_H__
 
 
#define USART_Transmit(n)		USART1_Transmit(n)

//针对林洋转换器，发送空闲数据，防止转换器超时
#if CONFIG_LINYANG_CONVERTET
    #define send_idle_data(date) USART1_Transmit(date)
#else 
    #define send_idle_data(date)
#endif 

void usart_init(int baud_rate,unsigned char is_even); 


int sys_uart_sync_write(int chn,unsigned char buffer[],int len);

void USART1_Transmit(unsigned char data); 

void USART1_RxIntHandler(void);  

void usart1_nvic_config(int state);

 int clac_seq(unsigned short time_seq[], unsigned char data );

 void USART1_Transmit(unsigned char  data);

extern volatile unsigned short uart_time_sequence[12];
extern volatile unsigned short uart_time_stage;
extern  volatile unsigned short uart_time_index;


extern void (*USART_Transmit_Byte)(unsigned char data);

void MCU_USART1_Transmit(unsigned char  data);
void Timer2_Transmit(unsigned char data);

#endif
