#include "camera.h"
#include "device_info.h"
#include "cache_control.h"
#include "bmp_layer.h"
#include "recognize.h"
#include "stdlib.h"
#include "comfunc.h"
#include "main.h"
#include <math.h>
#include "proto_camera.h"
#include "hardware_layer.h"
#include <string.h>
#include "mem_manage.h"
#include "config.h"

cache_control_t  adjust_bmp_cache;

point_t rorate_get_map_point(float theta,point_t orgin,int orgin_width,int orgin_height,int new_width,int new_height)
{
    float x0,y0;
    point_t new_point;
    x0 = orgin.x - orgin_width/2.0;
    y0 = -orgin.y + orgin_height/2.0;

    new_point.x = (int)(x0*cos(theta) + y0*sin(theta)+new_width/2 + 0.5);
    new_point.y = (int)(new_height/2 -(-x0*sin(theta) + y0*cos(theta)) +0.5);

    return new_point;
}


int init_rorate_context(rorate_context_t *rc, quadrange_t *quad,int width,int height)
{
    float angle_value = ((float)(quad->p00.y - quad->p01.y))/(quad->p00.x - quad->p01.x);
    float theta = -atan(angle_value);

    int oldWidth = width ;
    int oldHeight = height;

    // 源图四个角的坐标（以图像中心为坐标系原点）
    float fSrcX1,fSrcY1,fSrcX2,fSrcY2,fSrcX3,fSrcY3,fSrcX4,fSrcY4;
    // 旋转后四个角的坐标（以图像中心为坐标系原点）
    float fDstX1,fDstY1,fDstX2,fDstY2,fDstX3,fDstY3,fDstX4,fDstY4;

    /*原图像映射的宽度和长度*/
    int map_width;
    int map_height;
    point_t old_start;

    rc->origin_width = width;
    rc->origin_height = height;

    fSrcX1 = (float) (- (oldWidth ) / 2);
    fSrcY1 = (float) (  (oldHeight) / 2);
    fSrcX2 = (float) (  (oldWidth ) / 2);
    fSrcY2 = (float) (  (oldHeight) / 2);
    fSrcX3 = (float) (- (oldWidth ) / 2);
    fSrcY3 = (float) (- (oldHeight) / 2);
    fSrcX4 = (float) (  (oldWidth  ) / 2);
    fSrcY4 = (float) (- (oldHeight ) / 2);


    fDstX1 =  cos(theta) * fSrcX1 + sin(theta) * fSrcY1+0.5;
    fDstY1 = -sin(theta) * fSrcX1 + cos(theta) * fSrcY1+0.5;
    fDstX2 =  cos(theta) * fSrcX2 + sin(theta) * fSrcY2+0.5;
    fDstY2 = -sin(theta) * fSrcX2 + cos(theta) * fSrcY2+0.5;
    fDstX3 =  cos(theta) * fSrcX3 + sin(theta) * fSrcY3+0.5;
    fDstY3 = -sin(theta) * fSrcX3 + cos(theta) * fSrcY3+0.5;
    fDstX4 =  cos(theta) * fSrcX4 + sin(theta) * fSrcY4+0.5;
    fDstY4 = -sin(theta) * fSrcX4 + cos(theta) * fSrcY4+0.5;


    map_width  = (int)( max( fabs(fDstX4 - fDstX1), fabs(fDstX3 - fDstX2) ) + 0.5);
    map_width = (int)(4 -(map_width%4))+ map_width;
    map_height = (int)( max( fabs(fDstY4 - fDstY1), fabs(fDstY3 - fDstY2) )  + 0.5);

    rc->dx = -0.5*map_width*cos(theta) - 0.5*map_height*sin(theta) + 0.5*oldWidth;
    rc->dy = 0.5*map_width*sin(theta) - 0.5*map_height*cos(theta) + 0.5*oldHeight;

    /*截取矩形的起点、宽度和长度*/

    old_start.x = quad->p00.x - min(quad->p00.x, quad->p10.x);
    old_start.y = quad->p00.y - min(quad->p00.y, quad->p01.y);
    rc->start_point = rorate_get_map_point(theta,
                                           old_start,
                                           oldWidth,
                                           oldHeight,
                                           map_width,
                                           map_height);
    rc->out_width = fabs((float)(quad->p00.x - quad->p01.x)/cos(theta))+0.5;
    rc->out_height = fabs((float)(quad->p00.y - quad->p10.y)/cos(theta))+0.5;
    rc->theta = theta;

    return 0;
}


