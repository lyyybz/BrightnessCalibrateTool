#include "config.h"
#include "proto_camera.h"
#include "protcl.h"
#include "usart.h"
#include "recognize.h"
#include "comfunc.h"
#include "main.h"
#include "camera.h"
#include "device_info.h"
#include "motor.h"
#include "hardware_layer.h"
#include "stdlib.h"
#include "JPGencode.h"


//都是小端
unsigned char send_header(unsigned char  cmd,int len)
{
    unsigned char sum =0;

    sync_switch_high_baud();

    USART1_Transmit(0xfe);
    USART1_Transmit(0xfe);
    USART1_Transmit(2);

    USART1_Transmit(cmd);
    sum += cmd;

    USART1_Transmit(len);
    sum += len&0xff;
    USART1_Transmit(len>>8);
    sum += (len>>8)&0xff;
    USART1_Transmit(len>>16);
    sum += (len>>16)&0xff;
    USART1_Transmit(len>>24);
    sum += (len>>24)&0xff;

    return sum;

}

unsigned char send_body(int len,unsigned char buff[],unsigned char sum)
{
    int i=0;

    for(i =0 ; i < len; i++)
    {
        USART1_Transmit(buff[i]);
        sum += buff[i];
    }
    return sum;
}

void send_tail(unsigned char  sum)
{
    //send end magic
    USART1_Transmit(sum);
    USART1_Transmit(3);

    sync_switch_2400_baud();
}



void  send_bmp_in_ram(int width,int height,unsigned char *buff)
{
    int i;
    unsigned char sum = 0;

    sum = send_header(IMG_CMD_DIGIT_BMP,height*width+4);
    sum = send_body(2,(unsigned char *)&width,sum);
    sum = send_body(2,(unsigned char *)&height,sum);
    for (i = 0 ; i < height*width ; i++)
    {
        sum += buff[i];
        USART1_Transmit(buff[i]);
    }

    send_tail(sum);

}

void send_jpeg_in_flash(int cmd,int start,int len)
{
    unsigned char sum = 0;
    int read_length;
    int read_addr;
    unsigned char line[CAMERA_MAX_LINE_BUFF];

    sum = send_header(cmd,len);
    read_addr = start;
    while(len >0 )
    {
        read_length = min(sizeof(line),len);
        dataflash_read(read_addr,line,read_length);
        sum = send_body(read_length,line,sum);
        len -= read_length;
        read_addr += read_length;
    }

    send_tail(sum);
}

void send_bmp_in_flash(int cmd,int start,int height,int width)
{
    unsigned char sum = 0;
    int read_length;
    int read_addr;
    unsigned char line[CAMERA_MAX_LINE_BUFF];
    int len = height*width;

    sum = send_header(cmd,height*width+4);
    sum = send_body(2,(unsigned char *)&width,sum);
    sum = send_body(2,(unsigned char *)&height,sum);
    read_addr = start;
    while(len >0 )
    {
        read_length = min(sizeof(line),len);
        dataflash_read(read_addr,line,read_length);
        sum = send_body(read_length,line,sum);
        len -= read_length;
        read_addr += read_length;
    }

    send_tail(sum);
}


void send_digit_sample_in_flash(int cmd,int start,struct DEV_INFO *dev)
{

    unsigned char sum = 0;
    int read_length;
    int read_addr;
    unsigned char line[CAMERA_MAX_LINE_BUFF];
    short width = DIGIT_WIDTH*(devinfo.digit_count);
    short height = DIGIT_HEIGHT;
    short light = dev->light_pwm;
    unsigned int steps = dev->steps/STEP_UINITS;

    int len = height*width;

    sum = send_header(cmd,height*width+19);
    sum = send_body(2,(unsigned char *)&width,sum);
    sum = send_body(2,(unsigned char *)&height,sum);
    sum = send_body(7,devinfo.devid,sum);
    sum = send_body(2,(unsigned char *)&devinfo.batch,sum);
    sum = send_body(4,(unsigned char *)&steps,sum);
    sum = send_body(2,(unsigned char *)&light,sum);
    read_addr = start;
    while(len >0 )
    {
        read_length = min(sizeof(line),height*width);
        dataflash_read(read_addr,line,read_length);
        sum = send_body(read_length,line,sum);
        len -= read_length;
        read_addr += read_length;
    }

    send_tail(sum);
}


/*发送jpeg图像*/
int capture_jpeg_to_flash(int left,int right,int top,int bottom)
{
    int len;
    camera_start_capture_rect(left,right,top,bottom);

    camera_frame_end_callback();
    return len;
}


/*发送bmp图像*/
void capture_bmp_to_flash(int left,int right,int top,int bottom)
{
    short width = right - left;
    short height = bottom - top;

    unsigned char buff[WIN_DEFAULT_CAPUTE_WIDTH]= {0};
    int i =0;

    dataflash_erase(RECGONIZE_IMAGE_ADDR,SIZE_1K*75);

    camera_start_capture_rect(left,right,top,bottom);

    for (i=0 ; i<height ; i++)
    {
        get_a_line_from_fifo(i,buff);
        dataflash_write(RECGONIZE_IMAGE_ADDR+i*width,buff,width);
    }
    camera_frame_end_callback();
}


double capture_bmp_to_calculate_brightness(void)
{
    int width = 320;
    int height = 240;
    int ss[256];
    unsigned char buff[WIN_DEFAULT_CAPUTE_WIDTH] = { 0 };
    int i = 0;
    camera_start_capture_rect(0, width, 0, height);
    for (i = 0; i < height; i++)
    {
        get_a_line_from_fifo(i, buff);
        int j = 0;
        for (j = 0; j < width; j++)
        {
            int tmp = buff[j];
            ss[tmp]++;
        }
    }
    int k = 0;
    int sum = 0;
    for (k = 0; k < 256; k++) {
        sum += k * ss[k];
    }
    double first_average = sum / 256;
    int count = 0;
    sum = 0;
    for (k = 0; k < 256; k++) {
        if (ss[k] > first_average)
        {
            sum += k * ss[k];
            count++;
        }
    }
    double double_average = sum / count;
    camera_frame_end_callback();
    return double_average;
}

