﻿/*******************************************************************************
*  Copyright (C) 2010 - All Rights Reserved
*		
* ???者:	?飞家?
* ?权?有: ?飞?子	
* 创建日期:	2012?8月18日 
* ???史:	2012?8月18日首?
* Version:  1.0 
* Demo 淘宝地址：http://store.taobao.com/shop/view_shop.htm?asker=wangwang&shop_nick=qifeidianzi
**********************************************************************************************************************************************
懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒
懒懒懒懒懒懒懒懒懒懒懒一懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒困一懒懒懒懒懒懒懒懒懒懒懒懒懒懒一一一一一一一懒懒懒懒懒懒懒
懒懒困一一一一懒一一一一一一?懒懒懒懒困一一一一一一一一懒懒懒懒懒懒懒懒懒四厉懒懒一一懒懒懒一懒懒懒懒懒懒懒懒懒一一一一四四一一一懒懒懒懒懒懒
懒懒懒懒懒懒一懒懒懒一?一懒懒懒懒懒懒一一一一四厉?一四懒一一懒懒懒懒懒四一一一一一一一一一一一懒懒懒懒懒懒懒懒四懒懒四一一一一厉懒懒懒懒懒懒
懒懒懒四厉厉一懒懒厉懒懒懒四懒懒懒懒懒懒懒懒懒懒懒困一懒一一懒懒懒懒懒懒懒一一厉厉一一厉厉厉一一懒懒懒懒懒懒厉厉厉厉厉厉一一厉厉厉懒懒懒懒懒懒
懒懒懒一懒?一一一一一一一一一一一懒懒懒懒懒懒懒懒?一一困懒懒懒懒懒懒懒懒一一一一一一一一一一四懒懒懒懒懒一一一一一一一一一一一一一一一懒懒懒
懒懒懒一一一一一厉?一一厉一懒厉懒懒懒懒懒懒懒懒懒一一一一一懒懒懒懒懒懒懒一一懒懒一一懒懒懒一困懒懒懒懒懒一一四懒懒懒懒一一懒懒困一一四懒懒懒
懒懒懒懒懒懒懒一困一懒一厉一懒懒懒懒懒懒懒懒懒懒懒困一懒懒一一懒懒懒懒懒懒一一一一一一一一一一厉懒懒懒懒懒懒懒懒懒懒懒懒一一厉懒懒懒懒懒懒懒懒
懒?一一一懒四困厉一一一厉一懒懒懒懒懒懒懒懒懒懒懒懒一一懒懒懒懒懒懒懒懒懒懒懒懒懒四一厉懒懒懒懒懒懒一懒懒懒懒懒懒懒懒懒一一厉懒懒懒懒懒懒懒懒
懒懒懒懒懒懒一懒懒懒懒懒一一懒懒懒懒懒懒懒懒懒懒懒懒厉一一一一一一困懒懒懒懒懒懒懒懒一一一一一一一一一懒懒懒懒懒困懒懒厉一一懒懒懒懒懒懒懒懒懒
懒懒懒懒一一一懒懒困一一一一懒懒懒懒懒懒懒懒懒懒懒懒懒懒一一一一一懒懒懒懒懒懒懒懒懒懒一一一一一一一懒懒懒懒懒懒懒一一一一四懒懒懒懒懒懒懒懒懒
懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒懒
**********************************************************************************************************************************************/

#ifndef OV7670_H
#define OV7670_H

#define GC0308_DEVICE_WRITE_ADDRESS    0x42
#define GC0308_DEVICE_READ_ADDRESS     0x43
     
#define CMOS1_RST  PBout(4)
#define CMOS1_PWDN PBout(3)
#define	WriteControl	PCout(13) 

/*OV7670FIFO模?控制?脚定?*/ 
#define FIFO_WR    PBout(1) 
#define FIFO_WRST  PBout(5)
#define FIFO_RCK   PBout(2)
#define FIFO_RRST  PBout(7)
#define FIFO_OE	   PBout(6)



void set_gc0308_reg(void);	
unsigned char Cmos7670_init(void);
unsigned char WrCmos7670(unsigned char regID, unsigned char regDat);
unsigned char rdCmos7670Reg(unsigned char regID);
void Cmos7670_Size(unsigned int Startx,unsigned int Starty,unsigned int width,unsigned int height);

#endif
