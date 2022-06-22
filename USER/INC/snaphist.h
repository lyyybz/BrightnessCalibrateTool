#ifndef __SNAPHIST_H__
#define __SNAPHIST_H__
#ifdef __cplusplus
extern "C" {
#endif
 
#define  IMG_CACHE_ADDR           0x00000
#define  IMG_CACHE_SZ          0x80000
#define  BMP_CACHE_SZ           (0x800  * 5)

int get_cur_blk_pos(void);
 void write_view_to_flash(int pos,int width ,int height);
 int _get_digits_to_flash(void);

int _get_digits_to_ram(unsigned char buff[],unsigned char id);



int _get_digits_null(void);
int get_digits_from_flash_1(void);


#ifdef __cplusplus
}
#endif

#endif
