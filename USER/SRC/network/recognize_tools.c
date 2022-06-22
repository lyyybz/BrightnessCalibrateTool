#include "recognize_tools.h"
#include "config.h"
#include <math.h>
#include "recognize_cnn.h"

#if  0
float tansig(float z)
{
    float a;
    a = 2 / (1.0 + exp(-2*z)) - 1;
    return(a);
}

float sigmod_f(float z)
{
    float ret;
    ret = 1.0 / ( 1.0 + exp(-z));
    return(ret);
}
float mult_pair_f(float x[],float y[],int sz)
{
    int i;
    float ret = 0.0;
    for(i = 0 ; i < sz ; i++)
    {
        ret += x[i] * y[i];
    }
    return(ret);
}

#endif

int  mult_char_short_to_int(signed char *x, signed short  *y, int sz)
{
    int ret = 0;
    while (sz >= 8)
    {

        ret += *x++ * *y++;
        ret += *x++ * *y++;
        ret += *x++ * *y++;
        ret += *x++ * *y++;
        ret += *x++ * *y++;
        ret += *x++ * *y++;
        ret += *x++ * *y++;
        ret += *x++ * *y++;

        sz -=8;

    }
    while (sz-- > 0)
    {
        ret += *x++ * *y++;
    }

    return (ret);
}


long long  mult_short_short_to_int(signed short *x, signed short  *y, int sz)
{
    int i;
    long ret  ;
    long long llret = 0;

    while (sz >=5)
    {
        ret = 0;

        ret += x[0] * y[0];
        ret += x[1] * y[1];
        ret += x[2] * y[2];
        ret += x[3] * y[3];
        ret += x[4] * y[4];

        sz -= 5;
        x += 5;
        y += 5;

        llret += ret;
    }
    ret = 0;
    for (i = 0; i < sz; i++)
    {
        ret += x[i] *  y[i];
    }
    llret += ret;

    return(llret);
}

long long  mult_short_int_to_ll(short *x, signed int  *y, int sz)
{
    int i;
    int ret  ;
    long long llret = 0;

    while (sz >=5)
    {
        ret = 0;

        ret += x[0] * y[0];
        ret += x[1] * y[1];
        ret += x[2] * y[2];
        ret += x[3] * y[3];
        ret += x[4] * y[4];

        sz -= 5;
        x += 5;
        y += 5;

        llret += ret;
    }
    ret = 0;
    for (i = 0; i < sz; i++)
    {
        ret += x[i] *  y[i];
    }
    llret += ret;

    return(llret);

}

#if MATH_IN_FLASH
/*
  输入需要扩大 2^13次幂
*/
// flash中x的自变量范围是 -16 - 16 参数的扩大倍数是1024*8
unsigned int  exp_u(int z)
{
    unsigned int sigm;

    if (z <= -16 * 128)
    {
        return (0);
    } else if (z >= 16 * 128)
    {
        return  0x87975f;
    }
    //分割精度为 2*16*1024*8/(1024*8) = 32
    z <<= 1;
    z += 4096;
    dataflash_read(EXP_MAP_ADDR+z*sizeof(unsigned int),&sigm,sizeof(sigm));
    return (sigm);
}

// 大小为16K,数组总共8K个。
//自变量大范围-8192~8192
// tanh的值域为 -1 到 1。输入参数扩大2^13， 从0开始，每步大小为2^2。
int tanh_int(int in,int value_mulx)
{
    int flag, x;
    short int tanh;
    int out;

    in = in << (TANH_MULT - value_mulx);

    if (in < 0)
    {
        in = -in;
        flag = 1;
    }
    else
    {
        flag = 0;
    }

    if (in >=4*(0x2000))
    {
        x = 1 << TANH_MULT;
    }
    else
    {
        x = in;
        x >>= 2; //0~1
        dataflash_read(TANH_ADDR+x*sizeof(short),&tanh,sizeof(tanh));
        x = tanh;
    }

    if (flag)
    {
        out = -x;
    }
    else
    {
        out = x;
    }
    out = out >> (TANH_MULT - value_mulx);
    return out;
}

#else
unsigned int  exp_u(int z)
{
    unsigned int sigm=0;
    double x=z;
    if (z <= -16 * DEFAULT_AMPIFIER_NUMBER)
    {
        return (0);
    } else if (z >= 16 * DEFAULT_AMPIFIER_NUMBER)
    {
        return  0x87975f;
    }
    x = x/pow(2,DEFAULT_AMPIFIER_NUMBER);
    return exp(x);
}

#endif




