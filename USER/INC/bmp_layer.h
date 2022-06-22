#ifndef __BMP_LAYER__H__
#define __BMP_LAYER__H__ 


#ifdef __cplusplus
 extern "C" {
#endif

/*
   图像矫正需要额外的图像矫正buff。 
   图像矫正和图像缓存两者的大小根据处理速度仔细调节，匹配大小，才能让图像处理的速度最快.
*/

#include "cache_control.h"
#include "config.h"


#define  BMP_ADJUST_BUFF_SIZE  (11*1024)

struct bmp_layer_type
{
	unsigned char bmp_raw[CAMERA_MAX_LINE_BUFF*8];
	unsigned char rorated_buff[CAMERA_MAX_LINE_BUFF*14];
	unsigned char scaled_buff[DIGIT_WIDTH*MAX_DIGIT_CNT*2];
};




typedef struct bmp_layer_type bmp_layer_t;

/*
    top 
  left  right 
    bottom

  前闭后开区间
*/
struct TRECT
{
     int left,top,right,bottom;
};


struct point_type
{
	short x;
	short y;
};


typedef struct point_type point_t;

/*四边形*/
struct quadrange_type
{
    point_t p00,p01,p10,p11;
};

typedef struct quadrange_type quadrange_t;


struct rorate_context_type
{
	point_t start_point;
    short origin_width;
	short origin_height;
	short out_width;
	short out_height;

	float theta;
	float dx;
	float dy;

};

typedef struct rorate_context_type rorate_context_t;


struct DIGITS_MEM
{
    unsigned char digits[ DIGIT_HEIGHT ][DIGIT_WIDTH*MAX_DIGIT_CNT ];
};


void camera_adjust_buff_alloc(void);
void camera_adjust_buff_free(void);

int init_rorate_context(rorate_context_t *rc, quadrange_t *qr,int width,int height);

/*从摄像头获取数据并存储到digits_mem中*/
int get_digits_from_camera_fast(void);
/*从digits_mem中获取数据*/
void read_sample_from_flash(unsigned char buff[],unsigned char id);
void read_digit_from_flash(unsigned char buff[],unsigned char id);
int  read_digits_line_from_flash(int y,unsigned char dline[],int digit_bitmap);
int  get_line_from_flash(int y,unsigned char buff[]);
int capture_bmp_from_camera(void);
#ifdef __cplusplus
 }
#endif

#endif 

