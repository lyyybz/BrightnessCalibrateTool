#ifndef _RECOGNIZE_H_
#define _RECOGNIZE_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdint.h>
#include "config.h"
#include "recognize_cnn.h"
#include "bmp_layer.h"


#define NORM_BATCH     0
#define LW3_OPTIMIZE    1
#define RELU            0
#define TANH            1
#define LAYER3_ACTIVATE_MTD   TANH

#ifndef INT_MIN
    #define INT_MIN  (-2147483648)
#endif

extern unsigned char _REAL_NUMS[MAX_DIGIT_CNT];



extern int  (*capture_bmp)(void);
extern void (*read_bmp)(unsigned char buff[],unsigned char id);

enum
{

    LAYER_CNN1=0,
    LAYER_CNN2=1,
    LAYER_FULLCONNECT1=2,
    LAYER_FULLCONNECT2=3,
    LAYER_BMP,
    LAYER_PRECALC,
    LAYER_SOFTMAX,
};

int category2digits(signed char real_numbers[]
                          ,signed char read_numbers[]
                          ,int cnt);

int fix_dirty_digit(signed char raw_numbers[], signed char previous_numbers[],int digit_cnt);

signed char digit_map(signed char digit, int idx, signed char previous);

/*
  识别失败返回负数，否则返回正数。
*/
int recognize_gray_digits_dual_cnn(unsigned char real_numbers[]);
int recognize_gray_digits(unsigned char real_numbers[],
                         int  (*recognize_digit)(int digit_id,int net_id,cnn_result_t *result));
int reconginze_raw_number_use_cnn(unsigned char raw_numbers[]);

#ifdef __cplusplus

}
#endif
#endif
