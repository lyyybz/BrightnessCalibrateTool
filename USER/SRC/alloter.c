#include "alloter.h"
#include <string.h>
#include <assert.h>
//#include <stm32f10x.h>
#include "uart_fifo.h"
#include "config.h"
#include "hardware_layer.h"
#include "comfunc.h"

static struct _CHN_POOL_MGR chn_pool_mgr;

void init_chn_pool_mgr(void)
{
    //while((sizeof(chn_pool_mgr.buffer) / BLK_SZ) > 8*sizeof(chn_pool_mgr.free_bitmap));
    chn_pool_mgr.free_bitmap = (1 << (sizeof(chn_pool_mgr.buffer) / BLK_SZ)) -1;
}


unsigned int alloc_a_slot(void)
{
    int k;
    uint8_t bitmap;

    bitmap = chn_pool_mgr.free_bitmap;
    if(0x00 == bitmap)
        return(INVALID_BLK_NO);
    k = get_last_bit_seqno(bitmap);
    bitmap &= (bitmap -1);
    chn_pool_mgr.free_bitmap = bitmap;
    //chn_pool_mgr.buffer[(k << 8)| 0xFF] = 0xFF;
    chn_pool_mgr.buffer[(k << BLK_NO_SHIFT)|(BLK_SZ - 1)] = INVALID_BLK_NO;
    return(k);
}
void free_a_slot(int blk_no)
{
    chn_pool_mgr.free_bitmap |=  1 << blk_no;
}

