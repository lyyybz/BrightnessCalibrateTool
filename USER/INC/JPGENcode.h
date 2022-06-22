#ifndef _JENCODE_H
#define _JENCODE_H

#ifdef __cplusplus
 extern "C" {
#endif
int  fake_get_320pixel_per_line(int y,unsigned char buff[]);

int JPEG_encode(int width,
                     int height,
                     int (*get_line)(int y,unsigned char buff[]),
                     int start,
                     int max_size);//编码主函数

#ifdef __cplusplus

}
#endif

#endif
