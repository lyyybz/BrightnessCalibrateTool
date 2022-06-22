#ifndef _INODE_H_
#define _INODE_H_
//#include <stm32f10x.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
 
#include <stdlib.h>
#include <delay.h>
#include "config.h" 

#ifdef __cplusplus
 extern "C" {
#endif

#define  OS_CRITICAL_METHOD   3
typedef unsigned int   OS_CPU_SR;                /* Define size of CPU status register (PSR = 32 bits) */

#if OS_CRITICAL_METHOD == 3
#define  OS_ENTER_CRITICAL()  {cpu_sr = OS_CPU_SR_Save();}
#define  OS_EXIT_CRITICAL()   {OS_CPU_SR_Restore(cpu_sr);}
#endif

OS_CPU_SR OS_CPU_SR_Save(void);
void OS_CPU_SR_Restore(OS_CPU_SR cpu_sr);


#define BUFFER_LEN 0x08

struct _CHN_SLOT
{
	uint16_t rx;
	uint16_t tx;
	uint8_t data_cnt;
	uint8_t data_max;
	//uint8_t  buffer[BUFFER_LEN];
	//uint8_t  t_index;
	//uint8_t  r_index;
	//uint16_t idle_tick;
};
#define MAX_BUFFER_SZ           (MAX_POOL_SZ-BUFFER_NO_SZ)
#define BUFFER_NO_SZ            (MAX_POOL_SZ>>BLK_NO_SHIFT)
#define MAX_POOL_SZ             0x100

struct _CHN_POOL_MGR
{
	uint8_t buffer[0x100];
	uint8_t free_bitmap;
};
#define BLK_NO_SHIFT  5
#define BLK_SZ     (0x01 << BLK_NO_SHIFT)
/*BUFFER_SIZE is the power of the BUFFER_SHIFT*/
#define INVALID_BLK_NO  0xFF

#define INVALID_PTR     (INVALID_BLK_NO << BLK_NO_SHIFT)

void init_chn_pool_mgr(void);
int put_chn_bytes(struct _CHN_SLOT *pCHN_SLOT,unsigned char buffer[] ,int len);
int put_chn_byte(struct _CHN_SLOT *pCHN_SLOT,unsigned char c);
int get_chn_bytes(struct _CHN_SLOT *pCHN_SLOT,unsigned char buffer[] ,int len);
int get_chn_byte(struct _CHN_SLOT *pCHN_SLOT ,unsigned char buffer[]);
int peek_chn_byte(struct _CHN_SLOT *pCHN_SLOT,unsigned char data[],  int len);

#ifdef __cplusplus
 }
#endif

#endif
