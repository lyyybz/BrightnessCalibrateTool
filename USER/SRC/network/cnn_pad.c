
#include "cnn_pad.h"
#include "cnn.h"


int cnn3x3_2x2(short *kernels,short *img,int w,int imgsz,int c)
{
    int sum = 0;
    while (c--)
    {
        sum += img[0]*kernels[0];
        sum += img[1]*kernels[1];
        sum += img[0+w]*kernels[3+0];
        sum += img[1+w]*kernels[3+1];
        img += imgsz;
        kernels += 3*3;
    }
    return(sum);
}
int cnn3x3_3x2(short *kernels,short *img,int w,int imgsz,int c)
{
    int sum = 0;
    while (c--)
    {
        sum += img[0]*kernels[0];
        sum += img[1]*kernels[1];
        sum += img[0+w]*kernels[3+0];
        sum += img[1+w]*kernels[3+1];
        sum += img[0+w*2]*kernels[3*2+0];
        sum += img[1+w*2]*kernels[3*2+1];
        img += imgsz;
        kernels += 3*3;
    }
    return(sum);
}
int cnn3x3_2x3(short *kernels,short *img,int w,int imgsz,int c)
{
    int sum = 0;
    while (c--)
    {
        sum += img[0]*kernels[0];
        sum += img[1]*kernels[1];
        sum += img[2]*kernels[2];
        sum += img[0+w]*kernels[3+0];
        sum += img[1+w]*kernels[3+1];
        sum += img[2+w]*kernels[3+2];
        img += imgsz;
        kernels += 3*3;
    }
    return(sum);
}
void cnn3x3_pad1(short *kernels,short *img,int w,int h,int c,short *out,int shiftbit,int bias )
{
    int i,sum;

    sum = cnn3x3_2x2(&kernels[4],img,w,w*h,c);
    out[0] = (sum+bias)>>shiftbit;

    for (i = 0 ; i < w-2 ; i++)
    {
        sum = cnn3x3_2x3(&kernels[3],&img[i],w,w*h,c);
        out[i+1] = (sum+bias)>>shiftbit;
    }
    sum = cnn3x3_2x2(&kernels[3],&img[w-2],w,w*h,c);
    out[w-1] = (sum+bias)>>shiftbit;

    for (i = 0 ; i < h-2 ; i++)
    {
        sum = cnn3x3_3x2(&kernels[1],&img[i*w],w,w*h,c);
        out[w*(i+1)] = (sum+bias)>>shiftbit;
    }

    for (i = 0 ; i < h-2 ; i++)
    {
        sum = cnn3x3_3x2(&kernels[0],&img[i*w+w-2],w,w*h,c);
        out[w*(i+1)+w-1] = (sum+bias)>>shiftbit;
    }

    sum = cnn3x3_2x2(&kernels[1],&img[w*(h-2)],w,w*h,c);
    out[w*(h-1)] = (sum+bias)>>shiftbit;

    for (i = 0 ; i < w-2 ; i++)
    {
        sum = cnn3x3_2x3(&kernels[0],&img[i+w*(h-2)],w,w*h,c);
        out[w*(h-1)+i+1] = (sum+bias)>>shiftbit;
    }
    sum = cnn3x3_2x2(&kernels[0],&img[w*(h-2)+w-2],w,w*h,c);
    out[w*h-1] = (sum+bias)>>shiftbit;

    cnn3x3_outpad(kernels,img,w,h,c,&out[w+1],shiftbit,bias,2);
}
void cnn3x3(short *kernels,short *img,int w,int h,int c,short *out,int shiftbit,int bias )
{
    cnn3x3_outpad(kernels,img,w,h,c,out,shiftbit,bias,0);
}

int cnn1x5_1xn(short *kernels,short *img,int w,int imgsz,int c,int n)
{
    int sum = 0;
    while (c--)
    {
        sum += img[0]*kernels[0];
        sum += img[1]*kernels[1];
        sum += img[2]*kernels[2];
        if (n >= 4)
        {
            sum += img[3] * kernels[3];
        }
        img += imgsz;
        kernels += 5;
    }
    return(sum);
}

void cnn1x5_pad2(short *kernels,short *img,int w,int h,int c,short *out,int shiftbit,int bias )
{
    int i,sum;

    //not right,use asm to calculate 4 item a group
    if (w < 4+5)
    {
        return;
    }

    for (i = 0 ; i < h ; i++)
    {
        sum = cnn1x5_1xn(&kernels[2],&img[i*w],w,w*h,c,3);
        out[w*i] = (sum+bias)>>shiftbit;
        sum = cnn1x5_1xn(&kernels[1],&img[i*w],w,w*h,c,4);
        out[w*i+1] = (sum+bias)>>shiftbit;

        sum = cnn1x5_1xn(&kernels[0],&img[i*w+w-4],w,w*h,c,4);
        out[w*i+w-2] = (sum+bias)>>shiftbit;

        sum = cnn1x5_1xn(&kernels[0],&img[i*w+w-3],w,w*h,c,3);
        out[w*i+w-1] = (sum+bias)>>shiftbit;
    }


    cnn1x5_outpad(kernels,img,w,h,c,&out[2],shiftbit,bias,4);
}

