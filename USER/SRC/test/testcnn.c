
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "recognize.h"
#include "cnn.h"
#include "mem_manage.h"

static short *short_img = (short*)(0x20000000 + 0xc00);
void transpose(short *raw,short *out,int w,int h ,int c)
{
  int i,j,k;
  for(k = 0 ; k < c ; k++)
  for( i= 0 ; i < h ; i++)
  for( j=0 ; j < w; j++)
  {
    out[k*w*h+j*h+i] = raw[k*w*h+i*w+j];
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
												}
                    }
                }
             
            *out++=sum ;
			}
	}
}
 
 void avgpool_valid(short *img,int w,int h,short *out,int n_row,int n_col,int subsample  )
{
	int i,sum,j,s0,s1;
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
											 
                    }
                }
             
            *out++=sum/9 ;
			}
	}
}

 void maxpool_same(short *img,int w,int h,short *out,int n_row,int n_col,int subsample  )
{
	int i,sum,j,s0,s1,xc;
	int out_w ; 
	int out_h ;
	int pad_w = n_col/2;
	int pad_h = n_row/2;
	int x,y;
	out_w = w ;
	out_h = h ;
	for( i = 0 ; i <out_h ; i +=subsample)
	{
        for( j = 0 ; j < out_w ; j+=subsample)
			{
					  sum = -0x10000;
            
                for (s0 = 0; s0 < n_row; s0++)
                {
                    for(s1 = 0 ; s1 < n_col; s1++)
                    {
											x = j+s1 - pad_w;
											y = i + s0 - pad_h;
											if( (x < 0) || ( x > out_w) ||(y< 0) ||(y > out_h)) 
												continue;
											
											  if(sum < *(img+xc*w*h+w*(y)+x))
												{
													sum = *(img+xc*w*h+w*(y)+x);
												}
                    }
                }
             
            *out++=sum ;
			}
	}
}
void cnnnxnc_same(short *kernels,short *img,int w,int h,int c,short *out,int shiftbit,int n_row,int n_col,int bias,int subsample  )
{
	int i,sum,j,s0,s1,xc;
	int out_w ; 
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
					  sum = bias;
            for (xc = 0 ; xc < c ; xc++)
            {
                for (s0 = 0; s0 < n_row; s0++)
                {
                    for(s1 = 0 ; s1 < n_col; s1++)
                    {
											x = j+s1 - pad_w;
											y = i + s0 - pad_h;
											if( (x < 0) || ( x > out_w) ||(y< 0) ||(y > out_h)) 
												continue;
											px = (img+xc*w*h+w*(y)+x);
											v = *px;
											pxx = *(kernels+xc*n_row*n_col+s0*n_col+s1);
                        sum += v*pxx ;
                    }
                }
            }
            *out++=sum >> shiftbit;
			}
	}
}
void cnnnxnc_valid(short *kernels,short *img,int w,int h,int c,short *out,int shiftbit,int n_row,int n_col,int bias,int subsample  )
{
	int i,sum,j,s0,s1,xc;
	int out_w = (w-(n_col-1));
	int out_h = (h-(n_row-1));
	int v,pxx;
	short *px;
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
                    }
                }
            }
            *out++=sum >> shiftbit;
			}
	}
}

