
#ifndef __PROTCL_H__
#define __PROTCL_H__
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define COLD_WATER_METER 0x10 


enum FACOTR_TYPE
{
	FACOTR_NORMAL = 0,
	FACOTR_Point_1 = 1,// 0.1
};

enum PRO_188_ERROR
{
  ERROR_UNKNOWN =-1,

  /*CNN识别错误*/
  ERROR_CNN_VERTIFY=-10,
  ERROR_CNN_MIN_PROBILITY=-11,
  ERROR_FLASH_PARA_FORMAT=-12,

  /*硬件错误*/
  ERROR_CAMERA = -21,
  ERROR_NO_PARA=-22,
  
  /*消息格式错误*/
  ERROR_MESSAGE_FORMAT=-30,
  
  /*已经设置出厂模式*/
  ERROR_SETED_FACTORY_MODE = -40,	
	
  /*已经更新flash*/
  ERROR_SETED_UPDATE_FLASH = -50,
};

#define CMD_REPLY_BITS 0x80
#define CMD_ERROR_BITS 0xC0

enum
{
	CMD_188_READ = 0x01,
	CMD_188_READ_ADDRESS = 0x03,
    CMD_188_SET_ADDRESS = 0x15,
	CMD_188_WRITE = 0x04,
};

enum
{
    MOTOR_FORWARD,
    MOTOR_BACKWARD,
    MOTOR_STOP
};

enum DATA_DID
{
    /*读取用水量16H(22字节)*
       did 序号(3)
       当前累计流量(5) 单位(1) xxxxxx.xx(4 BCD码)
       结算日累计流量(5)
       实时时间(7)
       状态 ST(2)
       */
    DID_WATER_USAGE=0x901F,

    /*设置剪裁窗口
      start_x(2),width(2),start_y(2),height(2)
    */
    DID_CROP_WINDOWS=0xFF01,

    /*读取原始图像
    width(2),height(2),数据
    */
    DID_RAW_IMAGE_JPEG = 0xFF02,

   /*设置数字窗口参数:
      x00(2),y00(2), x01(2),y01(2), x10(2),y10(2), x11(2),y11(2), offset0(2),offset1(2),offset2(2),offset3(2),offset4(2)
    */
    DID_DIGIT_WINDOWS=0xFF03,

    /*读取拼接数字图像:*/
    DID_DIGIT_IMG=0xFF04,

    /*读取训练数据样品
      width(2),hegith(2),true_label(2),步进电机(4),light
    */
    DID_TRAIN_RAW_BMP=0xFF30,
    DID_TRAIN_SAMPLE=0xFF05,
        /*恢复出厂设置*/
    DID_FACTORY_RESET =0xEE01,
    /*montor action(1) 具体内容见枚举变量*/
    DID_MOTOR_CONTROL=0xFF07,

    DID_RAW_IMAGE_BMP= 0xFF08,
    
    /*a0-a7*/
    DID_WRITE_ADDRESS=0xA018, //write address
	DID_READ_ADDRESS=0x810A, //read address
	/*CNN计算测试*/
    DID_CNN_TEST = 0xFF09,

	/*进入命令行模式*/
    DID_MODE_CMD = 0xFF0A,

    /*设置高速波特率*/
    DID_HIGHSPEED_BAUD = 0xFF0B,

    /*设置灯光的明暗度*/
    DID_LIGHT_PWM = 0xFF0C,

    /*读取原始数据,返回10个字节  cnn1:data0-data4  cnn2:data0-data4*/
    DID_RAW_NUMBER = 0xFF0D,
	
    /*读取软件版本号和参数版本号*/
	//程序版本号（16 bytes）+参数版本号（12bytes）+ 1 个字节copyright
    DID_INFO_SUMMARY = 0xFF0E,
	
    /*set digit0 :最低位为  100*/
    DID_NUM_FACTOR = 0xFF0F,

    /* 1: 采用上次结果提高准确率  0:不用上次结果*/
    DID_USE_PREVIOUS_RESULT = 0xFF10,

    /*count,number*/
    DID_NUMBER_CHECK_MASK = 0xFF11,
    DID_DIRTY_DIGIT_MASK = 0xFF12,

    
    DID_LOW_BAUD = 0xFF13,

	/*设置指针裁剪中心和边长*/
	DID_POINTER_WINDOWS = 0XFF14,

	
    //DID_ADDRESS = 0x810A
	/*启用出厂设置*/
	DID_SET_OUT_FACTORY = 0xA019,
	/*设置更新flash*/
    DID_SET_FLAHS_INIT_DEFAUTL = 0xA020,
	
	/*读取芯片唯一ID*/
	//12位芯片ID
	DID_READ_MCU_ID = 0xEE02,
	
	/*设置芯片的正版性序号: 四个字节*/
	DID_SET_MUC_HASH = 0xEE03,

	/*切换高速波特率*/
    DID_SWITCH_BAUD = 0xEE04,

    DID_WATER_USAGE_SHOW =0xEE05,

    /*灯光范围*/
    DID_SET_LIGHT_RANGE = 0xDD01,

    /*读取bmp 565*/
    DID_BMP_565 = 0xEE07,  
	/*设置&&读取水表数字个数*/
	DID_DIGIT_COUNT = 0xBB00,
	/*设置&&读取水表数字宽度*/
	DID_DIGIT_WIDTH = 0xBB01,
	/*设置&&读取数字是否黑白翻转，0：不翻转，1：翻转*/
	DID_COLOR_REVERSAL = 0xBB02,

    DID_SEARCH_METER = 0xBB03,

    DID_SILENT_METER = 0xbb04,


};

enum PRO_188_UNIT
{
    UNIT_188_M3=0x2c
};

#define PROTO_188_ADDRESS_LEN   7
struct proto_188_type
{
   char head;
   unsigned char type;
   unsigned char address[PROTO_188_ADDRESS_LEN];
   unsigned char cmd;
   unsigned char len;
   unsigned char did[2];
   unsigned char data[1];//data【0】 seq
};
typedef struct  proto_188_type  proto_188_t;
typedef unsigned char uint8_t;

typedef struct proto_188_item_type
{
    unsigned short did;
	int (*read)(proto_188_t  *proto,uint8_t buff[],uint8_t arg_len, uint8_t max_len, uint16_t did); //读功能执行函数
    int (*write)(proto_188_t  *proto,uint8_t buff[],uint8_t w_len, uint16_t did);   //写功能执行函数
    int  dafault_task;
} proto_188_item_t;

int handle_proto_188_task(void);
int get_188_frame(unsigned char rxframe_raw[],int rxlen,int *call_my_addr,int *boardcast_my);
int get_my_frame(unsigned char rxframe_raw[],int rxlen);
void send_meter_usage_number(char  digit[6], unsigned char len);
void sync_switch_2400_baud(void);
void sync_switch_high_baud(void);
void read_dev_info_resp(void);
void send_eastsfot_error_code(int cmd,char code);
int handle_write_read_cmd(unsigned char data[]);
int send_raw_188_frame(unsigned char cmd,int did ,int length, unsigned char data[]);
int send_raw_reconginze_number(unsigned char buff[]);
int send_response_188_frame(unsigned char cmd,int did ,int length, unsigned char data[]);
#ifdef __cplusplus
  }
#endif

#endif
