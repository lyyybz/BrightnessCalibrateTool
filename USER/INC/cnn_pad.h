#ifndef __CNN_PAD_H__
#define __CNN_PAD_H__

#ifdef __cplusplus
 extern "C" {
#endif

void cnn3x3_pad1(short *kernels,short *img,int w,int h,int c,short *out,int shiftbit,int bias );
void cnn1x5_pad2(short *kernels,short *img,int w,int h,int c,short *out,int shiftbit,int bias );
void cnn1x7_pad3(short *kernels,short *img,int w,int h,int c,short *out,int shiftbit,int bias );
void cnn5x1_pad2(short *kernels,short *img,int w,int h,int c,short *out,int shiftbit,int bias );
void cnn7x1_pad3(short *kernels,short *img,int w,int h,int c,short *out,int shiftbit,int bias );
void cnn1x7(short *kernels,short *img,int w,int h,int c,short *out,int shiftbit,int bias  );
void cnn3x3(short *kernels,short *img,int w,int h,int c,short *out,int shiftbit,int bias );

#ifdef __cplusplus
 }
#endif

#endif