void cnnnxnc(short *kernels,short *img,int w,int h,int c,short *out,int shiftbit,int n,int bias  )
{
	int i,sum,j,s0,s1,xc;
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
                    }
                }
            }
            *out++=sum >> shiftbit;
		}
	}
} 
void cnn7x7(short *kernels,short *img,int w,int h,int c,short *out,int shiftbit,int bias);
void cnn5x5(short *kernels,short *img,int w,int h,int c,short *out,int shiftbit,int bias  );
void cnn3x3(short *kernels,short *img,int w,int h,int c,short *out,int shiftbit,int bias );
void cnn1x1_x(short *kernels,short *img,int w,int h,int c,short *out,int shiftbit,int bias );
int testcnn1x1(void)
{
    int w= 5,h = 3,c = 5;
    short *p =(short*) &short_img[0];
    short *pres1,*pres2;
    int i;
    int bias;
    short kernels[5*1*1];
    for (i = 0 ; i < w*h*c ; i++)
    {
        p[i] = rand()%1000 - 400;
    }
    for (i = 0 ; i < c*1 ; i++)
    {
        kernels[i] = rand()%100 - 50;
    }
    pres1 = p+w*h*c;
    pres2 = pres1 + w*h;
		bias = rand()% 0x11100;
    cnnnxnc(kernels,p,w,h,c,pres1,10,1,bias);
    cnn1x1_x(kernels,p,w,h,c,pres2,10,bias);
    for (i = 0 ; i <w * h ; i++ )
    {
        if (pres1[i] != pres2[i])
        {
            return(-1);;
        }
    }
   return(0);
}
int testcnn3x3(void)
{
    int w= 20,h = 10,c = 5;
    short *p =(short*) &short_img[0];
    short *pres1,*pres2;
    int i;
    int bias;
    short kernels[5*3*3];
    for (i = 0 ; i < w*h*c ; i++)
    {
        p[i] = rand()%1000 - 400;
    }
    for (i = 0 ; i < c*9 ; i++)
    {
        kernels[i] = rand()%100 - 50;
    }
    pres1 = p+w*h*c;
    pres2 = pres1 + w*h;
		bias = rand()% 0x11100;
    cnnnxnc(kernels,p,w,h,c,pres1,10,3,bias);
    cnn3x3(kernels,p,w,h,c,pres2,10,bias);
    for (i = 0 ; i <(w - 2) * (h - 2) ; i++ )
    {
        if (pres1[i] != pres2[i])
        {
            return(-1);;
        }
    }
    return(0);
}
int testmaxpool3x3_valid(void)
{
    int w= 14,h = 10;
    short *p =(short*) &short_img[0];
    short *pres1,*pres2;
    int i; 
    for (i = 0 ; i < w*h ; i++)
    {
        p[i] = rand()%1000 - 400;
    }
    pres1 = p+w*h;
    pres2 = pres1 + w*h;
    maxpool_valid(p,w,h,pres1,3,3,1);
    pool3x3max_valid(pres2,p,w,h);
    for (i = 0 ; i <(w - (3-1)) * (h - (3-1))  ; i++ )
    {
        if (pres1[i] != pres2[i])
        {
            return(-1);;
        }
    }
    return(0);
}
int testavgpool3x3_valid(void)
{
    int w= 14,h = 10;
    short *p =(short*) &short_img[0];
    short *pres1,*pres2;
    int i; 
    for (i = 0 ; i < w*h ; i++)
    {
        p[i] = rand()%1000 - 400;
    }
    pres1 = p+w*h;
    pres2 = pres1 + w*h;
    avgpool_valid(p,w,h,pres1,3,3,1);
    pool3x3avg_valid(pres2,p,w,h);
    for (i = 0 ; i <(w - (3-1)) * (h - (3-1))  ; i++ )
    {
        if (pres1[i] != pres2[i])
        {
            return(-1);;
        }
    }
    return(0);
}

 void pool3x3max_stride2_valid( short *out , short *img,int w,int h);;
int testmaxpool3x3_stride2_valid(void)
{
    int w=0xf,h = 0x1d;
	  int out_w = (w-1)/2;
	  int out_h = (h-1)/2;
    short *p =(short*) &short_img[0];
    short *pres1,*pres2;
    int i; 
    for (i = 0 ; i < w*h ; i++)
    {
        p[i] = rand()%1000 - 400;
    }
    pres1 = p+w*h;
    pres2 = pres1 + w*h;
    maxpool_valid(p,w,h,pres1,3,3,2);
    pool3x3max_stride2_valid(pres2,p,w,h);
    for (i = 0 ; i <out_w * out_h ; i++ )
    {
        if (pres1[i] != pres2[i])
        {
            return(-1);;
        }
    }
    return(0);
}