static  int  _rotate_cache_get_next_line(int line,unsigned char buff[],int size)
{
    return get_a_line_from_fifo(line,buff);
}


int  get_line_from_flash(int y,unsigned char buff[])
{
    dataflash_read(RECGONIZE_IMAGE_ADDR+y*DIGIT_WIDTH*(devinfo.digit_count),
                   buff,
                   DIGIT_WIDTH*(devinfo.digit_count));
    return DIGIT_WIDTH*(devinfo.digit_count);
}

/*
  图像变形总共分为三个过程:

  原图像(src) - 旋转、缩放 -> 目标图像(dst) - 裁剪、抠图-> 识别的数字图像()
*/

#define FLOAT_FATOR   (4096.0)
#define FLOAT_MOVE_LENGTH  (12)
#define POS_FATOR   (256)
#define POS_MOVE_LENGTH  (8)

#ifdef ESSN_WCR_P
struct pointer_info_t
{
    short x_src_start;
    short y_src_start;
	short y_src;
    short y_dst;
    unsigned char hit;

};

static void _pointer_info_update_hit(struct pointer_info_t *info,int y_src, int count)
{
    int i =0;
    for(i=0; i < count; i++)
    {
        if(info[i].y_src == y_src)
        {
            info[i].hit = 1;
        }
        else
        {
            info[i].hit = 0;
        }
    }
}

static int _point_info_get_next_y_src(struct pointer_info_t *info, int count, int radio)
{
    int i =0;
    int y_next = INT_MAX;
    int complete = 1;
    for(i=0; i < count; i++)
    {
        if(info[i].hit && (info[i].y_dst <= DIGIT_HEIGHT))
        {
            info[i].y_dst++;
            info[i].y_src = ((info[i].y_dst * radio)>>POS_MOVE_LENGTH) + info[i].y_src_start;
            info[i].hit = 0;
        }
    }

    for(i=0; i < count; i++)
    {
        if(info[i].y_dst < DIGIT_HEIGHT)
        {
            complete = 0;
			break;
        }
    }

    if(complete)
    {
        return INT_MIN;
    }

    for(i=0; i < count; i++)
    {
        if((info[i].y_dst < DIGIT_HEIGHT))
            y_next = min(info[i].y_src, y_next);
    }

    return y_next;
}


static int _pointer_info_init(struct pointer_info_t *info,int count,int radio)
{
    int i =0;
    for(i=0; i < count; i++)
    {
        info[i].y_dst = 0;
        info[i].y_src_start = devinfo.pi.center[i].y - (devinfo.pi.pointer_width+1)/2 ;
		info[i].y_src = info[i].y_src_start;
        info[i].hit = 0;
        info[i].x_src_start = devinfo.pi.center[i].x - (devinfo.pi.pointer_width+1)/2;
    }

    return _point_info_get_next_y_src(info, count, radio);
}


