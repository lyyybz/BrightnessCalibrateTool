
#ifndef _MY_FIFO_H_
#define _MY_FIFO_H_

struct myfifo
{
    unsigned char *buffer;
    int tx;
    int rx;
    int buffer_sz;
};

int fifo_init(struct myfifo *pfifo,unsigned char *fifo,int sz);
int put_byte_to_fifo(struct myfifo *pfifo,unsigned char b);
int get_byte_from_fifo(struct myfifo *pfifo,unsigned char *pb);
int get_fifo_data_sz(struct myfifo *pfifo);
int get_fifo_free_sz(struct myfifo *pfifo);

#endif
