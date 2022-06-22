#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "comfunc.h"
#include "recognize.h"
#include "bmp_arithmetric.h"
#include "bmp_layer.h"
#include "hardware_layer.h"
#include "mem_manage.h"
#include "recognize_tools.h"
#include "bmp_ops.h"
#include "recognize_cnn.h"
#include "snaphist.h"
#include "proto_camera.h"
#include "protcl.h"
#include "math.h"

//捕捉将要识别的图像，将图像捕捉到ram中或者flash中。
int (*capture_bmp)(void) ;

//从ram或者flash中取数据
void (*read_bmp)(unsigned char buff[],unsigned char id) = read_sample_from_flash;


/*
   return < 0 表示两次结果无法融合。
          >= 0 两次识别融合的结果
*/
signed char combine_two_result(unsigned char cnn1,unsigned char cnn2,char allow_gap, int period)
{
    char num;
    /*
       cnn1 99	 0	  40
       cnn2 0	99	 42
    */
    if((cnn2>100) || (cnn1>100))
    {
        return ERROR_CNN_VERTIFY;
    }

    if (abs( cnn1 - cnn2) <=allow_gap)
    {
        num = (cnn1+cnn2+1)/2;
    }
    else if(abs(cnn1 -  cnn2) >= (period-allow_gap))
    {
        num = ((cnn1+cnn2+100+1)/2)%100;
    }
    else
    {
        num =  ERROR_CNN_VERTIFY;
    }

    return num;
}


/*数字在进位的时候，例如当19->20:
   十位数字:可能被识别为10-20
   个位数字可能被识别为:90-99
   两者可以组合为 19或者为20，但是不能被组合为29和10

   8-17   90-94  ->  19
   13-22   95-99  ->  20

   13 14 15 16 17 在两种情况下都能正常工作。
*/
enum
{
    STATUS_FORCE_CARRY,
    STATUS_FORCE_FLOOR,
    STATUS_NORMAL
};
#define LIMIT_CARRY  95

#ifndef ESSN_WCR_P

static char  _convert_a_number(signed char current,signed char previous,signed char *status)
{
    signed char result;

    switch(*status)
    {
    case STATUS_FORCE_CARRY:
        // prvious 会在88-99之间
        if((previous >= 88) && (previous <=94))
        {
            result =  (current +2)/10+1;
            if((current < 88) || (current > 97))
            {
                *status = STATUS_NORMAL;
            }
        }
        else if(previous >= 95)
        {
            result =  (current +7)/10;
            /*如果当前数字在93以上，那么进位之后还会进位，不需要进行清carry位*/
            if(current < 93)
            {
                *status = STATUS_NORMAL;
            }
        }
        break;

    case STATUS_FORCE_FLOOR:
        if((previous >= 88) && (previous <=94))
        {
            result =  (current +2)/10;
            /*如果当前数字脱离9的范围，那么变恢复正常*/
            if((current < 88) || (current > 97))
            {
                *status = STATUS_NORMAL;
            }
        }
        else if(previous >= 95)
        {
            result =  (current +7)/10 - 1+10;
            // 92 会向下归到8，脱离9的范围
            if(current < 93)
            {
                *status = STATUS_NORMAL;
            }
        }
        break;

    default:
        /*取1 9组合。上一位四舍五入进位，本位进行floor操作，不会进位*/
        if((current >= 90) && (current <=94))
        {
            result =  (current +5)/10;
            *status = STATUS_FORCE_FLOOR;
        }
        /*取2 0组合。上一位四舍五入进位，本位进行ceil操作,如果本位识别数字在90以上，则会进位*/
        else if(current >= LIMIT_CARRY)
        {
            result =  (current +5)/10;
            *status = STATUS_FORCE_CARRY;
        }
        else
        {
            //上一个数字有效，且不是9，那么当前数字不应该出现 0.5的情况
            int tail = current%10;
            if((5 == tail) && (previous >=0))
            {
                result = -1;
            }
            else
            {
                result =	(current +5)/10;
            }
        }
        break;
    }

    return result%10;
}

int category2digits(signed char real_numbers_10[]
                    ,signed char read_numbers_100[]
                    ,int cnt)
{

    int readx=0, i=0;
    int multx = 1;
    signed char previous = -1;
    signed char status = STATUS_NORMAL;
    char temp=0;

    for ( i = (cnt-1) ; i >= 0 ; i--)
    {
        temp = read_numbers_100[i];
        real_numbers_10[i] = _convert_a_number(read_numbers_100[i],previous,&status);
        if (real_numbers_10[i]  < 0)
        {
            return real_numbers_10[i];
        }
        previous =temp;
        readx =readx+real_numbers_10[i]*multx;
        multx *= 10;
    }

    return (readx);
}

