
#include<I2C.h>
#include<delay.h>
#include<camera_dev.h>
#include "bmp_layer.h"
#include "device_info.h"

unsigned char read_camera_reg(unsigned char dev_addr,unsigned char reg_addr)
{
    unsigned char regdata;
    start_i2c();
    if(0==i2c_write(dev_addr))
    {
        stop_i2c();
        return(0);
    }
    delay_us(100);
    if(0==i2c_write(reg_addr))
    {
        stop_i2c();
        return(0);
    }
    stop_i2c();

    delay_us(100);

    start_i2c();
    if(0==i2c_write(dev_addr | 1))
    {
        stop_i2c();
        return(0);
    }
    delay_us(100);
    regdata=i2c_read();
    i2c_no_ack();
    stop_i2c();
    return(regdata);
}

int write_camera_reg(unsigned char dev_addr,unsigned char reg_addr, unsigned char regdata)
{
    start_i2c();
    if(0==i2c_write(dev_addr))
    {
        stop_i2c();
        return(0);
    }
    delay_us(10);
    if(0==i2c_write(reg_addr))
    {
        stop_i2c();
        return(0);
    }
    delay_us(10);
    if(0==i2c_write(regdata))
    {
        stop_i2c();
        return(0);
    }
    stop_i2c();

    return(1);
}

#define GC_7670   1
int cmos_init(void)
{
    int x;
#if GC_7670
    //test if 7670
		unsigned char id[2];
    id[0] = read_camera_reg(OV7670_DEVICE_ADDRESS,0x1c);
    id[1] = read_camera_reg(OV7670_DEVICE_ADDRESS,0x1d);
    x= id[0] |(id[1] << 8);
    if (x == 0xa27f)
    {
      cmos_ov7670_init(OV7670_DEVICE_ADDRESS);
      return(1);
		}
#else
    //test if gc0308
    x = read_camera_reg(GC0308_DEVICE_ADDRESS,0);
    if (x == 0x9b)
    {
        cmos_gc0308_init(GC0308_DEVICE_ADDRESS);
        return(2);
    }
#endif
    return(-1);
}


/*摄像头设置采样频率是2选1*/
int comos_0308_write_windows_rect(short x0,short y0,short height,short width)
{
    width = width*2;
    height = height*2;
    x0 = 640-(x0*2)- width;
    y0 = y0*2;
    write_camera_reg(GC0308_DEVICE_ADDRESS, 5,y0>>8);
    write_camera_reg(GC0308_DEVICE_ADDRESS, 6,y0&0xff);
    write_camera_reg(GC0308_DEVICE_ADDRESS, 7,x0>>8);
    write_camera_reg(GC0308_DEVICE_ADDRESS, 8,x0&0xff);
    write_camera_reg(GC0308_DEVICE_ADDRESS, 9,(height+8)>>8);
    write_camera_reg(GC0308_DEVICE_ADDRESS, 10,(height+8)&0xff);
    write_camera_reg(GC0308_DEVICE_ADDRESS, 11,(width +8)>>8);
    write_camera_reg(GC0308_DEVICE_ADDRESS, 12,(width +8)&0xff);

    write_camera_reg(GC0308_DEVICE_ADDRESS, 0xf7,1);
    write_camera_reg(GC0308_DEVICE_ADDRESS, 0xf8,1);
    write_camera_reg(GC0308_DEVICE_ADDRESS, 0xf9,width/4-2);
    write_camera_reg(GC0308_DEVICE_ADDRESS, 0xfa,height/4-2);

    return 0;
}

int comos_7670_write_windows_rect(short startx,short starty,short height,short width)
{
	
//		0x32 HREF        读写HREF控制 位[5：3]：HREF结束的低3位（高8位在HSTOP) 位[2：0]：HREF 结束的低 3 位（高 8 位在HSTOP)
//		0x17 HSTART      输出格式-行频开始高八位（低三位在HREF[2：0]）
//		0x18 HSTOP       输出格式-行频结束高八位（低三位在HREF[5：3]）
	
//		0x03 VREF        位[3：2]VREF 结束的低两位(高八位见VSTOP[7:0] 位[1：0]VREF 开始的低两位(高八位见 VSTOP[7:0]
//		0x19 VSTRT       输出格式-场频开始高八位（低二位在VREF[1：0]）
//		0x1A VSTOP       输出格式-场频结束高八位（低二位在VREF[3：2]）
	
    unsigned short endx=(startx + width*2);
		unsigned short endy=(starty + height*2);
		unsigned short temp;
		//设置HREF
		temp = read_camera_reg(OV7670_DEVICE_ADDRESS, 0x32);								//读取Href之前的值
		temp &= 0xC0;
		temp |= ((endx&0x7)<<3) | (startx&0x7);
		write_camera_reg(OV7670_DEVICE_ADDRESS, 0x32, temp);								//设置Href的start和end的最低3位
		write_camera_reg(OV7670_DEVICE_ADDRESS, 0x17, (startx&0x7F8)>>3);		//设置Href的start高8位
		write_camera_reg(OV7670_DEVICE_ADDRESS, 0x18, (endx&0x7F8)>>3);			//设置Href的end的高8位
		//设置VREF
		temp = read_camera_reg(OV7670_DEVICE_ADDRESS, 0x03);								//读取Vref之前的值
		temp &= 0xF0;
		temp |= ((endy&0x3)<<2) | (starty&0x3);
		write_camera_reg(OV7670_DEVICE_ADDRESS, 0x03, temp);								//设置Vref的start和end的最低2位
		write_camera_reg(OV7670_DEVICE_ADDRESS, 0x19, (starty&0x3FC)>>2);		//设置Vref的start高8位
		write_camera_reg(OV7670_DEVICE_ADDRESS, 0x1A, (endy&0x3FC)>>2);			//设置Vref的end的高8位

    return 0;
}