int get_digits_from_camera_fast(void)
{
    int w=0,i=0;
    int digit_width = devinfo.pi.pointer_width;
    int radio = (int)((float)(digit_width)/(float)(DIGIT_WIDTH)*POS_FATOR);
    unsigned char *src_line = NULL;
    unsigned char digit_line[MAX_POINTER_COUNT][DIGIT_WIDTH] = {{0},};
    struct pointer_info_t infos[MAX_POINTER_COUNT];
    int flash_addr = RECGONIZE_IMAGE_ADDR;
    int y_src_next = 0;
	int x_src_next = 0;
	
	camera_caputre_digit_rect();//init_cmos  + LED 
	camera_adjust_buff_alloc();
	dataflash_erase(RECGONIZE_IMAGE_ADDR,RECGONIZE_IMAGE_SIZE);//擦flash

    y_src_next = _pointer_info_init(infos, devinfo.digit_count,radio);

    while(y_src_next != INT_MIN)
    {
        _pointer_info_update_hit(infos, y_src_next, devinfo.digit_count);
        src_line = cache_control_get_line(&adjust_bmp_cache,y_src_next);
        for (i = 0; i < devinfo.digit_count; ++i)
        {
            if(infos[i].hit)
            {
				if(y_src_next >= CMAERA_MAX_HEGITH || NULL == src_line|| y_src_next<0)
				{
					memset(digit_line[i], 0, DIGIT_WIDTH); 
				}
				else
				{
	                for (w=0; w < DIGIT_WIDTH; w++)
	                {
	                    x_src_next = infos[i].x_src_start+ ((w*radio)>>POS_MOVE_LENGTH);
	                	if((x_src_next>= CMAERA_MAX_WIDTH) || (x_src_next<0))
						{
							digit_line[i][w] = 0;
							continue;
						}
	                    if(devinfo.color_reverse)
	                        digit_line[i][w] = 0xff - src_line[x_src_next];
	                    else          
							digit_line[i][w] = src_line[x_src_next];
	                }
				}
                flash_addr = RECGONIZE_IMAGE_ADDR + DIGIT_WIDTH*(devinfo.digit_count*infos[i].y_dst + i);
                dataflash_write(flash_addr, digit_line[i], DIGIT_WIDTH);
            }
        }
        y_src_next = _point_info_get_next_y_src(infos, devinfo.digit_count, radio);
    }

    camera_frame_end_callback();
    camera_adjust_buff_free();
    return(0);
}

