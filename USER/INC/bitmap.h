#ifndef __BITMAP_H__
#define __BITMAP_H__

#ifdef __cplusplus
 extern "C" {
#endif

signed char bitmap_set(unsigned char bitmap[], int max_count,
                     unsigned char number[],unsigned char count,
                     unsigned char value);

signed char bitmap_get_clear_bits(unsigned char bitmap[], int max_count,
                     unsigned char number[],unsigned char number_count);

unsigned char bitmap_is_bit_clear(unsigned char bitmap[], int max_count, unsigned char number);

#ifdef __cplusplus
 }
#endif

#endif /* __BITMAP_H__ */
