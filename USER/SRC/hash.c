#include "hash.h"
#include "hardware_layer.h"
#include "comfunc.h"
#include "device_info.h"

#define M  0xffffffff

static unsigned int RSHash(unsigned char*str,int len)
{
    unsigned int b=312551 ;
    unsigned int a=6309 ;
    unsigned int hash=0 ;

    while(len>0)
    {
        hash=hash*a+(*str++);
        a*=b ;
        len--;
    }

    return(hash % M);
}

unsigned int JSHash(unsigned char*str,int len)
{
    unsigned int hash=13423911 ;

    while(len >0)
    {
        hash^=((hash<<5)+(*str++)+(hash>>2));
        len--;
    }

    return(hash % M);
}

unsigned int ELFHash(unsigned char*str,int len)
{
    unsigned int hash=0 ;
    unsigned int x=0 ;

    while(len > 0)
    {
        hash=(hash<<4)+(*str++);
        if((x=hash&0xF0000000L)!=0)
        {
            hash^=(x>>24);
            hash&=~x ;
        }
        len--;
    }

    return(hash % M);
}

unsigned int BKDRHash(unsigned char*str,int len)
{
    unsigned int seed=131131313131 ;// 31 131 1313 13131 131313 etc..
    unsigned int hash=0 ;

    while(len > 0)
    {
        hash=hash*seed+(*str++);
        len--;
    }

    return(hash % M);
}

unsigned int multi_hash(unsigned char *data, int len)
{
    int hash[3] = {0};
    hash[0] = RSHash(data,len);
    hash[1] = JSHash(data,len);
    hash[2] = ELFHash(data,len);
    return BKDRHash((unsigned char *)hash,sizeof(hash));
}

int is_device_from_eastsoft()
{
#if defined(KEIL)
    uint32_t mcu_id[3];
    uint8_t buffer[12];
    unsigned int calc_mcu_hash;
    int i;

    Get_ChipID(mcu_id);

    for(i = 0; i < 3; i++)
    {
        put_be_val(mcu_id[i], &buffer[i*4], 4);
    }
    calc_mcu_hash = multi_hash(buffer,sizeof(buffer));
    return calc_mcu_hash== devinfo.mcu_id_hash;
#else

    return 1;
#endif
}