int testavgpool3x3_stride2_valid(void)
{
    int w= 11,h = 6;
	  int out_w = (w-1)/2;
	  int out_h = (h-1)/2;
    short *p =(short*) &short_img[0];
    short *pres1,*pres2;
    int i; 
    for (i = 0 ; i < w*h ; i++)
    {
        p[i] = rand()%1000 - 400;
    }
    pres1 = p+w*h;
    pres2 = pres1 + w*h;
    avgpool_valid(p,w,h,pres1,3,3,2);
    pool3x3avg_stride2_valid(pres2,p,w,h);
    for (i = 0 ; i <(w - 1) * (h - 1)/4 ; i++ )
    {
        if (pres1[i] != pres2[i])
        {
            return(-1);;
        }
    }
    return(0);
}

void pool3x3max_stride2_v( short *out , short *img,int w,int h,int out_w,int out_h,int out_w_adder);
void testmaxpool3x3_stride2_same(void)
{
    int w= 14,h = 10;
    short *p =(short*) &short_img[0];
    short *pres1,*pres2;
    int i; 
    for (i = 0 ; i < w*h ; i++)
    {
        p[i] = rand()%1000 - 400;
    }
    pres1 = p+w*h;
    pres2 = pres1 + w*h;
    maxpool_same(p,w,h,pres1,3,3,2);
    pool3x3max_stride2_valid(pres2,p,w,h);
    for (i = 0 ; i <(w - 1) * (h - 1)/4 ; i++ )
    {
        if (pres1[i] != pres2[i])
        {
            break;
        }
    }
    i++;
}
int testcnn3x3_stride2_valid(void)
{
    int w= 14,h = 10,c = 3;
    short *p =(short*) &short_img[0];
	int out_h,out_w;
    short *pres1,*pres2;
    int i;
	int bias;
    short kernels[5*3*3];
    for (i = 0 ; i < w*h*c ; i++)
    {
        p[i] = rand()%1000 - 400;
    }
    for (i = 0 ; i < c*3*3 ; i++)
    {
        kernels[i] = rand()%100 - 50;
    }
 
		
    pres1 = p+w*h*c;
    pres2 = pres1 + w*h;
		bias = rand()% 0x11100;
    cnnnxnc_valid(kernels,p,w,h,c,pres1,10,3,3,bias,2);
    cnn3x3_stride2_valid(kernels,p,w,h,c,pres2,10,bias);
	out_h = (h - 1)/2;
	out_w = (w - 1)/2;
    for (i = 0 ; i < out_h*out_w ; i++ )
    {
        if (pres1[i] != pres2[i])
        {
            return(-1);;
        }
    } 
    return(0);
}
void cnn5x5_stride2_valid(short *kernels,short *img,int w,int h,int c,short *out,int shiftbit ,int bias );
void cnn3x3_stride2_valid(short *kernels,short *img,int w,int h,int c,short *out,int shiftbit ,int bias );
 
		 
int testcnn5x5_stride2_valid(void)
{
    int w= 14,h = 10,c = 3;
    short *p =(short*) &short_img[0];
    short *pres1,*pres2;
    int i;
	int bias;
    short kernels[5*5*5];
    for (i = 0 ; i < w*h*c ; i++)
    {
        p[i] = rand()%1000 - 400;
    }
    for (i = 0 ; i < c*5*5 ; i++)
    {
        kernels[i] = rand()%100 - 50;
    }
    pres1 = p+w*h*c;
    pres2 = pres1 + w*h;
		bias = rand()% 0x11100;
    cnnnxnc_valid(kernels,p,w,h,c,pres1,10,5,5,bias,2);
    cnn5x5_stride2_valid(kernels,p,w,h,c,pres2,10,bias);
    for (i = 0 ; i <(w - (5-1)) * (h - (5-1))/4 ; i++ )
    {
        if (pres1[i] != pres2[i])
        {
            return(-1);;
        }
    }
    return(0);
}

