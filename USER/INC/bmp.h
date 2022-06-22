/*
 * FILE								: bmp.h
 * DESCRIPTION				: This file is iHMI43 bmp file header.
 * Author							: XiaomaGee@Gmail.com
 * Copyright					:
 *
 * History
 * --------------------
 * Rev								: 0.00
 * Date								: 03/05/2012
 *
 * create.
 * --------------------
 */
#ifndef __bmp_h__
#define __bmp_h__

//----------------- Define ------------------------------//
//COMPRESSION
#define BI_RGB	0
#define BI_RLE8	1
#define BI_RLE4	2
#define BI_BITFIELDS	3

#define LINE_PIXEL	480

//----------------- Typedef -----------------------------//
#pragma pack(2)

typedef __packed struct{
	unsigned char type[2];
	unsigned long int file_size;
	unsigned long int reserved;
	unsigned long int offset;
	unsigned long int header_info_size;   //0x28
	unsigned long int width;
	unsigned long int height;
	unsigned short int planes;   //always 1
	unsigned short int bit_count; // 1 4 8 16 24 32
	unsigned long int compression;
	unsigned long int image_size; //can set 0(rgb)
	unsigned long int xpels_per_meter;
	unsigned long int ypels_per_meter;
	unsigned long int color_used;
	unsigned long int color_important; 
	
}BMP_HEADER_T;



typedef __packed struct{
	unsigned char type[2];
	unsigned long int file_size;
	unsigned long int reserved;
	unsigned long int offset;
	unsigned long int header_info_size;   //0x28
	unsigned long int width;
	unsigned long int height;
	unsigned short int planes;   //always 1
	unsigned short int bit_count; // 1 4 8 16 24 32
	unsigned long int compression;
	unsigned long int image_size; //can set 0(rgb)
	unsigned long int xpels_per_meter;
	unsigned long int ypels_per_meter;
	unsigned long int color_used;
	unsigned long int color_important; 
	unsigned long int mask; 
	
}BMP_MASK_T;

typedef struct{
	int (* header0)(char * file,BMP_HEADER_T *);
	int (* header)(char * file,BMP_MASK_T *);
	int (* show)(char * file,int x,int y);
	int (* capture)(void);
}BMP_T;

//----------------- Extern ------------------------------//
extern BMP_T bmp;

#pragma pack()


#endif //__bmp_h__
