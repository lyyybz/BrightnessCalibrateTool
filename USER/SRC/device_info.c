#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "device_info.h"
#include "bmp_layer.h"
#include "comfunc.h"
#include "protcl.h"
#include "bitmap.h"
#include "math.h"
#include "mem_manage.h"
#include "hardware_layer.h"
//#include "bmp.h"
#include "comfunc.h"
#include "bitmap.h"


struct DEV_INFO devinfo __attribute__((aligned(4)));

#define MAGIC_NUM           0x55BB2B2A
enum
{
    FLASH_NORMAL_AREA,
    FLASH_BACK_UP_AREA,
    FLASH_NONE_AREA
};
struct search_meter
{
    unsigned char id;
    unsigned char enable;
};

static  struct search_meter  _search_info;


static int valid_block = FLASH_NONE_AREA;

const struct TRECT *get_digital_rect(void)
{	
#ifdef ESSN_WCR_P
   devinfo.digital_rect.left = 0;
   devinfo.digital_rect.top = 0;
   devinfo.digital_rect.right = WIN_DEFAULT_CAPUTE_WIDTH;
   devinfo.digital_rect.bottom = WIN_DEFAULT_HEIGHT;
#endif 
    return  (&devinfo.digital_rect);
}

int get_valid_device_block()
{
    return valid_block;
}


void init_default_check_mask()
{
#ifdef ESSN_WCR_P
    device_reset_check_mask();
#else
    unsigned char mask[]= {74,75,76,14,15,16,58,97,86,87,88}; //(97:87),(58:88)
    device_reset_check_mask();
    device_set_check_mask(mask,sizeof(mask),1);
#endif
}

void device_init_tanh_table()
{
    short int expect_table[]= {8186,8186,8186,8186,8186,8186,8187,8187};
    short int buff[8] = {0};
    int tanh_true = 1;
    unsigned int i =0;

    if(FLASH_NONE_AREA == valid_block)
        return;
    dataflash_read(TANH_ADDR+TANH_SIZE-sizeof(buff),buff,sizeof(buff));

    for(i=0; i< ARRAY_SIZE(expect_table); i++)
    {
        if(abs(buff[i] - expect_table[i]) > 1)
        {
            tanh_true = 0;
            break;
        }
    }

    if(!tanh_true)
    {
        int i =0, j=0 ;
        hardware_init_high_power();
        short *data = mem_alloc(0x100);
        dataflash_erase(TANH_ADDR, TANH_SIZE);
        for(i=0; i< 8*1024; i+=0x100/2)
        {
            for(j = 0; j<0x100/2; j++)
            {
                data[j] = tanh((i+j)*4/(1024*8.0))*8*1024+0.5;
            }
            dataflash_write(i*2, data, 0x100);
        }
        mem_free(data);
        hardware_init_low_power();
    }
}

void force_update_devinfo(void)
{
    struct DEV_INFO devinfo_test;

    devinfo.magic_version = Cal_CRC16((const volatile uint8_t*)(&devinfo),offsetof(struct DEV_INFO,magic_version));

    if(valid_block == FLASH_NORMAL_AREA)
    {
        dataflash_erase(DEV_INFO_BACKUP_ADDR,sizeof(devinfo));
        dataflash_write(DEV_INFO_BACKUP_ADDR,&devinfo,sizeof(devinfo));

        dataflash_read(DEV_INFO_BACKUP_ADDR,&devinfo_test,sizeof(devinfo_test));
        if(0 != memcmp(&devinfo_test, &devinfo, sizeof(devinfo)))
            return;
        dataflash_erase(DEV_INFO_ADDR,sizeof(devinfo));
        dataflash_write(DEV_INFO_ADDR,&devinfo,sizeof(devinfo));
    }
    else
    {
        dataflash_erase(DEV_INFO_ADDR,sizeof(devinfo));
        //dataflash_read(DEV_INFO_ADDR+64*1024 ,&devinfo_test,sizeof(devinfo_test));
        dataflash_write(DEV_INFO_ADDR,&devinfo,sizeof(devinfo));

        dataflash_read(DEV_INFO_ADDR,&devinfo_test,sizeof(devinfo_test));
        if(0 != memcmp(&devinfo_test, &devinfo, sizeof(devinfo)))
            return;
        dataflash_erase(DEV_INFO_BACKUP_ADDR,sizeof(devinfo));
        dataflash_write(DEV_INFO_BACKUP_ADDR,&devinfo,sizeof(devinfo));
    }

    valid_block = FLASH_NORMAL_AREA;
}

