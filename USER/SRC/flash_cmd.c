#include "cmd.h"
#include <stdio.h>
#include <string.h>
#include "Comfunc.h"
#include "stdlib.h"
#include "dbug.h"

#if CONFIG_DEBUG
void  format_output(char *s,char *pch, int len)
{
    int i;

    if(0 != s)
        printf(s);

    for ( i = 0 ; i < len ; i++ )
    {
        if((0 == (i&0xf))
                && (0 != i)  )
        {
            printf("\n");
        }

        printf(" %02x",pch[i]);
    }

    printf("\n");
}


uint32_t get_value (char *s, int *success, int base)
{
    uint32_t value;
    char *p;

    value = strtoul(s,&p,base);
    if ((value == 0) && (p == s))
    {
        *success = 0;
        return 0;
    }
    else
    {
        *success = 1;
        return value;
    }
}


int  erase_flash(int argc, char *argv[])
{
    return 0;
}

static int  __flash_dump(int addr, int len)
{
    char  buf[0x100];
    int  size;
    printf("dump from 0x%08x to 0x%08x ...\n", addr, addr+len);

    while (len > 0)
    {
        size = min(len,0x100);
        dataflash_read(addr,(unsigned char  *)buf,size);
        format_output(NULL,buf, size);
        addr += size;
        len -= size;
    }
    printf("\n");

    return 0;
}

int  flash_dump(int argc, char *argv[])
{
    int  success;
    int len,addr;


    if(3 != argc)
    {
        printf("too few arguments\n");
        return 0 ;
    }

    addr = get_value(argv[1],&success,16);

    if(0==success)
    {
        printf("illegal arguments\n");
        return 0 ;
    }

    len =  get_value(argv[2],&success,16);

    if(0==success)
    {
        printf("illegal arguments\n");
        return 0 ;
    }
    __flash_dump(addr,len);

    return 1;
}


#endif
