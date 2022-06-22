#ifndef __CNN_H__
#define __CNN_H__
void cnn7x7(short *kernels,short *img,int w,int h,int c,short *out,int shiftbit,int bias);
void cnn5x5(short *kernels,short *img,int w,int h,int c,short *out,int shiftbit,int bias  );
void pool1x5avg_five_valid( short *out , short *img,int w,int h);
void cnn1x5_outpad(short *kernels,short *img,int w,int h,int c,short *out,int shiftbit,int bias,int outpad);
void cnn1x1(short *kernels,short *img,int w,int h,int c,short *out,int shiftbit,int bias ); 
void cnn5x5_stride2_valid(short *kernels,short *img,int w,int h,int c,short *out,int shiftbit ,int bias );
void cnn3x3_stride2_valid(short *kernels,short *img,int w,int h,int c,short *out,int shiftbit ,int bias );
void pool3x3max_stride2_valid( short *out , short *img,int w,int h); 
void pool3x3avg_stride2_valid( short *out , short *img,int w,int h);
void pool3x3avg_valid( short *out , short *img,int w,int h);
void pool3x3max_valid( short *out , short *img,int w,int h);;
void cnn1x7_outpad(short *kernels,short *img,int w,int h,int c,short *out,int shiftbit,int bias,int outpad  );;
void cnn1x5(short *kernels,short *img,int w,int h,int c,short *out,int shiftbit,int bias  );
void transpose(short *raw,short *out,int w,int h,int c );
void pad_matrix(short *img ,short *out,int w,int h , int pad_w,int pad_h,int c);
void pad_matrix_only_vertical_top_bottom(short *img ,short *out,int w,int h ,int pad_top,int pad_bottom,int c);
void pad_matrix_maxpool(short *img ,short *out,int w,int h , int pad_w,int pad_h,int c);
void cnn3x1_seven(short *kernels,short *img,int w,int h,int c,short *out,int shiftbit,int bias  );
void cnn3x1_six(short *kernels,short *img,int w,int h,int c,short *out,int shiftbit,int bias  );
void cnn7x1_seven(short *kernels,short *img,int w,int h,int c,short *out,int shiftbit,int bias  );
void cnn7x1_six(short *kernels,short *img,int w,int h,int c,short *out,int shiftbit,int bias  );
void cnn5x1_six(short *kernels,short *img,int w,int h,int c,short *out,int shiftbit,int bias  );;
void cnn3x3_outpad(short *kernels,short *img,int w,int h,int c,short *out,int shiftbit ,int bias,int outpad);
#endif
