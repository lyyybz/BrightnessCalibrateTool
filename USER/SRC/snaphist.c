#include <stdio.h>
#include <string.h>
#include "config.h"
#include "snaphist.h"
#include "recognize.h"
#include "camera.h"
#include "recognize.h"
#include "main.h"
#include "device_info.h"

void write_view_to_flash(int pos,int width,int height)
{
    int i=0,len;
    unsigned char dline[DIGIT_WIDTH * (devinfo.digit_count)];

    for ( i = 0 ; i < height ; i++)
    {
        //len = read_digits_line_from_flash(dline,(1 << DIGIT_CNT )-1);
        dataflash_write(pos + i * len, dline,len );
    }
}
int _get_digits_to_flash(void)
{
    int pos;

    // pos = get_free_blk();

    camera_caputre_digit_rect();

    write_view_to_flash(pos, DIGIT_WIDTH * (devinfo.digit_count), DIGIT_HEIGHT);

    return(pos);
}

int _get_digits_null(void)
{
    return 0;
}


int get_digits_from_flash_1(void)
{
    int i,j;
    for(i=0; i < (devinfo.digit_count); i++)
    {
        for(j=0; j<DIGIT_HEIGHT; j++)
        {
            //dataflash_read(j*DIGIT_WIDTH,&digits_mem.digits[j][i*DIGIT_WIDTH],DIGIT_WIDTH);

        }


    }

    return 0;
}



