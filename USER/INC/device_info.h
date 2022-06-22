#ifndef __DEVICE_INFO_H__
#define __DEVICE_INFO_H__

#ifdef __cplusplus
extern "C" {
#endif

	
#include "config.h"
#include "bmp_layer.h"
	
#ifdef  ESSN_WCR_A
	#define  VERSION_PREFIX  "ESSN-WCR-A(v1.6)-Para-"
	/*第一版主程序*/
	#if CONFIG_DEBUG
	#define PROGRAM_VERSION_1  "ESSN-DBG-A(v1.6)-20180910"
	#else
	#define PROGRAM_VERSION_1  "ESSN-WCR-A(v2.0)-10081229"
	#endif
#endif	
	
#ifdef  ESSN_WCR_B
	#define  VERSION_PREFIX  "ESSN-WCR-B(v1.0)-Para-"

	/*第一版主程序*/
	#if CONFIG_DEBUG
	#define PROGRAM_VERSION_1  "ESSN-DBG-B(v1.0)-20181218"
	#else
	#define PROGRAM_VERSION_1  "ESSN-WCR-B(v1.0)-20181229"
	#endif
#endif

#ifdef  ESSN_WCR_P
	#define  VERSION_PREFIX  "ESSN-WCR-P(v1.1)-Para-"

	/*第一版主程序*/
	#if CONFIG_DEBUG
	#define PROGRAM_VERSION_1  "ESSN-DBG-P(v1.1)-20190119"
	#else
	#define PROGRAM_VERSION_1  "ESSN-WCR-P(v1.1)-20190119"
	#endif
#endif
	
#define DEFAULT_HIGHSPEED_BAUD     (9600)

#define WORK_NORMAL_MAX_CNT    (30)


#define  FLAG_FACTTORY_OUT        (0)
#define  FLAG_PREVIOUS_RESULT_VALID     (1)
#define  FLAG_NO_USE_PRE_RESULT  (2)
#define  MAX_USE_CNT  (20)

#define  DIGNT_MASK_MAX_COUNT  8

#pragma anon_unions

struct digit_info
{
	quadrange_t  digital_quadrange;
	short digit_offset[MAX_DIGIT_CNT];
};

struct pointer_info
{
	point_t  center [MAX_POINTER_COUNT];
	short	 pointer_width;//指针的宽度
};

struct DEV_INFO
{
    int steps;
    struct TRECT  crop_rect;
    struct TRECT digital_rect;
	
	union
	{
		struct digit_info   di;
		struct pointer_info  pi;
	};
	rorate_context_t rorate_context;

    unsigned short highspeed_baud;
    unsigned short batch;
    unsigned char devid[7];
    unsigned char light_pwm;

    unsigned char flags;
    signed char num_factor;
    unsigned char use_cnt;

    unsigned int mcu_id_hash;
    unsigned int work_normal_cnt;

    unsigned char check_mask_bitmap[13]; //加强校验的数字的mask
    unsigned char digit_count;
    unsigned char digit_width;
    unsigned int  magic_update_flash;
    unsigned char color_reverse;
    unsigned char previos_digit[MAX_DIGIT_CNT];
    unsigned char dirty_digit_mask; // 脏表数字的mask
    unsigned char parameter_unpack; //标记参数升级的阶段
    unsigned char pad;
    unsigned short low_baud;
    
    unsigned char extra_data[35];
    unsigned int magic_version;
};

#pragma pack(1)
struct OLD_DEV_INFO
{
    int steps;
    unsigned char high_digit[4];
    /*从摄像头的视野中捕捉矩形 T1 暂时不支持设置*/
    struct TRECT  crop_rect;
    /*从T1矩形中捕捉数字矩形*/
    struct TRECT digital_rect;
    quadrange_t  digital_quadrange; //四边形 四个点
    short digit_offset[5]; //偏移量
    rorate_context_t rorate_context;

    unsigned short highspeed_baud;
    unsigned short batch;
    /*表地址*/
    unsigned char devid[7];
    unsigned char light_pwm; //灯光

    unsigned char flags;
    /*必须保证程序4个字节对齐，否则运行不正常*/
    signed char num_factor;
    unsigned char low_digit;
    unsigned char use_cnt;

    unsigned int mcu_id_hash;
    unsigned int work_normal_cnt; // 校验不对时正常工作的次数

    unsigned int  magic_version1;
    unsigned char mask_bitmap[13]; // 默认均为1
    unsigned char extra_data[20]; //预留扩展使用
    unsigned int magic_version2;
};
#pragma pack()


#define CAPTURE_MAX_V_GAP  (BMP_ADJUST_BUFF_SIZE/ CAMERA_MAX_LINE_BUFF)

int get_valid_device_block(void);


void device_init_tanh_table(void);

void load_and_sync_dev_info(void);
void reload_dev_info(void);
int device_init_default(void);

void force_update_devinfo(void);
char security_update_devinfo(void);

int device_save_digits_info(quadrange_t *qr,short offset[MAX_DIGIT_CNT]);
int device_save_crop_rect(struct TRECT *rect);
int device_save_light_pwm(signed char pwm);
int device_save_digit_count(unsigned char type);

/**指针**/
int device_save_pointers_info(point_t center[MAX_POINTER_COUNT],short pointer_width);

unsigned char device_get_digit_width(void);
int device_save_digit_width(unsigned char width);

int device_save_high_baud(int baud);
int device_get_high_baud(void);

int device_get_low_baud(void);
int device_save_low_baud(unsigned short baud);


int device_get_windows_width(void);
int device_get_windows_height(void);

int device_add_work_normal_cnt(void);

int device_save_num_factor(signed char num_factor);
int device_is_last_fractional(void);

const struct TRECT *get_digital_rect(void);
void device_get_devid(unsigned char devid[7]);
void device_set_devid(unsigned char devid[7]);


/*搜表*/
int  device_set_slient_id(unsigned char id);
int  device_is_id_slient(unsigned char id);
int  device_is_address_in_range(int id, int low,int top);

int is_meter_in_factory_mode(void);
void device_enter_factory_mode(void);
void device_info_factory_reset(void);


int device_save_color_reversal(unsigned char type);
void device_set_use_previous_flag(unsigned char use);
char device_is_use_previous_result(void);


char device_add_previous_result_use_cnt(void);
char device_get_previos_result(signed char buff[MAX_DIGIT_CNT]);
void device_save_previous_result(signed char buff[MAX_DIGIT_CNT]);
void device_reset_rewind(int index);
int device_can_rewind(int index);

void  device_reset_check_mask(void);
signed char  device_set_check_mask(unsigned char number[],unsigned char count,unsigned char value);
signed char  device_get_check_mask(unsigned char number[],unsigned char max_count);
char  is_number_check_masked(unsigned char number);



void device_reset_dirty_digit_mask(void);
signed char device_set_dirty_digit_mask(unsigned char number[],unsigned char count,unsigned char value);
signed char device_get_dirty_digit_mask(unsigned char number[],unsigned char max_count);
char  is_dirty_digit_masked(unsigned char number);

#define  PARAMETER_COPY 0x55 //copy成功
#define  PARAMETER_UNPACK 0xaa //unpack成功
#define  PARAMETER_RESET 0xff  //
#define  PARAMETER_UPDATE 0x33  //
char device_unpack_is_need(char uart_time_stage);
void device_set_unpack_magic(char uart_time_stage);


extern struct DEV_INFO devinfo;


#ifdef __cplusplus
}
#endif

#endif