void cnn1x5(short *kernels,short *img,int w,int h,int c,short *out,int shiftbit,int bias  )
{
    cnn1x5_outpad(kernels,img,w,h,c,out,shiftbit,bias,0);
}


int cnn1x7_1xn(short *kernels,short *img,int w,int imgsz,int c,int n)
{
    int sum = 0;
    while (c--)
    {
        sum += img[0]*kernels[0];
        sum += img[1]*kernels[1];
        sum += img[2]*kernels[2];
        sum += img[3]*kernels[3];
        if (n >= 5)
        {
            sum += img[4]*kernels[4];
        }
        if (n >= 6)
        {
            sum += img[5]*kernels[5];
        }
        img += imgsz;
        kernels += 7;
    }
    return(sum);
}

void cnn1x7_pad3(short *kernels,short *img,int w,int h,int c,short *out,int shiftbit,int bias )
{
    int i,sum;



    for (i = 0 ; i < h ; i++)
    {
        sum = cnn1x7_1xn(&kernels[3],&img[i*w],w,w*h,c,4);
        out[w*i] = (sum+bias)>>shiftbit;
        sum = cnn1x7_1xn(&kernels[2],&img[i*w],w,w*h,c,5);
        out[w*i+1] = (sum+bias)>>shiftbit;
        sum = cnn1x7_1xn(&kernels[1],&img[i*w],w,w*h,c,6);
        out[w*i+2] = (sum+bias)>>shiftbit;

        sum = cnn1x7_1xn(&kernels[0],&img[i*w+w-6],w,w*h,c,6);
        out[w*i+w-3] = (sum+bias)>>shiftbit;

        sum = cnn1x7_1xn(&kernels[0],&img[i*w+w-5],w,w*h,c,5);
        out[w*i+w-2] = (sum+bias)>>shiftbit;

        sum = cnn1x7_1xn(&kernels[0],&img[i*w+w-4],w,w*h,c,4);
        out[w*i+w-1] = (sum+bias)>>shiftbit;
    }


    cnn1x7_outpad(kernels,img,w,h,c,&out[3],shiftbit,bias,6);
}

void cnn1x7(short *kernels,short *img,int w,int h,int c,short *out,int shiftbit,int bias  )
{
    cnn1x7_outpad(kernels,img,w,h,c,out,shiftbit,bias,0);
}


int cnn5x1_nx1(short *kernels,short *img,int w,int imgsz,int c,int n)
{
    int sum = 0;
    while (c--)
    {
        sum += img[0]*kernels[0];
        sum += img[w]*kernels[1];
        sum += img[w*2]*kernels[2];
        if (n >= 4)
        {
            sum += img[w*3]*kernels[3];
        }
        img += imgsz;
        kernels += 5;
    }
    return(sum);
}
void cnn5x1_pad2(short *kernels,short *img,int w,int h,int c,short *out,int shiftbit,int bias )
{
    int i,j,sum;


    for (j = 0; j < 2; j++)
    {
        for (i = 0; i < w; i++)
        {
            sum = cnn5x1_nx1(&kernels[2-j],&img[i],w,w*h,c,3+j);
            *out++ = (sum+bias)>>shiftbit;
        }
    }
    cnn5x1_six(kernels,img,w,h,c,out,shiftbit,bias);
    out += w * (h-4);

    img = &img[w*(h-4)];
    for (j = 0 ; j < 2 ; j++)
    {
        for (i = 0; i < w; i++)
        {
            sum = cnn5x1_nx1(&kernels[0],&img[i],w,w*h,c,4-j);
            *out++ = (sum+bias)>>shiftbit;

        }
        img += w;
    }


}


int cnn7x1_nx1(short *kernels,short *img,int w,int imgsz,int c,int n)
{
    int sum = 0;
    while (c--)
    {
        sum += img[0]*kernels[0];
        sum += img[w]*kernels[1];
        sum += img[w*2]*kernels[2];
        sum += img[w*3]*kernels[3];
        if (n >= 5)
        {
            sum += img[w*4]*kernels[4];
        }
        if (n >= 6)
        {
            sum += img[w*5]*kernels[5];
        }
        img += imgsz;
        kernels += 7;
    }
    return(sum);
}
void cnn7x1_pad3(short *kernels,short *img,int w,int h,int c,short *out,int shiftbit,int bias )
{
    int i,j,sum;

    if ((w < 6)||(h < 7))
    {
        return;
    }


    for (j = 0 ; j < 3 ; j++)
    {
        for (i = 0; i < w; i++)
        {
            sum = cnn7x1_nx1(&kernels[3-j],&img[i],w,w*h,c,4+j);
            *out++ = (sum+bias)>>shiftbit;
        }
    }

    cnn7x1_six(kernels,img,w,h,c,out,shiftbit,bias);
    out += w * (h-6);

    img = &img[w*(h-6)];
    for (j = 0 ; j < 3 ; j++)
    {
        for (i = 0; i < w; i++)
        {
            sum = cnn7x1_nx1(&kernels[0],&img[i],w,w*h,c,6-j);
            *out++ = (sum+bias)>>shiftbit;
        }
        img+=w;
    }

}
