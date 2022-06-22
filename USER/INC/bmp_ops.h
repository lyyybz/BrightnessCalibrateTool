#ifndef __BMP_OPS_H__
#define __BMP_OPS_H__

#ifdef __cplusplus
extern "C" {
#endif
	 
#include <stdint.h>
#include <string.h>

int get_threshold(const uint8_t bmp[],int w,int h );
float myln(float x);
float SqrtByCarmack( float number );
int get_digit_upline_pos(uint8_t bmp[],int w,int h);
int find_gap(uint8_t bmp[],int w,int h,int *upline,int *downline);
int calc_upline(int upline,int downline);
int find_56_gap(uint8_t bmp[],int *upline,int *downline); 
 int count_vline_blackdot(unsigned char *map,short int width,short int height,
                                 int x,int starty,int stopy);
 int count_hline_blackdot(unsigned char *map,short int width,short int height,
                                 int y,int startx,int stopx); 
int calc_frac(int startline);
#ifdef __cplusplus
}
#endif

#endif