#else
int get_digits_from_camera_fast(void)
{
    int h=0,id=0,w=0,i=0;
    rorate_context_t *rc = &devinfo.rorate_context;
    float theta = rc->theta;
    int src_height = devinfo.rorate_context.out_height;

    /*for循环中用到的变量*/
    int sin_theta = (int)(sin(theta)*FLOAT_FATOR);
    int cos_theta =(int)(cos(theta)*FLOAT_FATOR);
    int dx = (int)((rc->dx + 0.5)*FLOAT_FATOR);
    int dy = (int)((rc->dy + 0.5)*FLOAT_FATOR);

    int digit_delta = (int)((float)rc->out_width/(float)(devinfo.digit_count) + 0.5)*POS_FATOR ;
    int start_x = devinfo.rorate_context.start_point.x *POS_FATOR;
    int start_y = devinfo.rorate_context.start_point.y *POS_FATOR;
    int offset[MAX_DIGIT_CNT]= {0};
    int h_radio = (int)((float)(src_height)/(float)(DIGIT_HEIGHT)*POS_FATOR);
    int digit_width = device_get_digit_width();
    int w_radio = (int)((float)(digit_width)/(float)(DIGIT_WIDTH)*POS_FATOR);//4:0x15 5:0x20(default) 6:0x3B
    unsigned char *src_line = NULL;
    int dst_x,dst_y;/*目标图像的位置*/
    int x,y; /*在原图的的位置*/
    unsigned char digit_line[MAX_DIGIT_CNT*DIGIT_WIDTH]= {0};
    unsigned char *current_result = digit_line;
    int flash_addr =RECGONIZE_IMAGE_ADDR;

    for(i=0; i < (devinfo.digit_count); i++)
    {
        offset[i]= devinfo.di.digit_offset[i]*POS_FATOR;
    }

    /*
      这个三重for循环是为执行速度而专门优化的，没有特殊原因，不要修改。
      优化的思路如下:
      1 只计算数字识别需要的像素
      2 统一在函数入口出声明变量，不要在for循环里面声明变量，声明变量也需要执行代码
      3 对于常量和四舍五入的计算，放在for循环的外部，不要在for循环里面重复计算
      4 for循环内部尽量减少的函数层级的调用，尽量将代码直接编写在for循环内部
      5 将浮点数操作变为整数操作，整数操作比浮点数操作快100倍
        int 共有32位，1位符号位，31位数据。
        sin cos 扩大 2^12倍，本身占用1位,共需要占中13位
        坐标: 扩大8倍,本身占用10位,共需要占用18位。
        14+18 = 31 所以不会溢出
      6 擦除一个sector需要40ms，所以需要将擦除sector做成任务，尽量在空闲的时候擦除Flash
      7 可以考虑使用DMA写入Flash
    */
	camera_caputre_digit_rect();//init_cmos  + LED 
	camera_adjust_buff_alloc();
	dataflash_erase(RECGONIZE_IMAGE_ADDR,RECGONIZE_IMAGE_SIZE);//擦flash
    /*旋转矫正*/
    for ( h = 0; h < DIGIT_HEIGHT; h++)
    {
        dst_y = h*h_radio + start_y;
        current_result = digit_line;
#ifdef CONFIG_LINYANG_CONVERTET
		if((h%(DIGIT_HEIGHT/3))==0)
		{
			 send_idle_data(0xFE);
		}
#endif 
        for(id =0; id  < (devinfo.digit_count); id++)
        {	
            for (w=0; w < DIGIT_WIDTH; w++)
            {
                dst_x = digit_delta*id + w*w_radio + start_x + offset[id];
                x = (((dst_x*cos_theta) + (dst_y*sin_theta))>>POS_MOVE_LENGTH) + dx;
                x >>=FLOAT_MOVE_LENGTH;
                y = (((-dst_x*sin_theta) + (dst_y*cos_theta))>>POS_MOVE_LENGTH) + dy;
                y >>=FLOAT_MOVE_LENGTH;

                if ((x<0) || (x>= rc->origin_width) || (y<0) || (y>= rc->origin_height))
                {
                    *current_result++=0;
                }
                else
                {
                    src_line = cache_control_get_line(&adjust_bmp_cache,y);

                    if(NULL != src_line)
                    {
                        if(devinfo.color_reverse)
                            *current_result++ = 0xFF - src_line[x];
                        else
                            *current_result++ = src_line[x];

                    }
                    else
                    {
                        if(devinfo.color_reverse)
                            *current_result++ = 0xFF;
                        else
                            *current_result++ = 0;
                    }
                }
            }
        }
        dataflash_write(flash_addr,digit_line,DIGIT_WIDTH*(devinfo.digit_count));
        flash_addr += DIGIT_WIDTH*(devinfo.digit_count);
    }

    camera_frame_end_callback();
    camera_adjust_buff_free();
    return(0);
}
#endif

void camera_adjust_buff_alloc()
{
    unsigned char *buff =  mem_alloc(BMP_ADJUST_BUFF_SIZE);
    cache_control_init(&adjust_bmp_cache
                       ,buff
                       ,BMP_ADJUST_BUFF_SIZE
                       ,devinfo.digital_rect.right - devinfo.digital_rect.left
                       ,_rotate_cache_get_next_line);

}

void camera_adjust_buff_free()
{
    mem_free(adjust_bmp_cache.buff);
}


void read_sample_from_flash(unsigned char buff[],unsigned char id)
{
    int j;

    for(j=0; j<DIGIT_HEIGHT; j++)
    {
        dataflash_read(TEST_IMG_ADDR+j*DIGIT_WIDTH,buff+j*DIGIT_WIDTH,DIGIT_WIDTH);
    }
}


void read_digit_from_flash(unsigned char buff[],unsigned char id)
{
    int j;

    for(j=0; j< DIGIT_HEIGHT; j++)
    {
        dataflash_read(RECGONIZE_IMAGE_ADDR+j*DIGIT_WIDTH*(devinfo.digit_count)+id*DIGIT_WIDTH,
                       buff+j*DIGIT_WIDTH,
                       DIGIT_WIDTH);
    }

}


int  read_digits_line_from_flash(int y,unsigned char dline[],int digit_bitmap)
{
    dataflash_read(RECGONIZE_IMAGE_ADDR+y*DIGIT_WIDTH*(devinfo.digit_count),
                   dline,
                   DIGIT_WIDTH*(devinfo.digit_count));
    return 0;

}