int testcnn5x5(void)
{
    int w= 20,h = 10,c = 5;
    short *p =(short*) &short_img[0];
    short *pres1,*pres2;
    int i;
	int bias;
    short kernels[5*5*5];
    for (i = 0 ; i < w*h*c ; i++)
    {
        p[i] = rand()%1000 - 400;
    }
    for (i = 0 ; i < c*5*5 ; i++)
    {
        kernels[i] = rand()%100 - 50;
    }
    pres1 = p+w*h*c;
    pres2 = pres1 + w*h;
		bias = rand()% 0x11100;
    cnnnxnc(kernels,p,w,h,c,pres1,10,5,bias);
    cnn5x5(kernels,p,w,h,c,pres2,10,bias);
    for (i = 0 ; i <(w - (5-1)) * (h - (5-1)) ; i++ )
    {
        if (pres1[i] != pres2[i])
        {
            return(-1);;
        }
    }
    return(0);
}
void cnn1x7(short *kernels,short *img,int w,int h,int c,short *out,int shiftbit,int bias  );
int testcnn7x7(void)
{
    int w= 20,h = 10,c = 5;
    short *p =(short*) &short_img[0];
    short *pres1,*pres2;
    int i;
	  int bias;
    short kernels[5*7*7];
    for (i = 0 ; i < w*h*c ; i++)
    {
        p[i] = rand()%1000 - 400;
    }
    for (i = 0 ; i < c*7*7 ; i++)
    {
        kernels[i] = rand()%100 - 50;
    }
    pres1 = p+w*h*c;
    pres2 = pres1 + w*h;
		bias = rand()% 0x11100;
    cnnnxnc(kernels,p,w,h,c,pres1,10,7,bias);
    cnn7x7(kernels,p,w,h,c,pres2,10,bias);
    for (i = 0 ; i <(w - (7-1)) * (h - (7-1)) ; i++ )
    {
        if (pres1[i] != pres2[i])
        {
            return(-1);;
        }
    }
    return(0);
}



int testcnn7x1(void)
{
    int w= 12,h = 10,c = 3;
    short *p =(short*) &short_img[0];
    short *pres1,*pres2,*pres3,*pres4;
    int i;
	  int bias;
	  int outw,outh;
    short kernels[5*1*7];
    for (i = 0 ; i < w*h*c ; i++)
    {
        p[i] = rand()%1000 - 400;
    }
    for (i = 0 ; i < c*1*7 ; i++)
    {
        kernels[i] = rand()%100 - 50;
    }
    pres1 = p+w*h*c;
    pres2 = pres1 + w*h;
		pres3 = pres2 + w*h*c;
		pres4 = pres3 + w*h;
		transpose(p,pres2,w,h,c);
		bias = rand()% 0x11100;
		outw = w;
		outh = h - 6;
    cnnnxnc_valid(kernels,p,w,h,c,pres1,10,7,1,bias,1);
    cnn1x7(kernels,pres2,h,w,c,pres3,10,bias);
		transpose(pres3,pres4,outh,outw,1);
    for (i = 0 ; i <(w - (7-1)) * h ; i++ )
    {
        if (pres1[i] != pres4[i])
        {
            return(-1);;
        }
    }
    return(0);
}


int testcnn1x5(void)
{
    int w= 12,h = 10,c = 5;
    short *p =(short*) &short_img[0];
    short *pres1,*pres2;
    int i;
	  int bias;
    short kernels[5*1*7];
    for (i = 0 ; i < w*h*c ; i++)
    {
        p[i] = rand()%1000 - 400;
    }
    for (i = 0 ; i < c*1*7 ; i++)
    {
        kernels[i] = rand()%100 - 50;
    }
    pres1 = p+w*h*c;
    pres2 = pres1 + w*h;
		bias = rand()% 0x11100;
    cnnnxnc_valid(kernels,p,w,h,c,pres1,10,1,5,bias,1);
    cnn1x5(kernels,p,w,h,c,pres2,10,bias);
    for (i = 0 ; i <(w - (5-1)) * h ; i++ )
    {
        if (pres1[i] != pres2[i])
        {
            return(-1);;
        }
    }
    i++;
		return(0);
}

