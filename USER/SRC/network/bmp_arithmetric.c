#include "bmp_arithmetric.h"
#include "mem_manage.h"
#include <string.h>

typedef unsigned char uchar;

/*
    参考书：图像处理、分析与机器视觉 page87
*/
int  equalize_hist(unsigned char *data,int width,int height,int line_size)
{
    int size = height*width;
    unsigned char *buff = mem_add_extra_size(data,256*2+256*2+256);
    unsigned short *hist =(unsigned short *)buff;
    unsigned short *histAcc = (unsigned short *)(buff + 256*2);
    unsigned char  *histEQU = buff + 256*4;
    unsigned char min = 255;
    unsigned char max = 0;
    unsigned char temp=0;
    int eql_max= 0;
    int eql_min = 0;
    int i,j;

    memset(buff,0,256*2+256*2+256);

    for (i=0; i<height; i++)
    {
        for (j=0; j<width; j++)
        {
            temp = data[i*line_size+j];

            hist[temp]++;
            if(max < temp )
            {
                max = temp;
            }

            if(min > temp)
            {
                min = temp;
            }
        }
    }


    for (i=0; i<256; i++)
    {
        if (0==i)
        {
            histAcc[i] = hist[i];
        }
        else
        {
            histAcc[i] = histAcc[i-1] + hist[i];
        }
    }

    eql_max  = (int)(255.0 * histAcc[max]/(float)(size) + 0.5);
    eql_min = (int)(255.0 * histAcc[min]/(float)(size) + 0.5);

    if(eql_max - eql_min < 1)
    {
        return -1;
    }

    for (i=min; i<256; i++)
    {
        histEQU[i] = (int)(255.0 * histAcc[i]/(float)(size) + 0.5);
        histEQU[i] = (int)(255.0*(float)(histEQU[i]-eql_min)/(float)(eql_max-eql_min) + 0.5);
    }

    for (i=0; i<height; i++)
    {
        for (j=0; j<width; j++)
        {
            temp = data[i*line_size+j];
            data[i*line_size+j] = histEQU[temp];
        }
    }

    return 0;
}
