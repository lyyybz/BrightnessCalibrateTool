#include "decode_ebm.h"
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CODE_SHORT   0
#define WEIGHT_BITS  4
#define CODE_MASK   (unsigned short)(~((1 << WEIGHT_BITS)-1))

typedef struct {
    unsigned long  bits;
    unsigned char len;
} bitstring;


#pragma pack(1)
struct dict_node {
    struct dict_node *children[2];
    short val;
};
#pragma pack()

struct storage_info
{
    int pos;
};

static int _myfread(void *buffer,int len,struct storage_info *myfile)
{
    int ret;
    ret = dataflash_read(myfile->pos,buffer,len);
    myfile->pos += len;
    return(ret);
}

static int _myfwrite(unsigned char *buffer,int len,struct storage_info *myfile)
{
    int ret;
    ret = dataflash_write(myfile->pos,buffer,len);
    myfile->pos += len;
    return(ret);
}

struct MYALLOCER {
    int start;
    unsigned char *p;
};

static unsigned char *_mymalloc(struct MYALLOCER *buffer,int sz)
{
    unsigned char *p;
    p = &buffer->p[buffer->start];
    buffer->start += sz;
    return(p);
}
static int _add_leaf(int val,bitstring bits,struct dict_node *root,struct MYALLOCER *allocer )
{
    struct dict_node *p;
    while (--bits.len > 0)
    {
        p = root->children[bits.bits & 0x1];
        if (NULL == p)
        {
            p = (struct dict_node *)_mymalloc(allocer,sizeof(*p));
            memset(p,0,sizeof(*p));
            root->children[bits.bits & 0x1] = p;
        }
        root = p;
        bits.bits >>= 1;
    }
    p = (struct dict_node *)_mymalloc(allocer,sizeof(*p));
    memset(p,0,sizeof(*p));
    p->val = val;
    root->children[bits.bits & 0x1] = p;
    return(0);
}
static int _load_huff_dict_ebm(struct storage_info *in, struct dict_node **root,unsigned char *buffer,int buff_sz)
{
    bitstring bits = { .len = 0 };

    int cnt,node_cnt;
    _myfread(&node_cnt,sizeof(int),in);
    int need_sz = node_cnt * sizeof(struct dict_node);
    if (need_sz > buff_sz)
    {
        return(-1);
    }
    _myfread(&cnt, sizeof(int), in);

    struct MYALLOCER allocer;
    allocer.p = buffer;
    allocer.start = 0;

    *root = (struct dict_node *)_mymalloc(&allocer,sizeof(**root));
    struct dict_node *dict = *root;
    memset(dict,0,sizeof(*dict));

    unsigned char node[4];
    int i;
    for (i = 0 ; i < cnt ; i++)
    {
        _myfread(node,4,in);
        int val = node[0];
        val |= (node[1] & 0xf) << 8;
        bits.len = node[1] >> 4;
        bits.len++;
        bits.bits = node[2] | ( node[3] << 8);
#if 0
        printf("make node[%d]\n", val);
#endif
        _add_leaf(val,bits,dict,&allocer);
    }

    return(0);
}