char security_update_devinfo()
{
    if(FLASH_NONE_AREA == valid_block)
    {
        return 0;
    }

    force_update_devinfo();

    return 1;
}


void device_info_factory_reset(void)
{
    reset_bit(devinfo.flags,FLAG_FACTTORY_OUT);
    force_update_devinfo();
}

char dev_info_load_vertify(struct DEV_INFO *devinfo,int address,int valid_area)
{
    dataflash_read(address, devinfo, sizeof(struct DEV_INFO));
    if(is_all_xx(devinfo,0,sizeof(struct DEV_INFO)))//上电电压不稳定，读取flash全为0
    {
        return 0;
    }
    if (devinfo->magic_version == Cal_CRC16((const volatile uint8_t*)devinfo,
                                            offsetof(struct DEV_INFO,magic_version)))
    {
        valid_block = valid_area;
        return  1;
    }

    return 0;
}

char dev_info_load_vertify_older(struct OLD_DEV_INFO *devinfo,int address,int valid_area)
{
    dataflash_read(address, devinfo, sizeof(struct OLD_DEV_INFO));
    if(is_all_xx(devinfo,0,sizeof(struct OLD_DEV_INFO)))//上电电压不稳定，读取flash全为0
    {
        return 0;
    }
    if (devinfo->magic_version1 == Cal_CRC16((const volatile uint8_t*)devinfo,
            offsetof(struct OLD_DEV_INFO,magic_version1)))
    {
        valid_block = valid_area;
        return  1;
    }

    return 0;
}

static int _device_init_digits_info(quadrange_t *qr,short offset[MAX_DIGIT_CNT])
{
    int width,height;
    struct TRECT digital_rect;
    int padding;

    digital_rect.left =  min(qr->p00.x,qr->p10.x);
    digital_rect.right = max(qr->p01.x,qr->p11.x)+1;
    digital_rect.top = min(qr->p00.y,qr->p01.y);
    digital_rect.bottom = max(qr->p10.y,qr->p11.y)+1;

    width = digital_rect.right - digital_rect.left;
    padding = (4- width%4)%4;

    if((1 == padding) && (digital_rect.left>=1) )
    {
        digital_rect.left -=1;
    }
    else if((2 == padding)&&(digital_rect.left>=2))
    {
        digital_rect.left -=2;
    }
    else if(3 == padding &&(digital_rect.left>=3))
    {
        digital_rect.left -=3;
    }
    else
    {
        digital_rect.right = digital_rect.left + width+ padding;
    }

    if(digital_rect.left < 0)
        digital_rect.left = 0;

    while(digital_rect.right > WIN_DEFAULT_CAPUTE_WIDTH)
        digital_rect.right -= 4;

    height = digital_rect.bottom - digital_rect.top;

    if(abs(qr->p00.y - qr->p01.y)>= CAPTURE_MAX_V_GAP)
    {
        return -1;
    }

    if(width > CAMERA_MAX_LINE_BUFF)
    {
        return -1;
    }

    devinfo.di.digital_quadrange = *qr;
    memcpy(devinfo.di.digit_offset,offset,sizeof(devinfo.di.digit_offset));
    devinfo.digital_rect = digital_rect;

    init_rorate_context(&devinfo.rorate_context,
                        &devinfo.di.digital_quadrange
                        ,width
                        ,height);
    return 0;
}

