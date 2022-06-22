#ifndef __PROTO_CAMERA_H__ 
#define __PROTO_CAMERA_H__ 

#ifdef __cplusplus
 extern "C" {
#endif
#include "device_info.h"

/*发送图像的标准协议
  数据传送都为小段模式。
  协议头(1)  cmd(1)  长度(4)  data  cs(1) 协议尾(1) 
  0x02                                  0x03    
  cs 是cmd  长度 和 data的校验值
*/

enum
{
  /*width ,height,data*/
  IMG_CMD_RAW_BMP,
  IMG_CMD_DIGIT_BMP,

  /*data: JPEG文件*/
  IMG_CMD_RAW_JEPG, 
  IMG_CMD_DIGIT_JEPG,

  /*width height address batch steps light*/
  IMG_CMD_DIGIT_SAMPLE,

  IMG_CMD_COLOR_565,
  IMG_CMD_TRAIN_RAW_BMP,
};

#define PROTOL_IMG_HEAD  0x02
#define PROTOL_IMG_TAIL  0x03 

void send_bmp_in_ram(int width,int height,unsigned char *buff);

void send_bmp_in_fifo(int cmd,int start,int len);
void send_jpeg_in_flash(int cmd,int start,int len);
void send_bmp_in_flash(int cmd,int start,int height,int width);

void send_digit_sample_in_flash(int cmd,int start,struct DEV_INFO *dev);


int capture_jpeg_to_flash(int left,int right,int top,int bottom);
void capture_bmp_to_flash(int left,int right,int top,int bottom);
double capture_bmp_to_calculate_brightness(void);

#ifdef __cplusplus

}

#endif 

#endif
