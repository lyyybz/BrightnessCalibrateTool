#ifndef __UPDATE_H__
#define __UPDATE_H__
#include <stdint.h>
#include "update_proto.h"

/*
   升级概述
1. 升级格式流程
(1).应用程序接受升级文件，收到完整升级文件后，在flash中标记需要重启，reboot
(2).bootloader进行升级重启，升级过程中index序号回整，升级成功并校验crc成功后，执行应用程序
(3).应用程序查看是否需要对参数进行huffman解码，需要解码则进行解码。
(4).解码成功后进行识别。
2. 升级包格式详解
升级文件打包格式： #progrom_len(4), crc16(2), paras_len(4) crc16(2),program(index乱序),paras(哈夫曼编码,index乱序)
*/

enum
{
	UPDATE_BOARDCAST,
	UPDATE_PEER2PEER
};

enum
{
 CRC_NEED_PACKAGE=3,
 CRC_WRONG=0,
 CRC_NO_UPDATE =2,
};

#define UPDATE_MAX_TIMEOUT   60
#define UPDATE_MAGIC_VALUE  0x77

struct update_state_type
{
    uint16_t file_crc;
    unsigned short max_block_cnt;
    uint32_t file_sz; 
    uint8_t  block_sz;
    char mode;
    unsigned char state;
    unsigned short timeouts;
    unsigned short start_reviced_block;
    uint8_t  version[0x20];//此处软件版本号根据设备自身的长度修改
    struct block_manage_type *block_manager;    
};

typedef struct update_state_type  update_state_t;

void updater_start(unsigned short crc,
				     unsigned int file_sz,
				     unsigned char block_sz,
				     char version[],
				     char ver_len,
				     char mode);


/*
 @return: 
 	1 正常数据帧
	0 异常数据帧
*/	
unsigned char updater_receive_data(unsigned short crc,
				     unsigned char block_sz,
				     unsigned short seq,
				     unsigned char *data);


unsigned short updater_get_next_seq(void);

unsigned short updater_ge_lost_seq(unsigned short end,
                                             unsigned short seq[],
                                             unsigned short max_count);

unsigned short updater_get_receive_max_seq(void);
                                       
unsigned char  updater_check_file_crc(void);

void updater_receive_data_hook(void);


int is_update_finish(void);
int is_in_updateing(void);
void update_1s_tick(void);


#endif 
