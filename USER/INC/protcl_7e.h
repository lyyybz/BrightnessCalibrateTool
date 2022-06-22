
#ifndef __PROTCL_7E_H__
#define __PROTCL_7E_H__
#include <stdint.h>

#include "SEGGER_RTT.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LOG_FRAME  1

#define	CHN_PLC		    0x00
#define	CHN_SOFT		  0x00

#define log_msg(...)  SEGGER_RTT_printf(0, __VA_ARGS__)

enum
{
	CMD_READ = 0x02,
	CMD_WRITE = 0x07,
};

enum
{
	DID_TYPE = 0x0001,
	
	DID_SET_PANID = 0x0603,

	DID_SWITCH = 0xC012,

	DID_REPORT = 0xD005,

	/* C137 读写面板背光亮度百分比
	YY+XX+ZZ => YY:bit0~bit3分别表示通道信息，XX为按键断开亮度，ZZ为按键闭合亮度(0~100,16进制表示)
	eg: 读1、2通道亮度 36 C1 01 03， 回复 37 C1 05 03 05 64 05 65
	*/
	DID_BACKLIGHT_SET = 0xC137,	
	
	/*测试DID 指定通道按照指定亮度显示
	YY + XX
	eg: 10 FF 02 01 64 表示第一通道100%亮度显示*/ 
	DID_BACKLIGHT_TEST = 0xFF10,
	
};

void scan_soft_uart_channel(void);
int get_soft_uart_frame(unsigned char rxframe_raw[], int rxlen);
void handle_soft_uart_frame(unsigned char frame[]);

void scan_plc_channel(void);
int get_7e_frame(unsigned char rxframe_raw[], int rxlen);
void handle_7e_frame(unsigned char frame[], int frame_len);

int send_7e_local_frame(int did, int length, unsigned char data[]);
int send_7e_app_frame(unsigned char cmd, int did, int length, unsigned char data[]);
int send_7e_response_frame(unsigned char cmd, int did, int length, unsigned char data[]);

void local_setting(void);
void clear_device_panid(void);
void read_device_type(void);
void read_report_mode(void);
void set_report_mode(void);
void close_all_channel(void);
void open_all_channel();
void open_channel(int channel);
void set_backlight(int channel, int bicklight);
void show_backlight(int channel, int backlight);

void task_end(int is_succ);

int capture_bmp_info(void);
void correct_brightness(void);

int computeOtsu(int histogram[]);	// 大津算法计算阈值
	
void send_frame_to_pc(unsigned char frame[], int len);

void log_frame(unsigned char title[], unsigned char frame[], int frame_len);
void log_frame2(unsigned char title[], unsigned char frame[], int frame_len);
void log_frame3(unsigned char title[], int frame[], int frame_len);
void log_frame4(unsigned char title[], unsigned char frame[], int frame_len);

#ifdef __cplusplus
  }
#endif

#endif
