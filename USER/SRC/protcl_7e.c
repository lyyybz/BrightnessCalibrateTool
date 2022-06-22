#include <stdio.h>
#include <stm32f10x.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <assert.h>
#include "config.h"
#include "comfunc.h"
#include "uart_fifo.h"
#include "main.h"
#include "camera.h"
#include "recognize.h"
#include "hardware_layer.h"
#include "usart.h"
#include "update.h"
#include "bmp_layer.h"
#include "update.h"
#include "hash.h"
#include "comfunc.h"
#include "protcl_7e.h"
#include "proto_camera.h"
#include "SEGGER_RTT.h"
int said = 1;
int taid = 839807;	// 两按键：839807 23688, 三按键：800737 1530, 四按键：320847 34504  
int pwd = 23688;    // 设备密码，用于清空panid
int channel_count = 0;      // 设备通道总数
int current_channel = 0;    // 当前校准通道
int target_backlight = 195;
int target_backlight_min = 180;
int target_backlight_max = 210;
int set_backlight_value = 100;
int error = 3;
int steep = 3;
int is_transparent = 0; // 透传
int isJumpCurrentButton = 0; // 设置到极限（50或者100）仍无法满足设置亮度，跳过当前按钮

void scan_soft_uart_channel() {
    // 从串口读取报文
    uint8_t buffer[0x100];
    int idx;
    /*从串口缓冲区中读数据*/
    int peek_len = my_uart_peek_data(CHN_SOFT, buffer, sizeof(buffer));
    /*寻找完整帧*/
		if(peek_len < 9)
		{
			return;
		}
    idx = get_soft_uart_frame(buffer, peek_len);
    if (idx >= 0)
    {
        my_uart_read(CHN_SOFT, buffer, idx);
        my_uart_read(CHN_SOFT, buffer, 9);
        /*处理完整帧*/
        handle_soft_uart_frame(buffer);
    }
		// 可支持透传7e报文
		idx = get_7e_frame(buffer, peek_len);
		if (idx >= 0){
				int flen = buffer[idx + 10] + 11 + 1;
        my_uart_read(CHN_SOFT, buffer, idx);
        my_uart_read(CHN_SOFT, buffer, flen);
        /*转发*/
				log_frame("we recv transparent frame", buffer, flen);
				is_transparent = 1;
        sys_uart_sync_write(0, buffer, flen);
		}
}

// 77 7F D0 0C 00 88 5C C8 7E  // 77 + 4字节aid（小端） + 2字节pwd（小端） +  1字节背光亮度 + CS校验和
// 77 E1 37 0C 00 FA 05 BE 58
// 77 4F E5 04 00 C8 86 BE BB
int get_soft_uart_frame(unsigned char rxframe_raw[], int rxlen) {
    int  i = 0;
    uint8_t cs;
    int framelen;

    if (rxlen < 9) return(-1);

check_frame:

    while (i < rxlen)
    {
        if (0x77 == rxframe_raw[i])//77 head
            break;
        i++;
    }
    if (i >= rxlen) return(-1);//if not 77 head return -1
    framelen = rxlen - i;// frame len

    if (framelen < 9) return(-1);

    cs = checksum(rxframe_raw + i, 8);
    if (rxframe_raw[i + 8] != cs)
    {
        i++;
        goto check_frame;
    }
    return(i);
}

