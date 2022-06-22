#ifndef _SPIFLASH_H_
#define _SPIFLASH_H_
#include "stdint.h"

#define flash_cs_low()       GPIO_ResetBits(GPIOA, GPIO_Pin_15)
/* Deselect SPI FLASH: Chip Select pin high */
#define flash_cs_high()      GPIO_SetBits(GPIOA, GPIO_Pin_15)

#define  PAGE_SIZE      	0x100	//bytes
#define  SECTOR_SIZE        0x1000	//bytes
#define  BLOCK_SIZE         0x10000 //bytes

#define  MAX_PAGE_CNT      	0x8000
#define  MAX_SECTOR_CNT		0x800
#define  MAX_BLOCK_CNT		0x80

void sys_spi_init(void);
int flash_read(int addr,unsigned char buffer[],int len);
int flash_write(int addr,unsigned char buffer[],int len);
void SPI_FLASH_SectorErase(int SectorAddr);
//void SPI_FLASH_Write(int WriteAddr,unsigned char buffer[],int NumByteToWrite);
void SPI_FLASH_Write(uint16_t WriteAddr,uint8_t buffer[],uint8_t NumByteToWrite);
void SPI_FLASH_Read(int ReadAddr,unsigned char buffer[],int NumByteToRead);
#endif
