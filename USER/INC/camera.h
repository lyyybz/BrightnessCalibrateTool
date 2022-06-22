#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "config.h"
#include "main.h"
#include "bmp_layer.h"

#ifdef __cplusplus
 extern "C" {
#endif

/*
  camera: 图像采集需要两个buff。
  1  dma数据buff
  2  图像缓存buff
  todo: 将dmabuff也用动态内存分配
*/

#define CAMERA_FIFO_SIZE  (9*WIN_DEFAULT_CAPUTE_WIDTH)

#define CAMERA_WAIT_TIME    (0x600000)


#define CAMERA_MAX_TIMEOUT_LINE  (5)

enum 
{
    START_CAP ,
    CAPTURING ,
    CAP_REQUEST_FINISHING ,
    CAP_FINISHED , 
};

struct IT_FIFO
{
    volatile int cap_state;
    volatile int rx,tx,sz;
    volatile unsigned char *buffer;
    volatile int camera_scan_line;
    volatile int next_store_line;
    volatile struct TRECT rect; // 当前缓存的矩形
    volatile int frame_id;
    volatile int timeout_line_cnt;
};

short it_fifo_get_width(void);

void fill_line_to_itfifo(void);
int  get_a_line_from_fifo(int y,unsigned char line[]);
void vsync_interrupt_config(int state);


void camera_init_fifo(void);
void camera_start_capture_rect(int left,int right,int top,int bottom);
void camera_caputre_digit_rect(void);
void camera_frame_begin_callback(void);
void camera_frame_end_callback(void);

extern  volatile unsigned short SRC_Buffer[DMA_BUF_SZ]; 
extern struct IT_FIFO itfifo;

#ifdef __cplusplus
  }
#endif


#endif