#if 1
int put_chn_bytes(struct _CHN_SLOT *pCHN_SLOT,unsigned char buffer[],int len)
{
    int k,i = 0x00;

#if OS_CRITICAL_METHOD == 3       /* Allocate storage for CPU status register */
    OS_CPU_SR     cpu_sr = 0;
#endif
    if(len < 0x01)	return(0);
    OS_ENTER_CRITICAL();
    while(len > 0x00)
    {
        if(pCHN_SLOT->data_cnt > pCHN_SLOT->data_max)
        {
            //over limit
            goto put_chn_exit;
        }
        //if(0xFF  == (pCHN_SLOT->rx  >> 8))
        if(INVALID_BLK_NO == (pCHN_SLOT->rx >> BLK_NO_SHIFT))
        {
            //not get any buffer
            //pCHN_SLOT->tx = pCHN_SLOT->rx = alloc_a_slot() << 8;
            pCHN_SLOT->tx = pCHN_SLOT->rx = alloc_a_slot() << BLK_NO_SHIFT;
            //if(0xFF  == pCHN_SLOT->rx >> 8)
            if(INVALID_BLK_NO == (pCHN_SLOT->rx >> BLK_NO_SHIFT))
            {
                //no room
                //pCHN_SLOT->tx = pCHN_SLOT->rx = 0xFF<<8;
                pCHN_SLOT->tx = pCHN_SLOT->rx = INVALID_BLK_NO << BLK_NO_SHIFT;
                pCHN_SLOT->data_cnt = 0x00;

                goto put_chn_exit;
            }
        }
        //if(0xFF == (pCHN_SLOT->rx & 0xFF))
        if((BLK_SZ - 1) == (pCHN_SLOT->rx & (BLK_SZ - 1)))
        {
            //need more room
            chn_pool_mgr.buffer[pCHN_SLOT->rx] = alloc_a_slot();
            if(INVALID_BLK_NO == chn_pool_mgr.buffer[pCHN_SLOT->rx])
            {   //no room
                goto put_chn_exit;
            }
            //pCHN_SLOT->rx = chn_pool_mgr.buffer[pCHN_SLOT->rx] << 0x08;
            pCHN_SLOT->rx = chn_pool_mgr.buffer[pCHN_SLOT->rx] << BLK_NO_SHIFT;
        }

        //some space free
        k = pCHN_SLOT->rx & (BLK_SZ - 1);
        k = (BLK_SZ - 1) - k;
        k = MIN(len,k);
        memcpy(&chn_pool_mgr.buffer[pCHN_SLOT->rx],&buffer[i],k);
        len -= k;
        pCHN_SLOT->rx += k;
        i += k;
    }
put_chn_exit:
    pCHN_SLOT->data_cnt += i;
    OS_EXIT_CRITICAL();
    return(i);
}
#endif
int peek_chn_byte(struct _CHN_SLOT *pCHN_SLOT, unsigned char data[],int len)
{
    uint8_t n = 0;
    struct _CHN_SLOT chn_slot;
#if OS_CRITICAL_METHOD == 3       /* Allocate storage for CPU status register */
    OS_CPU_SR     cpu_sr = 0;
#endif

    OS_ENTER_CRITICAL();
    chn_slot.rx = pCHN_SLOT->rx;
    chn_slot.tx = pCHN_SLOT->tx;
    chn_slot.data_cnt = pCHN_SLOT->data_cnt;
    OS_EXIT_CRITICAL();

    //if(chn_slot.data_cnt > 0)
    {
        while((len > 0) && (chn_slot.tx != chn_slot.rx))
        {
            if((BLK_SZ - 1) == (chn_slot.tx & (BLK_SZ - 1)))
            {
                chn_slot.tx = chn_pool_mgr.buffer[chn_slot.tx] << BLK_NO_SHIFT;
            }
            data[n++] = chn_pool_mgr.buffer[chn_slot.tx];
            chn_slot.tx++;
            len--;
        }
    }

    return(n);
}
/*******************************************************************************
* Function Name  : get_chn_bytes
* Description    :
* Input          : *pCHN_SLOT,buffer[],len
* Output         : None
* Return         : i
*******************************************************************************/
#if 1
int get_chn_bytes(struct _CHN_SLOT *pCHN_SLOT,unsigned char buffer[],int len)
{

    int k,i = 0x00;
#if OS_CRITICAL_METHOD == 3       /* Allocate storage for CPU status register */
    OS_CPU_SR     cpu_sr = 0;
#endif
    if(len < 0x01) return(0);

    OS_ENTER_CRITICAL();
    while(len > 0x00)
    {
        if(0x00000 == pCHN_SLOT->data_cnt)
        {
            //pCHN_SLOT->tx = pCHN_SLOT->rx = 0xFF<< 8;
            pCHN_SLOT->tx = pCHN_SLOT->rx = INVALID_BLK_NO << BLK_NO_SHIFT;
            goto get_chn_exit;
        }
        if((BLK_SZ - 1) == (pCHN_SLOT->tx  & (BLK_SZ - 1)))
        {
            //free_a_slot(pCHN_SLOT->tx >> 0x08);
            free_a_slot(pCHN_SLOT->tx >> BLK_NO_SHIFT);
            if(INVALID_BLK_NO ==chn_pool_mgr.buffer[pCHN_SLOT->tx])
            {
                //pCHN_SLOT->tx = pCHN_SLOT->rx = 0xFF << 8;
                pCHN_SLOT->tx = pCHN_SLOT->rx = INVALID_BLK_NO << BLK_NO_SHIFT;
                if(0x00 != pCHN_SLOT->data_cnt)
                {
                    while(1)
                    {
                        //error
                    }
                }
                pCHN_SLOT->data_cnt = 0x00;
                goto get_chn_exit;
            }
            //pCHN_SLOT->tx = chn_pool_mgr.buffer[pCHN_SLOT->tx] << 0x08;
            pCHN_SLOT->tx = chn_pool_mgr.buffer[pCHN_SLOT->tx] << BLK_NO_SHIFT;
        }

        {
            //some space free
            k = pCHN_SLOT->tx & (BLK_SZ - 1);
            k = (BLK_SZ - 1) - k;
            k = MIN(len,k);
            k = MIN(k,pCHN_SLOT->data_cnt );
            memcpy(&buffer[i],&chn_pool_mgr.buffer[pCHN_SLOT->tx],k);
            len -= k;
            pCHN_SLOT->tx += k;
            i += k;
            pCHN_SLOT->data_cnt -= k;
#if 0
            if((BLK_SZ - 1) == (pCHN_SLOT->tx & (BLK_SZ - 1)))
            {
                //free_a_slot(pCHN_SLOT->tx >> 0x08);
                free_a_slot(pCHN_SLOT->tx >> BLK_NO_SHIFT);
                if(INVALID_BLK_NO == chn_pool_mgr.buffer[pCHN_SLOT->tx])
                {
                    pCHN_SLOT->data_cnt = 0x00;
                }
                else
                {
                    //pCHN_SLOT->tx = chn_pool_mgr.buffer[pCHN_SLOT->tx]<<8;
                    pCHN_SLOT->tx = chn_pool_mgr.buffer[pCHN_SLOT->tx]<<BLK_NO_SHIFT;
                }
            }
#endif
            if(pCHN_SLOT->data_cnt <= 0x00)
            {
                //free_a_slot(pCHN_SLOT->tx >> 0x08);
                free_a_slot(pCHN_SLOT->tx >> BLK_NO_SHIFT);
                //pCHN_SLOT->tx = pCHN_SLOT->rx = 0xFF << 0x08;
                pCHN_SLOT->tx = pCHN_SLOT->rx = INVALID_BLK_NO << BLK_NO_SHIFT;
                goto get_chn_exit;
            }
        }
    }
get_chn_exit:
    OS_EXIT_CRITICAL();
    return(i);
}
#endif
#if 0
int get_chn_byte(struct _CHN_SLOT *pCHN_SLOT,unsigned char buffer[])
{
    uint8_t i = 0;
#if 1
#if OS_CRITICAL_METHOD == 3       /* Allocate storage for CPU status register */
    OS_CPU_SR     cpu_sr = 0;
#endif
    OS_ENTER_CRITICAL();
#endif
    if(0x00000 == pCHN_SLOT->data_cnt)
    {
        //pCHN_SLOT->tx = pCHN_SLOT->rx = 0xFF<< 8;
        pCHN_SLOT->tx = pCHN_SLOT->rx = INVALID_BLK_NO << BLK_NO_SHIFT;
        goto get_chn_exit;
    }
#if 0
    if((BLK_SZ - 1) == (pCHN_SLOT->tx  & (BLK_SZ - 1)))
    {
        //free_a_slot(pCHN_SLOT->tx >> 0x08);
        free_a_slot(pCHN_SLOT->tx >> BLK_NO_SHIFT);
        if(INVALID_BLK_NO ==chn_pool_mgr.buffer[pCHN_SLOT->tx])
        {
            //pCHN_SLOT->tx = pCHN_SLOT->rx = 0xFF << 8;
            pCHN_SLOT->tx = pCHN_SLOT->rx = INVALID_BLK_NO << BLK_NO_SHIFT;
            /* if(0x00 != pCHN_SLOT->data_cnt)
             {
            	 while(1) {
            		 //error
            	 }
             }*/
            pCHN_SLOT->data_cnt = 0x00;
            goto get_chn_exit;
        }
        //pCHN_SLOT->tx = chn_pool_mgr.buffer[pCHN_SLOT->tx] << 0x08;
        pCHN_SLOT->tx = chn_pool_mgr.buffer[pCHN_SLOT->tx] << BLK_NO_SHIFT;
    }
#endif

    {

        /*since in this function len=1,so k and i loop once  */
        buffer[0] = chn_pool_mgr.buffer[pCHN_SLOT->tx];
        pCHN_SLOT->tx++;
        i++;
        pCHN_SLOT->data_cnt--;

        if((BLK_SZ - 1) == (pCHN_SLOT->tx & (BLK_SZ - 1)))
        {
            //free_a_slot(pCHN_SLOT->tx >> 0x08);
            free_a_slot(pCHN_SLOT->tx >> BLK_NO_SHIFT);
            if(INVALID_BLK_NO == chn_pool_mgr.buffer[pCHN_SLOT->tx])
            {
                pCHN_SLOT->data_cnt = 0x00;
            }
            else
            {
                //pCHN_SLOT->tx = chn_pool_mgr.buffer[pCHN_SLOT->tx]<<8;
                pCHN_SLOT->tx = chn_pool_mgr.buffer[pCHN_SLOT->tx]<<BLK_NO_SHIFT;
            }
        }
        if(pCHN_SLOT->data_cnt <= 0x00)
        {
            //free_a_slot(pCHN_SLOT->tx >> 0x08);
            free_a_slot(pCHN_SLOT->tx >> BLK_NO_SHIFT);
            //pCHN_SLOT->tx = pCHN_SLOT->rx = 0xFF << 0x08;
            pCHN_SLOT->tx = pCHN_SLOT->rx = INVALID_BLK_NO << BLK_NO_SHIFT;
            goto get_chn_exit;
        }
    }

get_chn_exit:
#if 1
    OS_EXIT_CRITICAL();
#endif
    return(i);

}
#endif

/******************* (C) COPYRIGHT @ EastSoft ******************END OF FILE****/


