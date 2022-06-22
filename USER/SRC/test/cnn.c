#include "cnn.h"

typedef  int  SUM_TYPE;

void check_range(long long x)
{
    long long m;
    m = 1;
    m <<= 31;
    m--;
    if (x > m )
    {
        while (1)
        {
        }
    }
    m = 1;
    m <<= 31;
    m = 0- m;
    if (x < m)
    {
        while (1)
        {
        }
    }
}
void cnnnxnc(short *kernels,short *img,int w,int h,int c,short *out,int shiftbit,int n,int bias  )
{
	int i,j,s0,s1,xc;
    SUM_TYPE sum; 
	for( i = 0 ; i < h-(n-1) ; i ++)
	{
        for( j = 0 ; j < w-(n-1) ; j++)
		{
					  sum = bias;
            for (xc = 0 ; xc < c ; xc++)
            {
                for (s0 = 0; s0 < n; s0++)
                {
                    for(s1 = 0 ; s1 < n; s1++)
                    {
                        sum += *(img+xc*w*h+w*(s0+i)+j+s1)* *(kernels+xc*n*n+s0*n+s1);
                        check_range(sum);
                    }
                }
            }
            *out++=sum >> shiftbit;
		}
	}
}
void cnnnxnc_valid(short *kernels,short *img,int w,int h,int c,short *out,int shiftbit,int n_row,int n_col,int bias,int subsample  )
{
	int i,j,s0,s1,xc;
	int out_w = (w-(n_col-1));
	int out_h = (h-(n_row-1));
	int v,pxx;
	short *px;
     SUM_TYPE sum; 
	for( i = 0 ; i <out_h ; i +=subsample)
	{
        for( j = 0 ; j < out_w ; j+=subsample)
			{
					  sum = bias;
            for (xc = 0 ; xc < c ; xc++)
            {
                for (s0 = 0; s0 < n_row; s0++)
                {
                    for(s1 = 0 ; s1 < n_col; s1++)
                    {
											px = (img+xc*w*h+w*(s0+i)+j+s1);
											v = *px;
											pxx = *(kernels+xc*n_row*n_col+s0*n_col+s1);
                        sum += v*pxx ;
                        check_range(sum);
                    }
                }
            }
            *out++=sum >> shiftbit;
			}
	}
}

 void avgpool_valid(short *img,int w,int h,short *out,int n_row,int n_col,int subsample  )
{
	int i,j,s0,s1;
     SUM_TYPE sum; 
	int out_w = (w-(n_col-1));
	int out_h = (h-(n_row-1));
	for( i = 0 ; i <out_h ; i +=subsample)
	{
        for( j = 0 ; j < out_w ; j+=subsample)
			{
					  sum = 0;
            
                for (s0 = 0; s0 < n_row; s0++)
                {
                    for(s1 = 0 ; s1 < n_col; s1++)
                    {
											  
													sum += *(img +w*(s0+i)+j+s1);
                                                    check_range(sum);
											 
                    }
                }
             
            *out++=sum/(n_row*n_col) ;
			}
	}
}
 void maxpool_valid(short *img,int w,int h,short *out,int n_row,int n_col,int subsample  )
{
	int i,sum,j,s0,s1;
	int out_w = (w-(n_col-1));
	int out_h = (h-(n_row-1));
	for( i = 0 ; i <out_h ; i +=subsample)
	{
        for( j = 0 ; j < out_w ; j+=subsample)
			{
					  sum = -0x10000;
            
                for (s0 = 0; s0 < n_row; s0++)
                {
                    for(s1 = 0 ; s1 < n_col; s1++)
                    {
											  if(sum < *(img +w*(s0+i)+j+s1))
												{
													sum = *(img +w*(s0+i)+j+s1);
                                                    check_range(sum);
												}
                    }
                }
             
            *out++=sum ;
			}
	}
}

void cnnnxnc_same(short *kernels,short *img,int w,int h,int c,short *out,int shiftbit,int n_row,int n_col,int bias,int subsample  )
{
	int i,j,s0,s1,xc;
	int out_w ; 
     SUM_TYPE sum; 
	int out_h ;
	int v,pxx;
	short *px;
	int pad_w = n_col/2;
	int pad_h = n_row/2;
	int x,y;
	out_w = w ;
	out_h = h ;
	for( i = 0 ; i <out_h ; i +=subsample)
	{
        for( j = 0 ; j < out_w ; j+=subsample)
			{
					  sum = 0;
            for (xc = 0 ; xc < c ; xc++)
            {
                for (s0 = 0; s0 < n_row; s0++)
                {
                    for(s1 = 0 ; s1 < n_col; s1++)
                    {
											x = j+s1 - pad_w;
											y = i + s0 - pad_h;
											if( (x < 0) || ( x >= out_w) ||(y< 0) ||(y >= out_h)) 
												continue;
											px = (img+xc*w*h+w*(y)+x);
											v = *px;
											pxx = *(kernels+xc*n_row*n_col+s0*n_col+s1);
                                            sum += v*pxx ;
                                            check_range(sum);
                    }
                }
            }
						sum +=bias;
            *out++=sum >> shiftbit;
			}
	}
}

void cnn1x1(short *kernels,short *img,int w,int h,int c,short *out,int shiftbit,int bias )
{
    cnnnxnc(kernels,img,w,h,c,out,shiftbit,1,bias);
}
void cnn3x3(short *kernels,short *img,int w,int h,int c,short *out,int shiftbit,int bias )
{
    cnnnxnc_valid(kernels,img,w,h,c,out,shiftbit,3,3,bias,1);
}
void cnn3x3_stride2_valid(short *kernels,short *img,int w,int h,int c,short *out,int shiftbit ,int bias )
{
    cnnnxnc_valid(kernels,img,w,h,c,out,shiftbit,3,3,bias,2);
}
void pool3x3avg_stride2_valid( short *out , short *img,int w,int h)
{
    avgpool_valid(img,w,h,out,3,3,2);
}
void pool3x3avg_valid( short *out , short *img,int w,int h)
{
    avgpool_valid(img,w,h,out,3,3,1);
}

void pool3x3max_stride2_valid( short *out , short *img,int w,int h)
{
    maxpool_valid(img,w,h,out,3,3,2);
}

void cnn3x3_pad1(short *kernels,short *img,int w,int h,int c,short *out,int shiftbit,int bias )
{
    cnnnxnc_same(kernels,img,w,h,c,out,shiftbit,3,3,bias,1);
}
void cnn1x5_pad2(short *kernels,short *img,int w,int h,int c,short *out,int shiftbit,int bias )
{
    cnnnxnc_same(kernels,img,w,h,c,out,shiftbit,1,5,bias,1);
}
void cnn1x7_pad3(short *kernels,short *img,int w,int h,int c,short *out,int shiftbit,int bias )
{
    cnnnxnc_same(kernels,img,w,h,c,out,shiftbit,1,7,bias,1);
}
void cnn5x1_pad2(short *kernels,short *img,int w,int h,int c,short *out,int shiftbit,int bias )
{
    cnnnxnc_same(kernels,img,w,h,c,out,shiftbit,5,1,bias,1);
}
void cnn7x1_pad3(short *kernels,short *img,int w,int h,int c,short *out,int shiftbit,int bias )
{
    cnnnxnc_same(kernels,img,w,h,c,out,shiftbit,7,1,bias,1);
}