static int _prepare_space(struct storage_info *out,int sz)
{
    dataflash_erase(out->pos,sz);
    return(0);
}
static int _decode_stream(struct dict_node *dict, struct storage_info *in, struct storage_info *out)
{
    struct dict_node *node = dict;
    bitstring next;
    int chars = 0; // number of characters output
    int bitlen,sz;
    _myfread(&sz,sizeof(int),in);
    _prepare_space(out,sz);

    _myfread(&bitlen,sizeof(int),in);

    unsigned char buffer[0x100];
    int datalen = 0;

    // read only one char at a time to avoid endian issues
    while (bitlen > 0 )
    {
        unsigned char rdbuffer[0x100];
        int rdlen = bitlen ;
        if (rdlen > sizeof(rdbuffer))
        {
            rdlen = sizeof(rdbuffer);
        }
        _myfread(rdbuffer, rdlen, in);
        bitlen -= rdlen;

        int i = 0;
        for (i = 0 ; i < rdlen ; i++)
        {
            next.bits = rdbuffer[i];
            // Pretend we always get a full char's worth of bits. We will stop
            // before end-of-stream because we count characters emitted.
            next.len = CHAR_BIT;
            // consume byte until it is gone
            while (next.len > 0)
            {
                if (node->children[next.bits & 1] == NULL)
                {   // leaf node
                    // fputc(node->val, out); // emit code byte
                    if (node->val & 0x100)
                    {
                        buffer[datalen++] = node->val & 0xff;
                        if (datalen == sizeof(buffer))
                        {
                            _myfwrite(buffer,datalen,out);
                            datalen = 0;
                        }
                        buffer[datalen++] = 0;
                        if (datalen == sizeof(buffer))
                        {
                            _myfwrite(buffer,datalen,out);
                            datalen = 0;
                        }
                        chars+=2;
                    }
                    else if (node->val & 0x200)
                    {
                        buffer[datalen++] = node->val & 0xff;
                        if (datalen == sizeof(buffer))
                        {
                            _myfwrite(buffer,datalen,out);
                            datalen = 0;
                        }
                        buffer[datalen++] = 0xff;
                        if (datalen == sizeof(buffer))
                        {
                            _myfwrite(buffer,datalen,out);
                            datalen = 0;
                        }
                        chars+=2;
                    }
                    else
                    {
#ifndef KEIL
                        if (node->val >= 0x100)
                        {
                            printf("got some err?\n");
                        }
#endif
                        buffer[datalen++] = node->val & 0xff;

                        if (datalen == sizeof(buffer))
                        {
                            _myfwrite(buffer,datalen,out);
                            datalen = 0;
                        }
                        chars++;

                    }
                    node = dict; // reset to root for next bits
                    if (chars >= sz)
                    {
                        goto got_all_lbl;
                    }
                }
                else
                {
                    node = node->children[next.bits & 1];
                    next.bits >>= 1;
                    next.len--; // consumed a bit
                }
            }

        }
    }
got_all_lbl:
    if (datalen > 0)
    {
        _myfwrite(buffer,datalen,out);
        datalen = 0;
    }
    return chars;
}


int huff_decode_ebm(unsigned char *buffer,int buff_sz,int src, int dst)
{
    struct storage_info srcf,dstf;

    srcf.pos = src;
    dstf.pos = dst;

    int ret ;
    struct dict_node *root;
    ret = _load_huff_dict_ebm(&srcf,&root,buffer,buff_sz);
    if (ret)
    {
        goto func_exit_lbl;
    }
    ret =_decode_stream(root, &srcf, &dstf);
func_exit_lbl:
    return(ret);
}

#include "config.h"
#include "comfunc.h"
struct compress_head
{
    char  magic[12];
    unsigned short compress_crc;
    unsigned short para_crc;
    int compress_length;
    int unpack_length;
};

int dataflash_copy(int from, int to, int size)
{
    char buff[200];
    int length;
    dataflash_erase(to,size);
    while(size > 0)
    {
        length = min(size,sizeof(buff));
        dataflash_read(from,buff,length);
        dataflash_write(to,buff,length);
        from += length;
        to += length;
        size -= length;
    }
    return size;
}

#include"mem_manage.h"
#include"update.h"
#include "device_info.h"
#include "hardware_layer.h"

static int _do_load_and_unpack_parameter()
{
    struct compress_head  head;
    memset(&head,0,sizeof(struct compress_head));
    if(device_unpack_is_need(PARAMETER_COPY))
    {
        dataflash_read(CNN_VERSION_ADDR,&head,sizeof(struct compress_head));
        if(0 != strcmp(head.magic,"compress"))
        {
            return -3;
        }
        dataflash_copy(CNN_VERSION_ADDR, BACK_UP_ADDR, head.compress_length+sizeof(struct compress_head));
        if(head.compress_crc !=  dataflash_calc_crc16(BACK_UP_ADDR+sizeof(struct compress_head), head.compress_length))
        {
            return -1;
        }
        device_set_unpack_magic(PARAMETER_COPY);
    }

    if(device_unpack_is_need(PARAMETER_UNPACK))
    {
        unsigned char *temp=NULL;
        dataflash_read(BACK_UP_ADDR,&head,sizeof(struct compress_head));
        if(0 != strcmp(head.magic,"compress"))
        {
            return -3;
        }
        temp = mem_alloc(1024*8);
        huff_decode_ebm(temp, 1024*8, BACK_UP_ADDR+sizeof(struct compress_head), CNN_VERSION_ADDR);
        if(head.para_crc != dataflash_calc_crc16(CNN_VERSION_ADDR,head.unpack_length))
        {
            mem_free(temp);
            return -2;
        }
        device_set_unpack_magic(PARAMETER_UNPACK);
        mem_free(temp);
    }

    return 0;

}

int load_and_unpack_parameter()
{
    int ret=0;

    if(device_unpack_is_need(PARAMETER_UPDATE))
    {
        hardware_init_high_power();
        ret = _do_load_and_unpack_parameter();
        hardware_init_low_power();
    }

    return ret;
}
