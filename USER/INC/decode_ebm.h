#ifndef __DECODE_EBM_H__
#define __DECODE_EBM_H__

#ifdef __cplusplus
 extern "C" {
#endif

int huff_decode_ebm(unsigned char *buffer,int buff_sz,int src, int dst);

int load_and_unpack_parameter(void);

#ifdef __cplusplus
   }
#endif

#endif
