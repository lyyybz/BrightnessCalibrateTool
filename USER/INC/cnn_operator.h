#ifndef __CNN_OPERATOR__
#define __CNN_OPERATOR__

#include "recognize_cnn.h"

#ifdef KEIL
    #define CHECK_IN_INT_RANGE(x)
#else 
    void  check_int_range(long long x);
	#define CHECK_IN_INT_RANGE(x)   check_int_range(x);
#endif

#define cnn_data_alloc_output_mem(out,width1,height1,cnt,out_data) \
    if(NULL == out->data)\
    {\
        out->data = mem_alloc((cnt)*(width1)*(height1)*2);\
        (out_data) = out->data;\
    }\
    else\
    {\
        (out_data) = mem_expand_feature(out->data,(cnt)*(width1)*(height1)*2);\
        if(out->data > out_data)\
        {   \
            out->data = out_data;\
        } \
    }\
    out->width = (width1);\
    out->height = (height1);


enum
{
 /*0-20:    width,height,stride,pading*/
 OPERATOR_CNN_K11_S11_P00_D11_RELU=1,
 OPERATOR_CNN_K33_S11_P00_D11_RELU=2,
 OPERATOR_CNN_K33_S11_P11_D11_RELU=3,
 OPERATOR_CNN_K33_S22_P00_D11_RELU=4,
 OPERATOR_CNN_K51_S11_P20_D11=5,
 OPERATOR_CNN_K15_S11_P02_D11_RELU=6,
 OPERATOR_CNN_K71_S11_P30=7,
 OPERATOR_CNN_K17_S11_P03_D11_RELU=8,
 OPERATOR_CNN_K33_S22_P11_D22_RELU=9,
 OPERATOR_CNN_K33_S22_P10_D21_RELU=10,

 OPERATOR_FULLCNNT=20,
 OPERATOR_POOL_MAX_K33_S22_P00=21,
 OPERATOR_POOL_AVG_K33_S22_P00=22,
 OPERATOR_NORM_RELU =23,
 OPERATOR_POOL_AVG_K33_S11_P00=24,
 OPERATOR_POOL_AVG_K15_S11_P00=25,
 OPERATOR_POOL_AVG_K22_S22_P00=29,
 OPERATOR_CBAM_CHANNEL=26,
 OPERATOR_SPATIAL_AVG_MAX=27,
 OPERATOR_POOL_AVG_K13_S11_P00=30,
};

#define DMA_EXTRA_SIZE   4

struct cnn_operator_type
{
	int type;
	void (*operator)(cnn_layer_info_t *info,cnn_data_t *in,cnn_data_t *out);
};

void layer_exp_activate(cnn_data_t *in);

void cnn_layer_calc_feature(cnn_layer_info_t *info,cnn_data_t *in,cnn_data_t *out);
#endif 
