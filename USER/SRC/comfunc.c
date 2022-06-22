#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "comfunc.h"

#define swap(a, b) do {a ^= b; b ^= a; a ^= b;} while (0)

#if 0
#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *		 where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
    /* User can add his own implementation to report the file name and line number,
      ex: printfD("Wrong parameters value: file %s on line %d\r\n", file, line) */

    /* Infinite loop */
    while (1)
    {
    }
}

#endif
#endif


int only_one_1(int x)
{
    if (0 == x) return(0);
    if ( 0 == ((x-1)&x)) return(1);
    return(0);
}
int count_1bits(int x)
{
    int cnt = 0;
    while (x)
    {
        x &= x-1;
        cnt++;
    }
    return(cnt);
}
int get_last1_pos(unsigned int x)
{
    int ret = 0;
    while ( 0 == (x & 1))
    {
        x >>= 1;
        ret++;
    }
    return(ret);
}

uint8_t checksum (const void *data, int len)
{
    uint8_t cs = 0, *p = (uint8_t*)data;

    while (len-- > 0)
        cs += *p++;
    return cs;
}

void order_reverse(void *buff, int len)
{
    uint8_t *rearp = (uint8_t*)buff + len - 1;
    uint8_t *headp = (uint8_t *)buff;

    if (len < 1)
        return;
    while (headp < rearp)
    {
        swap(*headp, *rearp);
        headp++;
        rearp--;
    }
}
int get_last_bit_seqno(unsigned int x)
{
    int k = 0x00;

    while(0x00 == (x & 0x01))
    {
        k++;
        x >>= 0x01;
    }
    return(k);
}
int is_data_all_bcd(uint8_t data[], int len)
{
    while (len-- > 0)
    {
        if ((data[len] & 0x0F) > 0x09 || (data[len] & 0xF0) > 0x90)
            return 0;
    }
    return 1;
}

int is_data_all_xx(const unsigned char data[], int len, unsigned char x)
{
    int i;
    for ( i = 0x00 ; i < len ; i++)
    {
        if (x != data[i]) break;
    }
    if ( i >= len) return(0x01);
    return(0x00);
}

void memaddnum(uint8_t mem[], int num, int cnt)
{
    while (cnt-- > 0)
    {
        mem[cnt] += num;
    }
}


int mymax(int x,int y)
{
    if (x > y )
    {
        return(x);
    }
    return(y);

}
int mymin(int x,int y)
{
    if (x < y)
    {
        return(x);
    }
    return(y);
}
int myabs(int x)
{
    if (x >= 0)
    {
        return(x);
    }
    else
    {
        return(-x);
    }
}
uint16_t UpdateCRC16(uint16_t crcIn, uint8_t byte)
{
    uint32_t crc = crcIn;
    uint32_t in = byte|0x100;

    do {
        crc <<= 1;
        in <<= 1;
        if (in & 0x100)
            ++crc;
        if (crc & 0x10000)
            crc ^= 0x1021;
    } while (!(in & 0x10000));

    return crc & 0xFFFFu;
}

uint16_t Cal_CRC16(const volatile uint8_t* data, uint32_t size)
{
    uint32_t crc = 0;
    const volatile uint8_t* dataEnd = data+size;

    while (data < dataEnd)
        crc = UpdateCRC16(crc, *data++);
    crc = UpdateCRC16(crc,0);
    crc = UpdateCRC16(crc,0);

    return crc & 0xFFFFu;
}

int is_all_xx(const void *s1, int val, int n)
{
    while (n && *(unsigned char *) s1 == val)
    {
        s1 = (unsigned char *) s1 + 1;
        n--;
    }
    return !n;
}

void memset_my(void *s1, uint8_t value, uint8_t n)
{
    do
    {
        *(char*)s1 = value;
        s1 = (char *)s1 + 1;
    }
    while(--n);
}
#if  0
uint8_t memcmp_my(const void *s1, const void *s2, uint8_t n)
{
    while (n && *(char*)s1 == *(char*)s2)
    {
        s1 = (char*)s1 + 1;
        s2 = (char*)s2 + 1;
        n--;
    }

    if(n > 0)
    {
        return(1);
    }
    return(0);
}

void mymemcpy(void *dst,void *src,uint8_t len)
{
    while(len--)
    {
        *(char *)dst = *(char *)src;
        dst = (char *)dst + 1;
        src = (char *)src + 1;
    }
}
#endif



uint32_t get_le_val(const uint8_t * p, int bytes)
{
    uint32_t ret = 0;

    while (bytes-- > 0)
    {
        ret <<= 8;
        ret |= *(p + bytes);
    }
    return ret;
}
uint32_t get_be_val(const uint8_t * p, int bytes)
{
    uint32_t ret = 0;
    while (bytes-- > 0)
    {
        ret <<= 8;
        ret |= *p++;
    }

    return ret;
}
void put_le_val(uint32_t val, uint8_t * p, int bytes)
{
    while (bytes-- > 0)
    {
        *p++ = val & 0xFF;
        val >>= 8;
    }
}
void put_be_val(uint32_t val, uint8_t * p, int bytes)
{
    while (bytes-- > 0)
    {
        *(p + bytes) = val & 0xFF;
        val >>= 8;
    }
}