void handle_soft_uart_frame(unsigned char frame[]) {
    // 77 7F D0 0C 00 88 5C 64 1A  // 77 + 4字节aid（小端） + 2字节pwd（小端） +  1字节背光亮度 + CS校验和
		log_frame("we recv pc frame", frame, 9);
    unsigned char aid[4];
    memcpy(aid, frame + 1, sizeof(aid));
    taid = (aid[3] << 24) + (aid[2] << 16) + (aid[1] << 8) + aid[0];
    unsigned char password[2];
    memcpy(password, frame + 5, sizeof(password));
    pwd = (password[1] << 8) + password[0];
    int backlight = frame[7] & 0xff;
		if(backlight > target_backlight_max){
				backlight = target_backlight_max;
		}else if(backlight < target_backlight_min){
				backlight = target_backlight_min;
		}
		target_backlight = backlight;
		// 根据设置范围，计算一个较为接近的设置值
		set_backlight_value = (target_backlight-target_backlight_min)*(100-50)/(target_backlight_max-target_backlight_min) + 50;
		log_msg("aid[%d], pwd[%d], backlight[%d], target_backlight[%d], value[%d]\n", taid, pwd, target_backlight, target_backlight, set_backlight_value);
		
    // 开始校准
		current_channel = 0;
		isJumpCurrentButton = 0;
    flash_led_start();
		int sleep_time = 5000-(50 * is_device_ok());
		log_msg("sleep time %d\n", sleep_time);
		if(sleep_time > 0){
				delay_ms(sleep_time); // 被测设备载波上电初始化时间
		}
		clear_device_panid(); // 0603没有回复报文
		delay_ms(300); 
		read_device_type();
}

int peek_len;
int idx, framelen;

void scan_plc_channel() {
    // 从串口读取报文
    uint8_t buffer[0x100];
    /*从串口缓冲区中读数据*/
    peek_len = sys_uart_peek_data(CHN_PLC, buffer, sizeof(buffer));
    /*寻找完整帧*/
    idx = get_7e_frame(buffer, peek_len);
    if (idx >= 0)
    {
        framelen = buffer[idx + 10] + 11 + 1;
        sys_uart_read(CHN_PLC, buffer, idx);
        sys_uart_read(CHN_PLC, buffer, framelen);
        /*处理完整帧*/
				plc_time_count_stop();
        handle_7e_frame(buffer, framelen);
    }
		int timeout = is_plc_timeout();
		if(timeout > 20){
				// 50ms * 20 = 1s 载波超时结束
				plc_time_count_stop();
				task_end(0); 
		}
}

int get_7e_frame(unsigned char rxframe_raw[], int rxlen)
{
    int  i = 0, datalen;
    uint8_t cs;
    int framelen;

    if (rxlen < 12) return(-1);

check_frame:

    while (i < rxlen)
    {
        if (0x7E == rxframe_raw[i])//7E head
            break;
        i++;
    }
    if (i >= rxlen) return(-1);//if not 7E head return -1
    framelen = rxlen - i;// frame len

    datalen = rxframe_raw[i + 10];//datalen
    if (datalen + 11 + 1 > framelen)
    {
        i++;
        goto check_frame;
    }
    cs = checksum(rxframe_raw + i, datalen + 11);
    if (rxframe_raw[i + 11 + datalen] != cs)
    {
        i++;
        goto check_frame;
    }
    return(i);
}