int testcnn3x1_valid(void)
{
    int w= 12,h = 10,c = 5;
    short *p =(short*) &short_img[0];
    short *pres1,*pres2;
    int i;
	  int bias;
    short kernels[5*1*3];
	  
    for (i = 0 ; i < w*h*c ; i++)
    {
        p[i] = rand()%1000 - 400;
    }
    for (i = 0 ; i < c*1*3 ; i++)
    {
        kernels[i] = rand()%100 - 50;
    }
    pres1 = p+w*h*c;
    pres2 = pres1 + w*h;
		bias = rand()% 0x11100;
    cnnnxnc_valid(kernels,p,w,h,c,pres1,10,3,1,bias,1);
    cnn3x1_six(kernels,p,w,h,c,pres2,10,bias);
    for (i = 0 ; i <w * (h - (3-1)) ; i++ )
    {
        if (pres1[i] != pres2[i])
        {
            return(-1);
        }
    }
    i++;
		return(0);
}

void cnn7x1_seven(short *kernels,short *img,int w,int h,int c,short *out,int shiftbit,int bias  );
void cnn7x1_six(short *kernels,short *img,int w,int h,int c,short *out,int shiftbit,int bias  );
int testcnn7x1_valid(void)
{
    int w= 12,h = 10,c = 5;
    short *p =(short*) &short_img[0];
    short *pres1,*pres2;
    int i;
	  int bias;
    short kernels[5*1*7];
	  
    for (i = 0 ; i < w*h*c ; i++)
    {
        p[i] = rand()%1000 - 400;
    }
    for (i = 0 ; i < c*1*7 ; i++)
    {
        kernels[i] = rand()%100 - 50;
    }
    pres1 = p+w*h*c;
    pres2 = pres1 + w*h;
		bias = rand()% 0x11100;
    cnnnxnc_valid(kernels,p,w,h,c,pres1,10,7,1,bias,1);
    cnn7x1_seven(kernels,p,w,h,c,pres2,10,bias);
    for (i = 0 ; i <w * (h - (7-1)) ; i++ )
    {
        if (pres1[i] != pres2[i])
        {
            return(-1);
        }
    }
    i++;
		return(0);
}

short *global_mem ;


int testcnn1x7(void)
{
    int w= 12,h = 10,c = 5;
    
    short *p =(short*) &short_img[0];
    short *pres1,*pres2;
    int i;
	  int bias;
    short kernels[5*1*7];
	  
    for (i = 0 ; i < w*h*c ; i++)
    {
        p[i] = rand()%1000 - 400;
    }
    for (i = 0 ; i < c*1*7 ; i++)
    {
        kernels[i] = rand()%100 - 50;
    }
    pres1 = p+w*h*c;
    pres2 = pres1 + w*h;
		bias = rand()% 0x11100;
    cnnnxnc_valid(kernels,p,w,h,c,pres1,10,1,7,bias,1);
    cnn1x7(kernels,p,w,h,c,pres2,10,bias);
    for (i = 0 ; i <(w - (7-1)) * h ; i++ )
    {
        if (pres1[i] != pres2[i])
        {
            return(-1);
        }
    }
    i++;
		return(0);
}
 
void testcnn(void)
 {
	 int (*func[])(void) = 
	 {testmaxpool3x3_stride2_valid,
		 testavgpool3x3_stride2_valid,
	   testavgpool3x3_valid,
		 
		 
	   testmaxpool3x3_valid,
		 testcnn3x1_valid,
		 testcnn1x1,
		 testcnn7x1_valid,
		 testcnn1x5,testcnn1x7,
		 testcnn3x3,testcnn5x5,	testcnn7x7,	
		 testcnn5x5_stride2_valid,
		 testcnn3x3_stride2_valid,
		 
		 
	 };
	 int i,ret;  
		for( i = 0 ; i < sizeof(func)/4 ; i++)
		{
			ret = func[i]();
			if(ret < 0)
			{
			   while(1);
			}
		
		}
	 while(1);
}
