#ifndef __UPDATE_PROTO_H__
#define __UPDATE_PROTO_H__

#include <stdint.h>

#ifdef __cplusplus
 extern "C" {
#endif

#define little_bytes_to_uint16_t(byte)  ((uint16_t)((byte)[1]<<8)+(byte)[0])
#define uint16_t_to_little_bytes(word,buff)  (buff)[0]=word&0xFF; (buff)[1]=(word>>8);
#define offset_of(obj_type,mb)  ((unsigned int)&(((obj_type*)0)->mb))

#ifndef NULL 
	#define NULL            (void *)0
#endif 

#define   MAGIC_NUM          0xAA552B2B

#define     CMD_P2P_UPDATE      0x05
#define     CMD_BOARDCAST_UPDATE  0x06


enum
{
	 BOARDCAST_START=0,
	 BOARDCAST_DATA=1,
	 BOARDCAST_QUARY_LOST=2,
	 BOARDCAST_QUARY_RESTART=3
};

#define     STC             0x7e
#define     ADDRESS_LEN     0x04
#define     ID_LEN     0x04


struct SHS_frame
{
    uint8_t  stc;
    uint8_t  said[ADDRESS_LEN];
    uint8_t  taid[ADDRESS_LEN];
    uint8_t  seq;
    uint8_t  length;
    uint8_t  infor[1];
};

#define SHS_FRAME_HEAD       offset_of(struct SHS_frame, infor)
	
struct update_head_type
{
    uint8_t  seq[2];
    uint8_t  ack;
    uint8_t  crc[2];
    uint8_t  len;
    uint8_t  data[1];
};


struct UPDATE_FILE
{
    uint16_t file_sz[2];
    uint8_t  crc[2];
    uint8_t  blk_sz;
    uint8_t  type[8];
    uint8_t  data[1];
};

int handle_update_task(void);
struct SHS_frame *get_smart_frame(unsigned char  rxframe_raw[],unsigned char   rxlen, int *clr_len);

uint16_t sp_crc16_with_init(uint16_t crc, const uint8_t *buf, uint8_t size);


#ifdef __cplusplus
 extern "C" {
#endif

#endif