static int _device_init_pointers_info(point_t center[MAX_POINTER_COUNT],short width)
{
    int i = 0;
    struct TRECT digital_rect;
    devinfo.pi.pointer_width = width;
    memcpy(devinfo.pi.center,center,MAX_POINTER_COUNT*4);
    digital_rect.left = 0;
    digital_rect.right = WIN_DEFAULT_CAPUTE_WIDTH;
    digital_rect.top = 0;
    digital_rect.bottom = WIN_DEFAULT_HEIGHT;
    for(i=0; i < devinfo.digit_count; i++)
    {
        digital_rect.left = min(devinfo.pi.center[i].x - (devinfo.pi.pointer_width/2),digital_rect.left) ;
        digital_rect.right =max(devinfo.pi.center[i].x + (devinfo.pi.pointer_width/2), digital_rect.right);
        digital_rect.top = min(devinfo.pi.center[i].y - (devinfo.pi.pointer_width/2),digital_rect.top) ;
        digital_rect.bottom = max(devinfo.pi.center[i].y +(devinfo.pi.pointer_width/2),digital_rect.bottom);
    }
    devinfo.digital_rect.left = max(digital_rect.left, 0);
    devinfo.digital_rect.right = min(digital_rect.right, WIN_DEFAULT_CAPUTE_WIDTH);
    devinfo.digital_rect.top = max(digital_rect.top, 0);
    devinfo.digital_rect.bottom = max(digital_rect.bottom, WIN_DEFAULT_HEIGHT);
    return 1;
}


static void  _init_default_para(struct DEV_INFO *ndevinfo)
{
    point_t center[MAX_POINTER_COUNT];
    short width = POINTER_WIDTH;//60
    unsigned char i;
    quadrange_t   quad;
    short offset[MAX_DIGIT_CNT] = {0};
    memset(ndevinfo,0xff,sizeof(struct DEV_INFO));
    ndevinfo->light_pwm = 40;
    ndevinfo->flags = 0;
    ndevinfo->steps = 0;
    ndevinfo->num_factor = FACOTR_NORMAL;

    ndevinfo->devid[0] = 0x01;//设备地址
    ndevinfo->devid[1] = 0x02;
    ndevinfo->devid[2] = 0x03;
    ndevinfo->devid[3] = 0x04;
    ndevinfo->devid[4] = 0x05;
    ndevinfo->devid[5] = 0x06;
    ndevinfo->devid[6] = 0x07;

    ndevinfo->crop_rect.left = WIN_DEFAULT_X0;
    ndevinfo->crop_rect.top = WIN_DEFAULT_Y0;
    ndevinfo->crop_rect.right = WIN_DEFAULT_X0+ WIN_DEFAULT_CAPUTE_WIDTH;
    ndevinfo->crop_rect.bottom =WIN_DEFAULT_Y0+ WIN_DEFAULT_HEIGHT ;

    quad.p00.x = 0;
    quad.p00.y = 0;
    quad.p01.x = 240;
    quad.p01.y = 0;
    quad.p10.x = 0;
    quad.p10.y = 60;
    quad.p11.x = 240;
    quad.p11.y = 60;

    /*****初始化圆心位置****************/
    center[0].x=75;
    center[0].y=210;
    center[1].x=40;
    center[1].y=150;
	center[2].x=55;
    center[2].y=85;
	center[3].x=105;
    center[3].y=45;
	center[4].x=170;
    center[4].y=45;
	center[5].x=220;
    center[5].y=80;
    ndevinfo->highspeed_baud = DEFAULT_HIGHSPEED_BAUD;
    ndevinfo->digit_count = 0x05;//normal water meter

    ndevinfo->color_reverse = 0x00;

    init_default_check_mask();
	
#ifdef ESSN_WCR_P
	_device_init_pointers_info(center,width);
#else
    _device_init_digits_info(&quad,offset);  
#endif

    devinfo.digit_width = DIGIT_WIDTH;
    devinfo.dirty_digit_mask = 0xfC;

}