static int _do_fix_dirty_digit(signed char raw_numbers[], signed char previous_numbers[],int digit_cnt, int fix_pos)
{
    signed char raw_digit_10[MAX_DIGIT_CNT]= {0};
    signed char previous_digit_10[MAX_DIGIT_CNT]= {0};
    int raw = 0;
    int previous = 0;
    int cnt_length = fix_pos + 1;
    int previous_value = 0;
    int i;

    category2digits(raw_digit_10, raw_numbers, digit_cnt);

    /*检查上次结果是否能被正常转换*/
    if(category2digits(previous_digit_10, previous_numbers, digit_cnt)< 0)
    {
        return clac_valid_cnt(raw_numbers, digit_cnt);
    }

    /*获取上一次的高位值*/
    for(i=0; i < cnt_length; i++)
    {
        previous_value =previous_value*10 + previous_digit_10[i];
    }

    raw = raw_digit_10[fix_pos+1];
    previous =  previous_digit_10[fix_pos+1];

    /*当前数字都是识别数字低位都是9*/
    if(is_all_nine(&raw_numbers[fix_pos+1],digit_cnt-fix_pos-1))
    {
        return clac_valid_cnt(raw_numbers, digit_cnt);
    }
    /*进位+1*/
    else if(is_digit_winding(raw, previous,fix_pos+1))
    {
        previous_value++;
    }
    /*设置本次的高位值*/
    for(i=cnt_length-1; i >=0 ; i--)
    {
        raw_numbers[i] = previous_value%10*10;
        previous_value = previous_value/10;
    }
    return clac_valid_cnt(raw_numbers, digit_cnt);
}

#else

static int _do_fix_dirty_digit(signed char raw_numbers[], signed char previous_numbers[],int digit_cnt, int fix_pos)
{
    signed char raw_digit_10[MAX_DIGIT_CNT]= {0};
    signed char previous_digit_10[MAX_DIGIT_CNT]= {0};
    int raw = 0;
    int previous = 0;
    int cnt_length = fix_pos + 1;
    int previous_value = 0;
    int i;
    int low_current=0;

    category2digits(raw_digit_10, raw_numbers, digit_cnt);

    /*检查上次结果是否能被正常转换*/
    if(category2digits(previous_digit_10, previous_numbers, digit_cnt)< 0)
    {
        return clac_valid_cnt(raw_numbers, digit_cnt);
    }

    /*获取上一次的高位值*/
    for(i=0; i < cnt_length; i++)
    {
        previous_value = previous_value*10 + previous_digit_10[i];
    }

    raw = raw_digit_10[fix_pos+1];
    previous =  previous_digit_10[fix_pos+1];

    /*获取current low postion value*/
    for(i=fix_pos+1; i < digit_cnt; i++)
    {
        if(raw_digit_10[i]<0)
        {
            return clac_valid_cnt(raw_numbers, digit_cnt);
        }
        low_current = low_current*10 + raw_digit_10[i];
    }

    if(is_all_nine(&raw_numbers[fix_pos+1],digit_cnt-fix_pos-1))
    {
        return clac_valid_cnt(raw_numbers, digit_cnt);
    }
    /*进位+1*/
    else if(is_digit_winding(raw, previous,fix_pos+1))
    {
        previous_value++;
    }

    for(i=0; i<digit_cnt-fix_pos-1;i++)
    {
        previous_value *= 10;
    }
    previous_value+=low_current;

    /*设置本次的高位值*/
    raw_numbers[digit_cnt-1] =  (previous_value%10)*10;
    previous_value = previous_value/10;
    for(i=digit_cnt-2; i>=0 ; i--)
    {
        raw_numbers[i] = (previous_value%10)*10 + raw_numbers[i+1]/10;
        previous_value = previous_value/10;
    }

    return clac_valid_cnt(raw_numbers, digit_cnt);
}

