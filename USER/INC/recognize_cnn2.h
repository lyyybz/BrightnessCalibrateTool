ifndef _RECOGNIZE_CNN2_H_
#define _RECOGNIZE_CNN2_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include "config.h"
 
#ifndef KEIL

#define PARA_DMA_WORD_SZ  DIGIT_WIDTH
#endif


#define CNN2_L2_CACHE_LINE_CNT   1
#define CNN2_L1_FEATURE_CNT      6
#define CNN2_L1_KERNEL_SZ     5
#define CNN2_L2_KERNEL_SZ     3

#define CNN2_MAX_CALC_EXT_CONV_LINE_CNT   ((((1 * 2) + CNN2_L2_KERNEL_SZ -1)*2 + CNN2_L1_KERNEL_SZ -1) - 1)
#define CNN2_MAX_CALC_CONV_HEIGHT   (DIGIT_HEIGHT + CNN2_MAX_CALC_EXT_CONV_LINE_CNT*2)  //so complicate!!
#define CNN2_MAX_CALC_CNN2_L1_LINE_CNT   ((CNN2_MAX_CALC_CONV_HEIGHT - CNN2_L1_KERNEL_SZ + 1)/2)
#define CNN2_MAX_CALC_CNN2_L2_LINE_CNT   ((CNN2_MAX_CALC_CNN2_L1_LINE_CNT - CNN2_L2_KERNEL_SZ + 1)/2)

#if CNN2_L1_KERNEL_SZ > CNN2_L2_KERNEL_SZ
#define CNN2_MAX_KERNEL_SZ   CNN2_L1_KERNEL_SZ
#else
#define CNN2_MAX_KERNEL_SZ   CNN2_L2_KERNEL_SZ
#endif
 
#define CNN2_L1_LINE_CNT  ((DIGIT_HEIGHT - CNN2_L1_KERNEL_SZ + 1)/2)       
#define CNN2_L2_LINE_CNT   ((CNN2_L1_LINE_CNT - CNN2_L2_KERNEL_SZ + 1)/2)

#define CNN2_L1_CONV_WIDTH   (DIGIT_WIDTH - CNN2_L1_KERNEL_SZ + 1)
#define CNN2_L1_LINE_WIDTH   (CNN2_L1_CONV_WIDTH/2)

#define CNN2_L1_CACHE_LINE_CNT   (2*CNN2_L2_CACHE_LINE_CNT + CNN2_L2_KERNEL_SZ -1)
#define CNN2_L2_FEATURE_CNT      8

#define CNN2_RAW_GRAY_CACHE_LINE_CNT  (2*CNN2_L1_CACHE_LINE_CNT + CNN2_L1_KERNEL_SZ -1)

#define CNN2_L2_CONV_WIDTH   (CNN2_L1_LINE_WIDTH - CNN2_L2_KERNEL_SZ + 1)
#define CNN2_L2_LINE_WIDTH   (CNN2_L2_CONV_WIDTH /2)
 
 
#define CNN2_L3_DOT_CNT      (128)
#define CNN2_L4_DOT_CNT      (100) 

#ifdef KEIL
    #pragma anon_unions
#endif
struct CNN2MEM
{
    unsigned char max_gray,min_gray,span_gray;
    int digit_idx,img_pos;         //above member should be same pos in two cnn2 struct!!!!
    signed short int  raw_gray[CNN2_RAW_GRAY_CACHE_LINE_CNT][DIGIT_WIDTH];
    short int raw_gray_start,raw_gray_cnt,raw_gray_head;
        

	short gain[3];
	signed int layer1features[CNN2_L1_FEATURE_CNT][CNN2_L1_CACHE_LINE_CNT][CNN2_L1_LINE_WIDTH];
	short int layer1_start, layer1_cnt, layer1_head;
	signed int layer2features[CNN2_L2_FEATURE_CNT][CNN2_L2_CACHE_LINE_CNT][CNN2_L2_LINE_WIDTH];

	union
	{

	    long long layer3_llong[CNN2_L3_DOT_CNT];
	    int layer4_int[CNN2_L4_DOT_CNT];
	}; 


	int layer1bias[CNN2_L1_FEATURE_CNT];
	int layer2bias[CNN2_L2_FEATURE_CNT];


	signed short cache_kernels[2 + (CNN2_L1_FEATURE_CNT * CNN2_MAX_KERNEL_SZ*CNN2_MAX_KERNEL_SZ)]; //use for dma ,so add 2 ahead!!
	signed short int lw[2 + CNN2_L2_LINE_WIDTH * CNN2_L3_DOT_CNT];

	int net_addr;
};

struct CNN2_L3_DUAL_LW
{
    signed short para[2][2+ CNN2_L3_DOT_CNT];
};
struct CNN2_L1_K
{
    signed short k[CNN2_L1_KERNEL_SZ * CNN2_L1_KERNEL_SZ];
};
struct CNN2_L2_K
{
    signed short k[CNN2_L2_KERNEL_SZ * CNN2_L2_KERNEL_SZ];
};
 
struct CNN2_W_B
{
    signed short layer1kernel[CNN2_L1_FEATURE_CNT][CNN2_L1_KERNEL_SZ * CNN2_L1_KERNEL_SZ];
	signed int layer1bias[CNN2_L1_FEATURE_CNT];
	signed short layer2kernel[CNN2_L2_FEATURE_CNT][CNN2_L1_FEATURE_CNT][CNN2_L2_KERNEL_SZ * CNN2_L2_KERNEL_SZ];
	signed int layer2bias[CNN2_L2_FEATURE_CNT];
};
 
int recognize_a_gray_digit_cnn2(void);

#ifdef __cplusplus
}
#endif
#endif
