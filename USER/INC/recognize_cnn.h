#ifndef _RECOGNIZE_CNN_H_
#define _RECOGNIZE_CNN_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include "config.h"
#include "cache_control.h"
#include "cnn_para.h"

#define FULLCONNECT_LAYER    12
#define POOL_AVERGAE_LAYER   11

//#define  current_mulx           10
#define  BMP_ENLARGE   (LAYER0_MULX- 8)

/*最小可信概率 范围0-100*/
#define CONFIDIENCE_PROBILITY_LOW    35

#ifdef ESSN_WCR_P
    #define CONFIDIENCE_PROBILITY_HIGH   41
#else
    #define CONFIDIENCE_PROBILITY_HIGH   70
#endif
#define CONFIDIENCE_PROBILITY_MASK   87


#define CNN_L2_CACHE_LINE_CNT   1
#define CNN_L1_FEATURE_CNT     10
#define CNN_L1_KERNEL_SZ     5
#define CNN_L2_KERNEL_SZ     3

#define CNN_MAX_CALC_EXT_CONV_LINE_CNT   ((((1 * 2) + CNN_L2_KERNEL_SZ -1)*2 + CNN_L1_KERNEL_SZ -1) - 1)
#define CNN_MAX_CALC_CONV_HEIGHT   (DIGIT_HEIGHT + CNN_MAX_CALC_EXT_CONV_LINE_CNT*2)  //so complicate!!
#define CNN_MAX_CALC_CNN_L1_LINE_CNT   ((CNN_MAX_CALC_CONV_HEIGHT - CNN_L1_KERNEL_SZ + 1)/2)
#define CNN_MAX_CALC_CNN_L2_LINE_CNT   ((CNN_MAX_CALC_CNN_L1_LINE_CNT - CNN_L2_KERNEL_SZ + 1)/2)

#if CNN_L1_KERNEL_SZ > CNN_L2_KERNEL_SZ
#define CNN_MAX_KERNEL_SZ   CNN_L1_KERNEL_SZ
#else
#define CNN_MAX_KERNEL_SZ   CNN_L2_KERNEL_SZ
#endif
 
#define CNN_L1_LINE_CNT  ((DIGIT_HEIGHT - CNN_L1_KERNEL_SZ + 1)/2)       
#define CNN_L2_LINE_CNT   ((CNN_L1_LINE_CNT - CNN_L2_KERNEL_SZ + 1)/2)

#define CNN_L1_CONV_WIDTH   (DIGIT_WIDTH - CNN_L1_KERNEL_SZ + 1)
#define CNN_L1_LINE_WIDTH   (CNN_L1_CONV_WIDTH/2)

#define CNN_L1_CACHE_LINE_CNT   (2*CNN_L2_CACHE_LINE_CNT + CNN_L2_KERNEL_SZ -1)
#define CNN_L2_FEATURE_CNT      14
                          
#define CNN_RAW_GRAY_CACHE_LINE_CNT  (2*CNN_L1_CACHE_LINE_CNT + CNN_L1_KERNEL_SZ -1)

#define CNN_L2_CONV_WIDTH   (CNN_L1_LINE_WIDTH - CNN_L2_KERNEL_SZ + 1)
#define CNN_L2_LINE_WIDTH   (CNN_L2_CONV_WIDTH /2)
 
#define CNN_L3_DOT_CNT      (128)
#define CNN_L4_DOT_CNT      (100)

#ifdef KEIL
    #pragma anon_unions
#endif



typedef struct  cnn_result_type  cnn_result_t;
struct cnn_result_type 
{
    unsigned char max_probility;
    unsigned char second_probility;
    unsigned char digit;
};




#define CNN_NULL_DATA  {0,0,0,{(short*)0}}



void cnn_data_init_and_alloc(cnn_data_t *data,char width,char height,char cnt);
void cnn_data_free_data(cnn_data_t *data);

int  recognize_a_gray_digit_cnn(int digit_id,int net_id,cnn_result_t *result);

#ifdef __cplusplus
}
#endif
#endif