/*当前数字i的取值，和上一个数字相关。
  1. 如果当前数字是
*/
static char  _convert_a_number(signed char current,signed char previous,signed char *status)
{
    signed char result=-1;
    int fraction = current%10;

    if(previous>=0 && combine_two_result(fraction, previous, 4, 10)<0)
    {
       return	-1;
    }

    switch(*status)
    {
    case STATUS_FORCE_CARRY:
        // 46 ,  round up 50
        if((fraction>=6))
        {
            result = (current)/10+1;
        }
        //23, 99
        else if((fraction<=4))
        {
            result = current/10;
        }
        else
        {
            result = -1;
        }
        if((current < 95) && (current >3))
        {
            *status = STATUS_NORMAL;
        }
        break;

    case STATUS_FORCE_FLOOR:
        // 46 ,  round up 50
        if((fraction>=6))
        {
            result = (current)/10;
        }
        //23, 99
        else if((fraction<=4))
        {
            result = current/10+9;
        }
        else
        {
            result = -1;
        }
        if((current < 85) && (current > 3))
        {
            *status = STATUS_NORMAL;
        }
        break;

    default:
        if(previous < 0)
        {
            if(current >= LIMIT_CARRY)
            {
                *status = STATUS_FORCE_CARRY;
                result = 0;
            }
            else if(current >= 90)
            {
                result = ((current +5)/10)%10;
                *status = STATUS_FORCE_FLOOR;
            }
            else
            {
                result = (current +5)/10;
            }
        }
        else
        {
            if(current>=95)
            {
                /*preivous is 0-3, current is 97-99, current should be convert as 0*/
                if((current>=97) && (previous<=3))
                {
                    *status = STATUS_FORCE_CARRY;
                    result = 0;
                }
                /*0 0 95 84 ->0 0 9 8*/
                else if((current<97) &&(previous>3))
                {
                    *status = STATUS_FORCE_FLOOR;
                    result = 9;
                }
                /*0 0 98 83
                 1,30,96,44
               */
                else if((current<= 99) &&(previous>5))
                {
                    *status = STATUS_FORCE_FLOOR;
                    result = 9;
                }
                else
                {
                    result = -1;
                }
            }
            else if(current >= 90)
            {
                result = ((current - previous+5+100)/10)%10;
                *status = STATUS_FORCE_FLOOR;
            }
            else
            {
                result = (((current - previous +5)+100)/10)%10;
            }

        }
        break;
    }

    if (result < 0)
    {
        return result;
    }

    return result%10;
}

/*digit map, fix problom cause by angle of view*/
signed char digit_map(signed char look_number, int idx, signed char previous)
{
     //>0  1 : add look number ,
     //<- -1 : sub look number
                                       //0   10     20   30  40  50   60   70   80   90:look number
                                     //-95-5 6-15   20   18  29   0   49    0    0   82   92:actual real number
    signed char map[MAX_POINTER_COUNT-1][10]={{1,    0,   -1,   -1,  -1, -1,   0,  +1,  +1,  +1},
                                       //2   11    19    28   38  49    59  69   81  91
                                       {1,    1,    0,   -1,  -1, -1,   -1,  0,   1,   1},
                                       //2   11    21    31   40  49    60  69   79   90
                                       {1,   1,     1,    1,  -1,  -1,   -1,  -1,  -1,   0},
                                       //1   11     21    32  41  50    58   69  78    89
                                       {0,   1,     1,    1,  1,  0,    0,  -1,  -1,  -1},
                                       //99  11     21    32  41  50    58  69   78   89
                                       {0,   1,     1,    1,   1,  0,   0,  -1,  -1,  -1},
                                    };

    int reference = ((previous+5)/10)%10;
    int units_value = look_number%10;
    int ten_value = ((look_number+5)/10)%10;

    if(idx >= (MAX_POINTER_COUNT-1))
    {
        return look_number;
    }

    if(abs(units_value - reference) <=3 || abs(units_value - reference) >= 7)
    {
        return look_number;
    }

    return (look_number+map[idx][ten_value]+100)%100;
}

int category2digits(signed char real_numbers_10[]
                    ,signed char read_numbers_100[]
                    ,int cnt)
{

    int readx=0, i=0;
    int multx = 1;
    signed char previous = -1;
    signed char status = STATUS_NORMAL;
    char temp=0;
    signed char map_digit[MAX_POINTER_COUNT]={0};

    /*map digit*/
    for ( i = 0 ; i <= cnt-2 ; i++)
    {
        map_digit[i] = digit_map(read_numbers_100[i], i, read_numbers_100[i+1]);
    }

    map_digit[cnt-1] = read_numbers_100[cnt-1];

    /*convert*/
    for ( i = (cnt-1) ; i >= 0 ; i--)
    {
        temp = map_digit[i];
        real_numbers_10[i] = _convert_a_number(map_digit[i],previous,&status);
        if (real_numbers_10[i]  < 0)
        {
            return real_numbers_10[i];
        }
        previous =real_numbers_10[i];
        readx =readx+real_numbers_10[i]*multx;
        multx *= 10;
    }

    return (readx);
}