// 解析7E报文
void handle_7e_frame(unsigned char frame[], int frame_len)
{
    /*起始符 源地址   目的地址  帧序号  长度     帧体      算术和
     STC     SAID      TAID      FSEQ    LEN      FBD       SUM
     7E     00000000  00000000   00      05    01 78563412   98

     本地通信协议，原地址和目的地址为0

     */
    log_frame("we recv new frame", frame, frame_len);
	
		if(is_transparent){
				log_msg("this is a transparent frame, send to pc.\n");
				is_transparent = 0;
				send_frame_to_pc(frame, frame_len);
				return;
		}
	
    unsigned char sa[4];
    unsigned char ta[4];
    memcpy(sa, frame + 1, 4);
    memcpy(ta, frame + 5, 4);
    int len = frame[10] & 0xff;
    unsigned char fbd[0x20];
    memcpy(fbd, frame + 11, len);

    // 本地报文处理
    if ((sa[0] & 0xff) == 0 && (sa[0] & 0xff) == 0 && (sa[0] & 0xff) == 0 && (sa[0] & 0xff) == 0
        && (ta[0] & 0xff) == 0 && (ta[0] & 0xff) == 0 && (ta[0] & 0xff) == 0 && (ta[0] & 0xff) == 0)
    {
        if ((frame[11] & 0xff) == 0) {
            // 设置成功 7E 00 00 00 00 00 00 00 00 00 01 00 7F（首次设置或者设置不同的 AID 时）
        }
        else if ((frame[11] & 0xff) == 0xff) {
            // 设置失败 7E 00 00 00 00 00 00 00 00 00 01 FF 7E（设置相同的 AID 或者报文发送错误时）
        }
//				delay_ms(5000); // 被测设备载波上电初始化时间
//				clear_device_panid(); // 0603没有回复报文
//				delay_ms(300); 
//				read_device_type();
        return;
    }

    // 被测设备回复报文   // FBD : CMD DID CTRL DATA  1+2+1+n
		int cmd = fbd[0] & 0xff;
		if(cmd != CMD_WRITE && cmd != CMD_READ){
			// 设备上报报文不做处理 
				return;
		}
    int did = (fbd[2] << 8) + (fbd[1] & 0xff);
    int datalen = fbd[3] & 0x7f;
    unsigned char data[0x20];
    memcpy(data, fbd + 4, datalen);
    switch (did) {
        case DID_TYPE:
            // 根据设备类型确定设备的通道数
            if (data[6] == 0x01)
            {
                channel_count = 1;
            }else  if (data[6] == 0x02)
            {
                channel_count = 2;
            }
            else if (data[6] == 0x03)
            {
                channel_count = 3;
            }
            else if (data[6] == 0x04)
            {
                channel_count = 4;
            }
						log_msg("channel_count[%d]\n", channel_count);
            // 关闭上报
            set_report_mode();
            break;
        case DID_REPORT:
            // 关闭所有通道
            close_all_channel();
            break;
        case DID_SWITCH:
						// 关闭所有通道，面板回复0004错误（数据项不存在）
            if (data[0] == 0x00 || (data[0] == 0x04 && data[1] == 0x00)) {
                // 关灯回复，开启通道
                set_backlight(current_channel, set_backlight_value);
            }
            break;
        case DID_BACKLIGHT_SET:
						// 设置成功后，点亮测试
						show_backlight(current_channel, set_backlight_value);
						break;
        case DID_BACKLIGHT_TEST:
            // 亮度设置回复，检查亮度
						// data 01 00 ,通道一关闭
						if (data[1] != 0x00)
						{
								delay_ms(100);
								correct_brightness();
						}else{
								// 熄灯，进行下一个通道
								current_channel++;
								if (current_channel >= channel_count) {
										// 当前已经是最后一个通道了,全部校准完毕
										task_end(1);
										return;
								}
								set_backlight(current_channel, set_backlight_value);
						}
            break;
    }
}

void task_end(int is_succ){
		// 停止闪灯
		flash_led_stop();
		// 根据结果点灯或灭灯
		//set_led1(1);
		unsigned char site[9] = {0x77, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x77};
		if(is_succ){
				site[7] = 0x00;
		}else{
				site[7] = 0x01;
		}
		send_frame_to_pc(site, 9);
}

static int sno = 0;
int getSno(void) {
    sno++;
    if (sno > 0xff) {
        sno = 0;
    }
    return sno;
}

int send_7e_local_frame(int did, int length, unsigned char data[])
{
    unsigned char buff[0x20];
    int idx = 0;

    buff[idx++] = 0x7E;

    buff[idx++] = 0x00;
    buff[idx++] = 0x00;
    buff[idx++] = 0x00;
    buff[idx++] = 0x00;

    buff[idx++] = 0x00;
    buff[idx++] = 0x00;
    buff[idx++] = 0x00;
    buff[idx++] = 0x00;

    buff[idx++] = getSno() & 0xff; // sno
    buff[idx++] = 1 + length; // len (did + data.length)

    buff[idx++] = did & 0xff;

    memcpy(buff + idx, data, length);
    idx += length;

    buff[idx++] = checksum(buff, 11 + 1 + length);

    sys_uart_sync_write(0, buff, idx);

    log_frame("we send new frame", buff, idx);
		
		plc_time_count_start();
		
    return idx;
}

