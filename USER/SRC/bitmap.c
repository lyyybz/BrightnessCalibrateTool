#include "bitmap.h"
#include "string.h"
#include "comfunc.h"

signed char bitmap_set(unsigned char bitmap[], int max_count, unsigned char number[], unsigned char count, unsigned char value)
{
    int i=0;
    int pos=0;
    int bit_pos=0;

    for(i=0; i < count; i++)
    {
        if(number[i] >= max_count)
            return -1;
    }

    for(i=0; i < count; i++)
    {
        pos = number[i]/8;
        bit_pos = number[i]%8;
        if(value)
        {
            set_bit(bitmap[pos],bit_pos);
        }
        else
        {
            reset_bit(bitmap[pos],bit_pos);
        }

    }
    return 0;
}

unsigned char bitmap_is_bit_clear(unsigned char bitmap[],int max_count, unsigned char number)
{
    int pos;
    int bit_pos;
    if(number >= max_count)
    {
        return 1;
    }
    pos = number/8;
    bit_pos = number%8;
    return  !is_bit_set(bitmap[pos],bit_pos);
}

signed char bitmap_get_clear_bits(unsigned char bitmap[], int max_count, unsigned char number[], unsigned char number_count)
{
    int i=0;
    int mask_cnt=0;
    for(i=0; i < max_count; i++)
    {
        if(bitmap_is_bit_clear(bitmap,max_count,i))
        {
            number[mask_cnt] = i;
            mask_cnt++;
        }

        if(mask_cnt >= number_count)
            break;
    }

    return mask_cnt;
}
