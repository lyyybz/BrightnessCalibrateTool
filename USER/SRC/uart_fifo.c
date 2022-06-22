
#include <stdio.h>
#include <string.h>
#include "uart_fifo.h"
#include "alloter.h"
#include "comfunc.h"
#include "hardware_layer.h"
//#include <stm32f10x.h>


struct _UART_FILE_INFOR uart_file_infor[1];

void uart_chn_init(int chn)
{
    struct _UART_FILE_INFOR *puart_infor;

    puart_infor = &uart_file_infor[chn];

    puart_infor->rx_slot.data_cnt = 0x00;
    puart_infor->rx_slot.data_max = MAX_BUFFER_SZ ;
    puart_infor->rx_slot.tx = INVALID_BLK_NO << BLK_NO_SHIFT;
    puart_infor->rx_slot.rx = INVALID_BLK_NO << BLK_NO_SHIFT;


    puart_infor->tx_slot.data_cnt = 0x00;
    puart_infor->tx_slot.data_max = MAX_BUFFER_SZ;
    puart_infor->tx_slot.tx = INVALID_BLK_NO << BLK_NO_SHIFT;
    puart_infor->tx_slot.rx = INVALID_BLK_NO << BLK_NO_SHIFT;

    puart_infor->txing_no_rx = 1;
    puart_infor->over_time_tick = 2;
    puart_infor->busy_rxing = 0;
}


static void empty_a_chn_slot(struct _CHN_SLOT *pCHN_SLOT)
{
    unsigned char buffer[0x20];
    int k =0;
    k = get_chn_bytes(pCHN_SLOT,buffer,sizeof(buffer));
    while(k > 0x00)
    {
        k = get_chn_bytes(pCHN_SLOT,buffer,sizeof(buffer));
    }

}

//the below two function is used in interrupt ,
int uart_chn_tx_bytes(int chn,unsigned char buffer[],int len)
{
    return(get_chn_bytes(&(uart_file_infor[chn].tx_slot),buffer,len));
}
int uart_chn_rx_bytes(int chn,unsigned char buffer[],int len)
{
    return(put_chn_bytes(&(uart_file_infor[chn].rx_slot),buffer,len));
}
// this is used by user
int write_uart_chn_bytes(struct _CHN_SLOT *pCHN_SLOT,unsigned char buffer[],int len)
{
    return(put_chn_bytes(pCHN_SLOT,buffer,len));
}
int read_uart_chn_bytes(struct _CHN_SLOT *pCHN_SLOT,unsigned char buffer[],int len)
{
    return(get_chn_bytes(pCHN_SLOT,buffer,len));
}


void uart_tx_hook(int chn)
{
    if(uart_file_infor[chn].txing_no_rx  )
    {
        empty_a_chn_slot(&uart_file_infor[chn].rx_slot );
    }
}
void uart_rx_hook(int chn)
{
    if(uart_file_infor[chn].over_time_tick > 0x00)
    {
        uart_file_infor[chn].busy_rxing = uart_file_infor[chn].over_time_tick;

    }
}

void alarm_uart_rx_finish_flag(int chn)
{

}

void uart_tick_hook(int chn)
{
    if(uart_file_infor[chn].busy_rxing  > 0x00)
    {
        uart_file_infor[chn].busy_rxing--;
        if(0x00 == uart_file_infor[chn].busy_rxing)
        {
            empty_a_chn_slot(&uart_file_infor[chn].rx_slot);
        }
    }
}

int sys_uart_get(int chn, unsigned char buffer[],int len)
{
    return(get_chn_bytes(&(uart_file_infor[chn].rx_slot),buffer,len));
}
int sys_uart_read(int chn, unsigned char buffer[],int len)
{
    return(read_uart_chn_bytes(&(uart_file_infor[chn].rx_slot),buffer,len));
}

#if !defined(KEIL)
int sys_uart_write(int chn,unsigned char buffer[],int len)
{
    int result ;
    result = write_uart_chn_bytes(&(uart_file_infor[chn].tx_slot),buffer,len);
    //UART_IT_Ctrl(chn,ENABLE,ENABLE);
    return(result);
}
#endif


int sys_uart_peek_data(int chn,unsigned char buffer[],int len)
{
    return(peek_chn_byte(&(uart_file_infor[chn].rx_slot),buffer,len));
}


int uart_get_received_len(int chn)
{
    return uart_file_infor[chn].rx_slot.data_cnt;
}

void test_uart_send()
{
    unsigned char buff[10] = {0x1,0x4,0x2,0xb,0xaa,0xaa,0xaa,0xaa};
    sys_uart_sync_write(0,buff,sizeof(buff));
}

