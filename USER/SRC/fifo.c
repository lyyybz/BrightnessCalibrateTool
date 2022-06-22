
#include "fifo.h"

#if 0
int fifo_init(struct myfifo *pfifo,unsigned char *fifo,int sz)
{
    pfifo->buffer = fifo;
    pfifo->tx = 0;
    pfifo->rx = 0;
    pfifo->buffer_sz = sz;
    return(0);
}

int get_fifo_data_sz(struct myfifo *pfifo)
{
    if (pfifo->rx >= pfifo->tx)
    {
        return (pfifo->rx - pfifo->tx);
    }
    else
    {
        return (pfifo->rx + pfifo->buffer_sz - pfifo->tx);
    }
}
int get_fifo_free_sz(struct myfifo *pfifo)
{
    return( pfifo->buffer_sz - 1 - get_fifo_data_sz(pfifo));
}

int put_byte_to_fifo(struct myfifo *pfifo,unsigned char b)
{

    pfifo->buffer[pfifo->rx++] = b;
    if(pfifo->rx >= pfifo->buffer_sz)
    {
        pfifo->rx = 0;
    }
    return(1);
}
int get_byte_from_fifo(struct myfifo *pfifo,unsigned char *pb)
{
    if(pfifo->rx != pfifo->tx)
    {
        *pb = pfifo->buffer[pfifo->tx++];
        if(pfifo->tx >= pfifo->buffer_sz)
        {
            pfifo->tx = 0;
        }
        return(1);
    }
    return(0);
}
#endif