static void _load_dev_info(void)
{
    struct OLD_DEV_INFO old_devinfo;
    struct OLD_DEV_INFO *pdevinfo = &old_devinfo;
    struct DEV_INFO *ndevinfo = &devinfo;

    if(FLASH_NONE_AREA != valid_block)
        return;
    if(is_dataflash_id_correct() == 0)
        return;

    /*Flash中保存最新版本的参数*/
    if (dev_info_load_vertify(ndevinfo,DEV_INFO_ADDR,FLASH_NORMAL_AREA) ||
            dev_info_load_vertify(ndevinfo,DEV_INFO_BACKUP_ADDR,FLASH_BACK_UP_AREA))
    {

        return;
    }

    /*Flash中没有参数，需要重新初始化*/
    if(is_all_xx(ndevinfo,0xff,sizeof(struct DEV_INFO)))
    {
        if(is_dataflash_id_correct() == 0)
            return;
        _init_default_para(ndevinfo);
        force_update_devinfo();
    }

    /*Flash中有老版本的参数，需要对参数进行升级*/
    if(dev_info_load_vertify_older(pdevinfo,DEV_INFO_ADDR,FLASH_NORMAL_AREA) ||
            dev_info_load_vertify_older(pdevinfo,DEV_INFO_BACKUP_ADDR,FLASH_BACK_UP_AREA))
    {
        _init_default_para(ndevinfo);
        memcpy(ndevinfo->devid, pdevinfo->devid,7);
        ndevinfo->mcu_id_hash = pdevinfo->mcu_id_hash;
        if(!is_all_xx(pdevinfo->mask_bitmap,0xFF,sizeof(pdevinfo->mask_bitmap)))
            memcpy(ndevinfo->check_mask_bitmap,pdevinfo->mask_bitmap,sizeof(pdevinfo->mask_bitmap));

        memcpy(ndevinfo->di.digit_offset,pdevinfo->digit_offset,sizeof(pdevinfo->digit_offset));

        ndevinfo->crop_rect = pdevinfo->crop_rect;
        ndevinfo->digital_rect = pdevinfo->digital_rect;
        ndevinfo->di.digital_quadrange = pdevinfo->digital_quadrange;
        ndevinfo->rorate_context = pdevinfo->rorate_context;
        ndevinfo->highspeed_baud = pdevinfo->highspeed_baud;
        ndevinfo->batch = pdevinfo->batch;
        ndevinfo->light_pwm = pdevinfo->light_pwm;
        ndevinfo->flags = pdevinfo->flags;
        ndevinfo->num_factor = pdevinfo->num_factor;
        ndevinfo->use_cnt = pdevinfo->use_cnt;
        ndevinfo->work_normal_cnt = pdevinfo->work_normal_cnt;
        force_update_devinfo();
    }

#if CONFIG_DEBUG
    devinfo.batch++;
    devinfo.steps = 0;
    security_update_devinfo();
#endif
}

void load_and_sync_dev_info()
{
	static int last_low_baud = 0x00;
	
	_load_dev_info();
	if(last_low_baud != device_get_low_baud())
	{
		usart_init(device_get_low_baud(),1);
		last_low_baud = device_get_low_baud();
	}
}

void reload_dev_info()
{
    valid_block = FLASH_NONE_AREA;
    load_and_sync_dev_info();
}


void device_get_devid(unsigned char devid[7])
{
    memcpy(devid,devinfo.devid,7);
}

void device_set_devid(unsigned char devid[7])
{
    memcpy(devinfo.devid,devid,7);
    force_update_devinfo();
}

int  device_set_slient_id(unsigned char id)
{
    _search_info.enable = 1;
    _search_info.id = id;
    return 0;
}

