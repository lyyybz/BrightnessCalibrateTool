 
#ifndef __OV7670_H__
#define __OV7670_H__

#define OV7670_DEVICE_ADDRESS    0x42 
      
 
#define GC0308_DEVICE_ADDRESS    0x42 

#ifndef END_PARAMETER
#define END_PARAMETER           0xff
#endif

int write_camera_reg(unsigned char dev_addr,unsigned char reg_addr, unsigned char regdata);
unsigned char read_camera_reg(unsigned char dev_addr,unsigned char reg_addr);
int cmos_init(void);

int cmos_ov7670_init(unsigned char dev_addr);
int cmos_gc0308_init(unsigned char dev_addr);


int comos_0308_write_windows_rect(short x0,short y0,short height,short width);
int comos_7670_write_windows_rect(short x0,short y0,short height,short width);

#endif