#endif


signed char is_hight_position_same(signed char  last_numbers[],signed char raw_number[])
{
    int i=0;
    signed char same = 1;
    char count = devinfo.digit_count-1;

    if(device_is_last_fractional())
    {
        count -= 1;
    }

    for(i =0; i<count; i++)
    {
        if((raw_number[i]<0) || (combine_two_result(last_numbers[i],raw_number[i],10, 100)<0) )
        {
            same = 0;
        }
    }
    return same;
}


int is_result_accept(cnn_result_t  *re,unsigned char limit)
{
    return re->max_probility>limit;
}

int is_all_nine(signed char raw_numbers[], int count)
{
    int i =0;

    /*低位数字都是9，由进位系统自动加1,fixed 函数不进行加1操作*/
    for(i=0; i < count; i++)
    {
        if(raw_numbers[i] < 93)
        {
            return 0;
        }
    }
    return 1;
}


int is_digit_winding(int raw,int previous,int index)
{
    int gap = raw - previous;
    if((gap < -1) && (gap > -9))
    {
        return 1;
    }
    else if(raw == 0 && previous==9 && device_can_rewind(index))
    {
        device_reset_rewind(index);
        return 1;
    }
    return 0;
}

int clac_valid_cnt(signed char raw_numbers[],int digit_cnt)
{
    int i = 0;
    int valid = 0;
    for(i=0; i < digit_cnt; i++)
    {
        if(raw_numbers[i]>=0)
        {
            valid++;
        }
    }
    return valid;
}


/*
 脏字轮处理策略：
 1. 如果是高位字轮脏，那么便自动计数。
 2. 最低位字轮脏，那么便返回上一次数据，可以无限次返回，这个功能是通过返回上一次数据实现的，不需要特殊处理。
*/
int fix_dirty_digit(signed char raw_numbers[], signed char previous_numbers[],int digit_cnt)
{
    int i=0;
    int invalid_pos = -1;

    /*找到最低位无效数据*/
    for(i=(digit_cnt-2); i >=0; i--)
    {
        if(raw_numbers[i] < 0 )
        {
            invalid_pos = i;
            break;
        }
    }

    /*无需恢复数据*/
    if(invalid_pos < 0)
    {
        return clac_valid_cnt(raw_numbers, digit_cnt);
    }

    for(i=invalid_pos; i >=0; i--)
    {
        int gap =0;
        /*确认无法识别数据的位都被设置为mask*/
        if(raw_numbers[i] < 0 && !is_dirty_digit_masked(i))
        {
            return clac_valid_cnt(raw_numbers, digit_cnt);
        }
        /*确认高位字轮能正常识别的都和上次一样*/
        gap = abs(raw_numbers[i] - previous_numbers[i]);
        if(raw_numbers[i] > 0 &&  ((gap>3) &&  (gap<96)))
        {
            return clac_valid_cnt(raw_numbers, digit_cnt);
        }
    }

    /*保证脏字轮的下一位是有效的*/
    if(raw_numbers[invalid_pos+1] < 0)
    {
        return clac_valid_cnt(raw_numbers, digit_cnt);
    }

    return _do_fix_dirty_digit(raw_numbers, previous_numbers, digit_cnt, invalid_pos);
}

