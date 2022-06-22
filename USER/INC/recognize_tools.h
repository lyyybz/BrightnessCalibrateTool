#ifndef __RECOGNIZE_TOOLS_H
#define __RECOGNIZE_TOOLS_H

#ifdef __cplusplus
 extern "C" {
#endif
#include "cnn_para.h"
#include "recognize_cnn.h"

extern const unsigned short int tanh_map[]; 
extern const unsigned short int sigm_map[]; 
extern const unsigned int exp_table[];

#define TANH_MULT  13


/*激活函数*/
float sigmod_f(float z);
float tansig(float z);


unsigned int  exp_u(int z);
int tanh_int(int in,int out_mulx);

int  mult_char_short_to_int(signed char *x, signed short *y, int sz) ;


long long  mult_short_short_to_int(short *x, short  *y, int sz);
long long  mult_short_int_to_ll(short *x, signed int  *y, int sz);


void  cnn_log_feature(cnn_para_index_t *index,cnn_data_t *out);
void  cnn_log_int(const char *fmt, int x);

#ifdef CONFIG_LOG_FEATUERE
    void switch_feature(int x); 
	#define CNN_LOG_FEATUEE(layer,out_data)  cnn_log_feature(layer,out_data)
	#define CNN_SWITCH_FEATUEE(x) switch_feature(x)
    #define CNN_LOG_INT(fmt,x) cnn_log_int(fmt,x);
#else 
	#define CNN_LOG_FEATUEE(layer,data)
	#define CNN_SWITCH_FEATUEE(x)
	#define CNN_LOG_INT(fmt,x)
#endif 

#ifdef __cplusplus
}
#endif

#endif 