int send_7e_app_frame(unsigned char cmd, int did, int length, unsigned char data[])
{
    unsigned char buff[0x20];
    int idx = 0;

    buff[idx++] = 0x7E;

    buff[idx++] = said & 0xFF;
    buff[idx++] = said >> 8;
    buff[idx++] = said >> 16;
    buff[idx++] = said >> 24;

    buff[idx++] = taid & 0xFF;
    buff[idx++] = taid >> 8;
    buff[idx++] = taid >> 16;
    buff[idx++] = taid >> 24;

    buff[idx++] = getSno() & 0xff; // sno
    buff[idx++] = 4 + length; // len

    buff[idx++] = cmd; // cmd

    buff[idx++] = did & 0xff;
    buff[idx++] = did >> 8;

    if (cmd == CMD_READ) {
        buff[idx++] = length;
    }
    else {
        buff[idx++] = length;
        memcpy(buff + idx, data, length);
        idx += length;
    }

    buff[idx++] = checksum(buff, 11 + 4 + length);

    sys_uart_sync_write(0, buff, idx);

    log_frame("we send new frame", buff, idx);
		
		plc_time_count_start();
    
		return idx;
}

int send_7e_response_frame(unsigned char cmd, int did, int length, unsigned char data[])
{
    return send_7e_app_frame(cmd | 0x80, did, length, data);
}

void local_setting() {
    // 设置本地载波芯片地址   
    //	发送 ： 7E 00 00 00 00 00 00 00 00 00 05 01 01 00 00 00 85 
    //  应答 ：	7E 00 00 00 00 00 00 00 00 00 01 00 7F（首次设置或者设置不同的 AID 时）
    //			7E 00 00 00 00 00 00 00 00 00 01 FF 7E（设置相同的 AID 或者报文发送错误时）
    unsigned char data[] = { 0x01, 0x00, 0x00, 0x00 };
    send_7e_local_frame(0x01, sizeof(data), data);
}

void clear_device_panid() {
	  // 清空设备panid（设置为0）
		// 7E 01 00 00 00 7F D0 0C 00 04 12 07 03 06 0E 7F D0 0C 00 00 00 88 5C 01 00 00 00 01 00 4F
		// data : 4aid+2panid+2pw+4gid+2sid   7F D0 0C 00 00 00 88 5C 01 00 00 00 01 00
	  unsigned char data[14];  
		int idx = 0;

    data[idx++] = taid & 0xFF;
    data[idx++] = taid >> 8;
    data[idx++] = taid >> 16;
    data[idx++] = taid >> 24;
	
    data[idx++] = 0x00;
    data[idx++] = 0x00;
	
	  data[idx++] = pwd & 0xFF;
    data[idx++] = pwd >> 8;
	
    data[idx++] = said & 0xFF;
    data[idx++] = said >> 8;
    data[idx++] = said >> 16;
    data[idx++] = said >> 24;
		
    data[idx++] = 0x01;
    data[idx++] = 0x00;
	
    send_7e_app_frame(CMD_WRITE, DID_SET_PANID, sizeof(data), data);
}

void read_device_type() {
    //	发送 ： 7E 01 00 00 00 00 00 00 00 01 05 02 01 00 00 FC 
    //  应答 ：	7E 00 00 00 00 01 00 00 00 81 09 02 01 00 FF FF 18 00 00 00 E4
    unsigned char data[1];
    send_7e_app_frame(CMD_READ, DID_TYPE, 0, data);
}

void read_report_mode() {
    //	发送 ： 7E 01 00 00 00 00 00 00 00 01 05 07 05 D1 02 01 00 FC 
    //  应答 ：	
    unsigned char data[1];
    send_7e_app_frame(CMD_READ, DID_REPORT, 0, data);
}