int recognize_gray_digits(unsigned char real_numbers[],
                          int (*recognize_digit)(int, int, cnn_result_t *))
{
    signed char previous_numbers[MAX_DIGIT_CNT]= {0};
    unsigned char prevous_valid = device_get_previos_result(previous_numbers);
    int i,readx=-1 ;
    cnn_result_t result2;
    cnn_result_t result1;
    signed char raw_numbers[MAX_DIGIT_CNT]= {0};
    unsigned char valid_digit_count=0;
    unsigned char same_digit_count=0;
    reset_performance();

    if(capture_bmp)
    {
        capture_bmp();
    }
    /*识别数字*/
    for (i  = 0 ; i < devinfo.digit_count; i++)
    {
        signed char current_number=-1;
        signed char ret=-1;
        unsigned char limit = CONFIDIENCE_PROBILITY_HIGH;
        CNN_SWITCH_FEATUEE(0);
		send_idle_data(0xFE);
        ret = recognize_digit(i,0,&result1);
        /*本次识别结果错误*/
        if(ret < 0)
        {
            raw_numbers[i] = ret;
            continue;
        }

        if(is_number_check_masked(result1.digit))
        {
            limit  = CONFIDIENCE_PROBILITY_MASK;
        }

        /* 如果上次结果有效，本次识别结果和上次相同，则不需要验证。
         * 识别效果不同，则需要验证。
         */
        if(prevous_valid
                && (i< 4)
                && (combine_two_result(result1.digit, previous_numbers[i]+1,3,100)>= 0)
                &&  is_result_accept(&result1,limit))
        {
            if(result1.digit < previous_numbers[i])
                current_number = combine_two_result(result1.digit, previous_numbers[i]+1,3,100);
            else
                current_number = combine_two_result(result1.digit, previous_numbers[i],3,100);
        }
        else
        {
            limit = max(CONFIDIENCE_PROBILITY_HIGH, limit);
            CNN_SWITCH_FEATUEE(1);
            recognize_digit(i, 1, &result2);
            current_number = combine_two_result(result1.digit,result2.digit,3,100);
            if(!is_result_accept(&result1,limit)  || !is_result_accept(&result2,limit))
            {
                current_number =  ERROR_CNN_MIN_PROBILITY;
            }
        }

        if(current_number >= 0)
        {
            valid_digit_count++;
            if(combine_two_result(current_number, previous_numbers[i],2,100)>=0)
            {
                same_digit_count++;
            }
        }
        raw_numbers[i] = current_number;
    }

    if(prevous_valid)
    {
        valid_digit_count = fix_dirty_digit(raw_numbers, previous_numbers, devinfo.digit_count);
    }

    if(devinfo.digit_count == valid_digit_count)
    {
        readx = category2digits(real_numbers,raw_numbers,devinfo.digit_count);
    }

    if((devinfo.digit_count == valid_digit_count) && (readx>=0))
    {
#ifndef CONFIG_TEST_MBUS   
        device_save_previous_result(raw_numbers);
#endif 
    }
    else if(prevous_valid && device_is_use_previous_result())
    {
        /*
         * 识别失败时，如果数字高位一样，那么，那么概率需要提高。
          4. 如果本次识别失败，并且需要返回上次数据
        */
        if(is_hight_position_same(previous_numbers,raw_numbers))
        {
            memcpy(raw_numbers,previous_numbers,devinfo.digit_count);
            readx = category2digits(real_numbers,raw_numbers,devinfo.digit_count);
        }
        else if((valid_digit_count>=2)//识别的图片是有意义的，至少1两个数和上次识别的一样
                && (same_digit_count>=1)
                && device_add_previous_result_use_cnt())
        {
            memcpy(raw_numbers,previous_numbers,devinfo.digit_count);
            readx = category2digits(real_numbers,raw_numbers,devinfo.digit_count);
        }
        else
        {
            return ERROR_CNN_MIN_PROBILITY;
        }
    }
    else
    {
        return ERROR_CNN_MIN_PROBILITY;
    }

    store_performance();

    return readx;
}

int recognize_gray_digits_dual_cnn(unsigned char real_numbers[])
{
    return recognize_gray_digits(real_numbers,recognize_a_gray_digit_cnn);
}


int reconginze_raw_number_use_cnn(unsigned char raw_numbers[])
{
    int i = 0;
    cnn_result_t result;

    if(capture_bmp)
    {
        capture_bmp();
    }

    for (i = 0; i < (devinfo.digit_count); i++)
    {	
        raw_numbers[i] = recognize_a_gray_digit_cnn(i,0,&result);
        if(result.max_probility <= CONFIDIENCE_PROBILITY_HIGH)
            raw_numbers[i] = ERROR_CNN_MIN_PROBILITY;
        raw_numbers[i+devinfo.digit_count*2] = result.max_probility;
        raw_numbers[i+devinfo.digit_count*4] = result.second_probility;

        raw_numbers[i+(devinfo.digit_count)]  = recognize_a_gray_digit_cnn(i,1,&result);
        if(result.max_probility<= CONFIDIENCE_PROBILITY_HIGH)
            raw_numbers[i+(devinfo.digit_count)] = ERROR_CNN_MIN_PROBILITY;
        raw_numbers[i+devinfo.digit_count*2+(devinfo.digit_count)] = result.max_probility;
        raw_numbers[i+devinfo.digit_count*4+(devinfo.digit_count)] = result.second_probility;
    }
    return 0;
}

