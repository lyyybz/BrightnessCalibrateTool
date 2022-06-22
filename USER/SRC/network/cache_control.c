#include "cache_control.h"
#include "stdio.h"
#include "stdlib.h"
#include "main.h"
#include "string.h"
#include "math.h"
#include "comfunc.h"

#define cache_line_data(cache,line) (cache->buff + (line)*cache->line_size)

void cache_control_init(
    cache_control_t *cache,
    unsigned char *buff,
    unsigned int buff_size,
    unsigned short line_size,
    int (*get_next_line)(int line,unsigned char buff[],int size))
{
    memset(cache,0,sizeof(cache_control_t));
    cache->buff = buff;
    cache->max_cnt = buff_size/line_size;
    cache->line_size = line_size;
    cache->get_next_line = get_next_line;
}

FORCE_INLINE unsigned char *cache_control_get_line(cache_control_t *cache, int line)
{
    int gap = 0;
    int ret;

    if(line < cache->start_line || line > WIN_DEFAULT_HEIGHT)
    {
        return NULL;
    }

    while(1)
    {
        gap = line - cache->start_line;
        if(gap < cache->cached_cnt)
        {
            int dst_line = (cache->head+gap) % cache->max_cnt;
            return cache->buff + dst_line*cache->line_size;
        }
        else
        {
            if(cache->cached_cnt < cache->max_cnt)
            {
                int read_line = (cache->head+cache->cached_cnt) % cache->max_cnt;
                ret = cache->get_next_line(line
                                           ,cache_line_data(cache,read_line)
                                           ,cache->line_size);
                if(ret<0)
                {
                    return NULL;
                }
                cache->cached_cnt++;
            }
            else
            {
                ret = cache->get_next_line(line
                                           ,cache_line_data(cache,cache->head)
                                           ,cache->line_size);
                if(ret<0)
                {
                    return NULL;
                }
                cache->start_line++;
                cache->head++;
                cache->head = cache->head%cache->max_cnt;
            }
        }
    }
}


unsigned char cache_get_peixl(cache_control_t *cache,int x,int y)
{
    unsigned char *line = NULL;

    line = cache_control_get_line(cache,y);

    if(NULL != line)
    {
        return line[x];
    }
    else
    {
        return 0;
    }
}

unsigned char cache_get_peixl_float(cache_control_t *cache,float x,float y,int width,int height)
{
#if  0
    float radio[4] = {0};
    int x_floor = floor(x);
    int y_floor = floor(y);
    float sum = 0;
    int i,j;
    int index = 0;
    radio[0] = 1 - (x - x_floor);
    radio[1] = 1 - radio[0];
    radio[2] = 1- (y - y_floor);
    radio[3] = 1 - radio[2];

    for(i = 0; i<2; i++)
    {
        for(j = 0; j<2; j++)
        {
            if ((x<0) || (x>= width) || (y<0) || (y>=height))
            {
                continue;
            }
            else
            {
                sum += cache_get_peixl(cache,j+x_floor,i+y_floor)*radio[i*2+j];
                index++;
            }

        }
    }

    if(index)
    {
        return  sum/index;
    }
    else
    {
        return 0;
    }
#else

    if ((x<0) || (x>= width) || (y<0) || (y>=height))
    {
        return 0;
    }
    else
    {
        return cache_get_peixl(cache,(x+0.5),y+0.5);
    }
#endif
}



void cache_control_reset(cache_control_t *cache)
{
    cache->head = 0;
    cache->cached_cnt = 0;
    cache->start_line = 0;
}