void set_report_mode() {
    //	发送 ： 7E 01 00 00 00 00 00 00 00 01 05 07 05 D1 02 01 00 FC 
    //  应答 ：	
    unsigned char data[2];
    data[0] = 0x00; // 传感器类型
    data[1] = 0x00; // 00 不上报，01 上报网关
    send_7e_app_frame(CMD_WRITE, DID_REPORT, 2, data);
}

void close_all_channel() {
    //	发送 ： 7E 01 00 00 00 00 00 00 00 01 05 07 12 C0 01 0F FC 
    //  应答 ：	7E 00 00 00 00 01 00 00 00 81 03 12 C0 00 E4
    unsigned char data[] = { 0x0F }; // 关闭1234通道
    send_7e_app_frame(CMD_WRITE, DID_SWITCH, 1, data);
}

void open_all_channel() {
    //	发送 ： 7E 01 00 00 00 00 00 00 00 01 05 07 12 C0 01 0F FC 
    //  应答 ：	7E 00 00 00 00 01 00 00 00 81 03 12 C0 00 E4
    unsigned char data[] = { 0x8F }; // 打开1234通道
    send_7e_app_frame(CMD_WRITE, DID_SWITCH, 1, data);
}

void open_channel(int channel) {
    //	发送 ： 7E 01 00 00 00 00 00 00 00 01 05 07 12 C0 01 81 FC 
    //  应答 ：
    unsigned char data[1];
    data[0] = 0x80 | (0x01 << channel);
    send_7e_app_frame(CMD_WRITE, DID_SWITCH, 1, data);
}

void set_backlight(int channel, int backlight) {
    //  发送 ： 7E 01 00 00 00 00 00 00 00 01 05 07 36 C1 03 01 05 64 FC 
    //  应答 ：
    unsigned char data[3];
    data[0] = 0x01 << channel;
    data[1] = 0x05; // 关闭状态下的背光
    data[2] = backlight & 0xFF; // 开启状态下的背光
    send_7e_app_frame(CMD_WRITE, DID_BACKLIGHT_SET, 3, data);
}

void show_backlight(int channel, int backlight) {
    unsigned char data[2];
    switch (channel) {
    case 0:
        data[0] = 0x01;
        break;
    case 1:
        data[0] = 0x02;
        break;
    case 2:
        data[0] = 0x03;
        break;
    case 3:
        data[0] = 0x04;
        break;
    }
    data[1] = backlight & 0xFF; // 亮度
    send_7e_app_frame(CMD_WRITE, DID_BACKLIGHT_TEST, 2, data);
}

void SPI1_SendByte(uint8_t byte);
int ss[256];
int capture_bmp_info(void)
{
    int width = 320;
    int height = 240;
		memset(ss, 0, sizeof(ss));
    unsigned char buff[WIN_DEFAULT_CAPUTE_WIDTH] = { 0 };
    int i = 0;
    camera_start_capture_rect(0, width, 0, height);
    for (i = 0; i < height; i++)
    {
        get_a_line_from_fifo(i, buff);	
//				log_frame4("", buff, 320);	
//			  delay_ms(3);
        int j = 0; 	 
        for (j = 0; j < width; j++)
        {
            uint8_t tmp = buff[j];
            ss[tmp]++;
						//SPI1_SendByte(buff[j]);
        }
				//SPI1_SendByte(0);
    } 

		// 灰度直方图
    //log_frame3("", ss, 256);

		// 计算平均亮度
		int a = computeOtsu(ss);
		
    int count = 0;
    int sum = 0;
    int mincount = 0;
		int minsum = 0;
		int k = 0;
    for (k = a; k < 256; k++) {
        sum += k * ss[k];
        count += ss[k];
    }
		int average = sum / count;  
		
//		for (k = 0; k < a; k++) {
//        minsum += k * ss[k];
//        mincount += ss[k];
//    }
//		int minaverage = minsum / mincount; 
		
		log_msg("channel: [%d], value: [%3d], otsuThreshold: [%3d], sum: [%6d], count: [%4d], average: [%d]\n", current_channel, set_backlight_value, a, sum, count, average);
    camera_frame_end_callback();
    return average;
}