int  device_is_id_slient(unsigned char id)
{
    if(_search_info.enable && (_search_info.id == id))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int  device_is_address_in_range(int id, int low,int top)
{
    int value =0;

    if(id >= 14)
    {
        return 0;
    }

    if(low == 0xa && top == 0xa)
    {
        return 1;
    }

    value = devinfo.devid[id/2];

    if(0 == (id%2))
    {
        value = (value & 0xf0) >> 4;
    }
    else
    {
        value = value&0x0f;
    }

    if(value >= low && value <= top)
    {
        return 1;
    }
    else
    {
        return 0;
    }

}


int is_meter_in_factory_mode(void)
{
    return  is_bit_set(devinfo.flags,FLAG_FACTTORY_OUT);
}

void device_enter_factory_mode()
{
    set_bit(devinfo.flags,FLAG_FACTTORY_OUT);
    force_update_devinfo();
}

int device_init_default()
{
    unsigned int hash_id = 0;
    unsigned char devid[7];

    dataflash_erase(DEV_INFO_ADDR, sizeof(struct DEV_INFO));
    dataflash_erase(DEV_INFO_BACKUP_ADDR, sizeof(struct DEV_INFO));
    hash_id = devinfo.mcu_id_hash;
    device_get_devid(devid);
    _init_default_para(&devinfo);
    devinfo.mcu_id_hash = hash_id;
    memcpy(devinfo.devid, devid, sizeof(devid));
    force_update_devinfo();
   	load_and_sync_dev_info();
    return 0;
}

//保存数字字轮信息
int device_save_digits_info(quadrange_t *qr,short offset[MAX_DIGIT_CNT])
{
    int ret;
    ret = _device_init_digits_info(qr,offset);
    if(ret < 0)
        return ret;
    force_update_devinfo();

    return 0;
}


//保存指针信息
int device_save_pointers_info(point_t center[MAX_POINTER_COUNT],short pointer_width)
{
    int ret;
    ret = _device_init_pointers_info(center,pointer_width);
    if(ret < 0)
        return ret;
    force_update_devinfo();

    return 0;
}


int device_save_crop_rect(struct TRECT *rect)
{
    int height = rect->bottom - rect->top;
    int width = rect->right - rect->left;

    if(0!=(width %4))
    {
        return -1;
    }

    if((height > CMAERA_MAX_HEGITH)
            ||(height <= 0)
            || (rect->top <0)
            || (rect->top > CMAERA_MAX_HEGITH)
            ||(rect->bottom <0)
            || (rect->bottom > CMAERA_MAX_HEGITH) )
    {
        return -1;
    }

    if((width > CMAERA_MAX_WIDTH)
            ||(width <= 0)
            || (rect->left <0)
            || (rect->left > CMAERA_MAX_WIDTH)
            || (rect->right <0)
            || (rect->right > CMAERA_MAX_WIDTH) )
    {
        return -1;
    }

    devinfo.crop_rect = *rect;

    force_update_devinfo();

    return 0;
}

int device_get_windows_width()
{
    return devinfo.crop_rect.right - devinfo.crop_rect.left;
}


int device_get_windows_height()
{
    return devinfo.crop_rect.bottom - devinfo.crop_rect.top;
}

int device_save_high_baud(int baud)
{
    if(baud < 1200 || baud > 115200)
    {
        return 1;
    }

    devinfo.highspeed_baud = baud;
    force_update_devinfo();
    return 0;
}


int device_save_light_pwm(signed char pwm)
{
    if(pwm <0 || pwm >100)
    {
        return -1;
    }
    devinfo.light_pwm = pwm;
    force_update_devinfo();
    return 0;
}

int device_save_digit_count(unsigned char type)
{
    if((type != 0x04)&&(type != 0x05)&&(type != 0x06))
    {
        return -1;
    }
    devinfo.digit_count = type;
    force_update_devinfo();
    return 0;
}

int device_save_digit_width(unsigned char width)
{
    if(width < 0)
    {
        return -1;
    }
    devinfo.digit_width = width;
    force_update_devinfo();
    return 0;
}

int device_save_color_reversal(unsigned char type)
{
    if((type != 0) && (type != 1))
    {
        return -1;
    }
    devinfo.color_reverse = type;
    force_update_devinfo();
    return 0;
}


int device_save_num_factor(signed char num_factor)
{
    if(5 != devinfo.digit_count)
    {
        return -1;
    }

    if((num_factor == FACOTR_NORMAL) || (num_factor== FACOTR_Point_1))
    {
        devinfo.num_factor = num_factor;
        force_update_devinfo();
        return 0;
    }
    else
    {
        return -1;
    }
}

int device_is_last_fractional()
{
    if(5 == devinfo.digit_count)
    {
        return  FACOTR_Point_1 == devinfo.num_factor ;
    }
    else
    {
        return 0;
    }

}


int device_add_work_normal_cnt(void)
{
    devinfo.work_normal_cnt++;
    security_update_devinfo();
    return 0;
}


char device_get_previos_result(signed char buff[MAX_DIGIT_CNT])
{
    memcpy(buff,devinfo.previos_digit,devinfo.digit_count);
    return is_bit_set(devinfo.flags,FLAG_PREVIOUS_RESULT_VALID);
}

void device_save_previous_result(signed char buff[MAX_DIGIT_CNT])
{
    int i =0;
    set_bit(devinfo.flags,FLAG_PREVIOUS_RESULT_VALID);
    memcpy(devinfo.previos_digit,buff,devinfo.digit_count);
    for(i =0; i< devinfo.digit_count; i++)
    {
        /*自动计数标志位，设置此标志位意味着可以回环*/
        if(buff[i]>15 && buff[i] < 85)
        {
            set_bit(devinfo.previos_digit[MAX_DIGIT_CNT-1],i);
        }
    }
    devinfo.use_cnt = 0;
    security_update_devinfo();
}

/*此处不保存，如果结果正确，会自动保存上次结果*/
void device_reset_rewind(int index)
{
    reset_bit(devinfo.previos_digit[MAX_DIGIT_CNT-1],index);
}

int device_can_rewind(int index)
{
    return is_bit_set(devinfo.previos_digit[MAX_DIGIT_CNT-1],index);
}

void device_set_use_previous_flag(unsigned char use)
{
    if(use)
    {
        reset_bit(devinfo.flags,FLAG_NO_USE_PRE_RESULT);
    }
    else
    {
        set_bit(devinfo.flags,FLAG_NO_USE_PRE_RESULT);
    }
    force_update_devinfo();
}
char device_is_use_previous_result()
{
    return !is_bit_set(devinfo.flags,FLAG_NO_USE_PRE_RESULT);
}

char device_add_previous_result_use_cnt(void)
{
    if(devinfo.use_cnt <= MAX_USE_CNT)
    {
        devinfo.use_cnt++;
        security_update_devinfo();
        return 1;
    }
    return 0;
}

void  device_reset_check_mask()
{
    memset(devinfo.check_mask_bitmap,0xff,sizeof(devinfo.check_mask_bitmap));
    force_update_devinfo();
}


signed char  device_set_check_mask(unsigned char number[],unsigned char count,unsigned char value)
{
    int ret;
    /*因为Flash中默认值为0xff，所以讲mask设置为0表示该位被mask*/
    ret =  bitmap_set(devinfo.check_mask_bitmap,100,number,count,!value);
    if(ret < 0)
        return ret;
    force_update_devinfo();
    return 0;
}


signed char  device_get_check_mask(unsigned char number[],unsigned char max_count)
{
    return bitmap_get_clear_bits(devinfo.check_mask_bitmap,100,number,max_count);
}


char  is_number_check_masked(unsigned char number)
{
    return bitmap_is_bit_clear(devinfo.check_mask_bitmap,100,number);
}

unsigned char  device_get_digit_width(void)
{
    if(0xff == devinfo.digit_width)
    {
        return DIGIT_WIDTH;
    }
    return devinfo.digit_width;
}

void device_reset_dirty_digit_mask()
{
    devinfo.dirty_digit_mask = 0xff;
    force_update_devinfo();
}


signed char device_set_dirty_digit_mask(unsigned char number[], unsigned char count, unsigned char value)
{
    int ret;
    /*因为Flash中默认值为0xff，所以讲mask设置为0表示该位被mask*/
    ret =  bitmap_set(&devinfo.dirty_digit_mask,DIGNT_MASK_MAX_COUNT,number,count,!value);
    if(ret < 0)
        return ret;
    force_update_devinfo();
    return 0;
}

signed char device_get_dirty_digit_mask(unsigned char number[], unsigned char max_count)
{
    return bitmap_get_clear_bits(&devinfo.dirty_digit_mask,DIGNT_MASK_MAX_COUNT,number,max_count);
}

char is_dirty_digit_masked(unsigned char number)
{
    return bitmap_is_bit_clear(&devinfo.dirty_digit_mask,DIGNT_MASK_MAX_COUNT,number);
}

char device_unpack_is_need(char stage)
{
    if(FLASH_NONE_AREA == valid_block)
    {
        return 0;
    }


    if((PARAMETER_UPDATE==stage) && (PARAMETER_UNPACK != devinfo.parameter_unpack))
    {
        return 1;
    }

    /*解压已经完成*/
    if(PARAMETER_UNPACK == devinfo.parameter_unpack)
    {
        return 0;
    }

    /*copy成功*/
    if(PARAMETER_COPY == devinfo.parameter_unpack && PARAMETER_COPY == stage)
    {
        return 0;
    }

    return 1;
}


void device_set_unpack_magic(char stage)
{
    devinfo.parameter_unpack = stage;
    security_update_devinfo();
}

int device_get_high_baud()
{
    return devinfo.highspeed_baud;
}


int device_get_low_baud()
{
    if(devinfo.low_baud == 1200)
    {
        return 1200;
    }
    return 2400;
}

int device_save_low_baud(unsigned short baud)
{
    if((baud == 1200) ||  (baud == 2400))
    {
        devinfo.low_baud = baud;
        force_update_devinfo();
        return 0;
    }

    return -1;
}