void correct_brightness() {
    // 获取当前亮度
    int brightness = capture_bmp_info();
		if(isJumpCurrentButton || abs(brightness - target_backlight) <= error)
		{
				show_backlight(current_channel, 0);		//熄灭当前通道，进行下一通道
		}else{
				// 调整设置亮度值
				int diff = target_backlight - brightness;
				if(target_backlight < brightness){
						set_backlight_value -= steep;
				}else{
						set_backlight_value += steep;
				}
				set_backlight_value = set_backlight_value  > 100 ? 100 : set_backlight_value;
				set_backlight_value = set_backlight_value  < 50 ? 50 : set_backlight_value;
				if(set_backlight_value == 50 || set_backlight_value == 100){
					isJumpCurrentButton = 1;
				}else{
					isJumpCurrentButton = 0;
				}
				set_backlight(current_channel, set_backlight_value);
		}
			
			// 测试代码
//		set_backlight_value -= 5;
//		if(set_backlight_value < 50){
//			current_channel++;
//			log_msg("\ncurrent_channel : %d\n", current_channel);
//			if (current_channel >= channel_count) {
//				set_backlight_value = 100;
//				current_channel = 0;
//				return;
//			}
//			set_backlight_value = 100;
//		}
}

int computeOtsu(int histogram[])
{
    // histogram[256]; //存储灰度直方图，这里为256色灰度
    int width = 320;   //图像宽度
    int height = 240;  //图像长度
    int sumW = 0, sumb = 0, sumf = 0;
    int pos = 0;
    int thred = 0;
    short x;
    float wf = 0.0, wb = 0.0, ub, uf;
    float curVal = 0.0, maxVal = 0.0;
    int sumPixel = height * width;
    int i, j;

    for(i=0; i<256; i++)
    {
        sumW += i * histogram[i];
    }

    //枚举每个灰度
    for(i=0; i<256; i++)
    {
        //求两类类概率密度
        wb += histogram[i];
        wf = sumPixel - wb;

        if(wb==0 || wf==0)
        {
            continue;
        }

        //求类均值
        sumb += i * histogram[i];
        sumf = sumW - sumb;
        ub = sumb / wb;
        uf = sumf / wf;

        //求当前类间方差
        curVal = wb*wf*(ub-uf)*(ub-uf);
        if(curVal > maxVal)
        {
            thred = i;
            maxVal = curVal;
        }
    }
    return thred;
}

void send_frame_to_pc(unsigned char frame[], int len) {
    my_uart_write(frame, len);
		log_frame("we send frame to pc ", frame, len);
}

void log_frame(unsigned char title[], unsigned char frame[], int frame_len) {
#if LOG_FRAME
		int i;
    SEGGER_RTT_printf(0, "%s[ ", title);
    for (i = 0; i < frame_len; i++) {
        SEGGER_RTT_printf(0, "%02x ", frame[i]);
    }
    SEGGER_RTT_printf(0, "]\n");
		
		//send_frame_to_pc(frame, frame_len);
#endif
}

void log_frame2(unsigned char title[], unsigned char frame[], int frame_len) {
    int i;
    for (i = 0; i < frame_len; i++) {
			SEGGER_RTT_printf(0, "%02x ", frame[i]);
    }
    SEGGER_RTT_printf(0, "\n");
}

void log_frame3(unsigned char title[], int frame[], int frame_len) {
    int i;
    for (i = 0; i < frame_len; i++) {
			//SEGGER_RTT_printf(0, "[%d]:%d,", i, frame[i]);
			SEGGER_RTT_printf(0, "%d,", frame[i]);
    }
    SEGGER_RTT_printf(0, "\n");
}

void log_frame4(unsigned char title[], unsigned char frame[], int frame_len) {
    int i;
    for (i = 0; i < frame_len; i++) {
			//SEGGER_RTT_printf(0, "[%d]:%d,", i, frame[i]);
			SEGGER_RTT_printf(0, "%d,", frame[i]&0xff);
    }
    SEGGER_RTT_printf(0, "\n");
}

