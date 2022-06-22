#include "cnn_operator.h"
#include "cnn_para.h"
#include <stdlib.h>
#include "comfunc.h"
#include "mem_manage.h"
#include "cnn_para.h"
#include "recognize_tools.h"
#include "cnn.h"
#include "cnn_pad.h"

#define ACTIVE_VAlUE(x)  ((x)>(0)?(x):(x>>3))

void relu_in_place(short *input,int k)
{

    while (k >= 8)
    {
        input[0] = ACTIVE_VAlUE(input[0]);
        input[1] = ACTIVE_VAlUE(input[1]);
        input[2] = ACTIVE_VAlUE(input[2]);
        input[3] = ACTIVE_VAlUE(input[3]);
        input[4] = ACTIVE_VAlUE(input[4]);
        input[5] = ACTIVE_VAlUE(input[5]);
        input[6] = ACTIVE_VAlUE(input[6]);
        input[7] = ACTIVE_VAlUE(input[7]);
        k -= 8;
        input+=8;
    }
    while(k--)
    {
        input[0] = ACTIVE_VAlUE(input[0]);
        input++;
    }
}




typedef struct cnn_operator_type  cnn_operator_t;

void layer_exp_activate(cnn_data_t *in)
{
    int j;
    /*exp激活之后都为正数*/
    int *data_out = (int *)in->data;
    int max_value = 1;
    long long value;
    int max_input_value =16*128;

    for (j = 0 ; j < in->width; j++)
    {
        max_value = max(data_out[j],max_value);
    }

    /*如果最大值超出exp表的范围，那么便对数值进行归一化*/
    if(max_value > max_input_value )
    {
        for (j = 0 ; j < in->width; j++)
        {
            value =(long long)(data_out[j])*max_input_value;
            data_out[j] =  value/max_value;
        }
    }

    for (j = 0 ; j < in->width ; j++)
    {
        data_out[j] = exp_u(data_out[j]);
    }
}




void fullconnect_layer(cnn_layer_info_t *info,cnn_data_t *in,cnn_data_t *out)
{

    int i,j,member_offset;
    signed char *current_weight = NULL;
    signed char *back_weight = NULL;
    signed char *temp_weight = NULL;
    long  sum;
    int *data_out = NULL;
    fullconnect_t *kernel = &info->full_connect;
    int read_batch_size = in->width * in->height*sizeof(char);
    int  *bias_temp;
    short *feature;

    /*fullconnect 数据输出类型为int，需要单独编写内存输出函数*/
    out->width = kernel->hidden;
    out->height =1;
    out->data = mem_alloc(kernel->hidden*4);
    data_out =(int *)out->data;
    out->used_cnt = 1;

    /*异步读取需要多留四个字节用来存放读命令和地址*/
    current_weight = mem_add_extra_size(in->data,read_batch_size+DMA_EXTRA_SIZE);
    back_weight = mem_add_extra_size(in->data,read_batch_size+DMA_EXTRA_SIZE);
    member_offset = kernel->weight_offset;



    /*初始化为bias*/
    bias_temp =(int *)((unsigned char *)data_out);
    dataflash_read(kernel->bias_offset, bias_temp,kernel->bias_size);

    dataflash_start_read_prefix(kernel->weight_offset,
                                (uint8_t *)current_weight,
                                read_batch_size);

    member_offset = kernel->weight_offset+read_batch_size;


    for (j = 0; j < kernel->hidden; j++)
    {
        sum = 0;
        feature = (short *)in->data;
        for(i =0; i < in->used_cnt; i++)
        {
            dataflash_wait_read_end();

            if(j< (kernel->hidden-1)  || i < (in->used_cnt-1) )
            {
                dataflash_start_read_prefix(member_offset,
                                            (uint8_t *)back_weight,
                                            read_batch_size);
            }

            sum += mult_char_short_to_int((signed char *)(current_weight)+4, feature, read_batch_size);
            temp_weight=current_weight;
            current_weight = back_weight;
            back_weight = temp_weight;
            member_offset += read_batch_size;
            feature+=read_batch_size;
        }

        data_out[j] += sum>>(current_mulx);
    }
}


FORCE_INLINE  int  cnn_calc_n_k33(short *a_kernel,short *pixel,int line_width,int cnt,int page_size)
{
    int feature_in;
    long long sum = 0;
    short *page_start_pixel = pixel;

    for(feature_in=0; feature_in<cnt; feature_in++)
    {
        pixel = page_start_pixel;
        sum +=a_kernel[0] * pixel[0];
        CHECK_IN_INT_RANGE(sum);
        sum +=a_kernel[1] * pixel[1];
        CHECK_IN_INT_RANGE(sum);
        sum +=a_kernel[2] * pixel[2];
        CHECK_IN_INT_RANGE(sum);

        pixel += line_width;
        sum +=a_kernel[3] * pixel[0];
        CHECK_IN_INT_RANGE(sum);
        sum +=a_kernel[4] * pixel[1];
        CHECK_IN_INT_RANGE(sum);
        sum +=a_kernel[5] * pixel[2];
        CHECK_IN_INT_RANGE(sum);

        pixel += line_width;
        sum +=a_kernel[6] * pixel[0];
        CHECK_IN_INT_RANGE(sum);
        sum +=a_kernel[7] * pixel[1];
        CHECK_IN_INT_RANGE(sum);
        sum +=a_kernel[8] * pixel[2];
        CHECK_IN_INT_RANGE(sum);

        a_kernel += 9;
        page_start_pixel += page_size;
    }
    return sum >> current_mulx;
}

#if ASM_SPEEDUP
void cnn_k11_s11_p00_relu(cnn_layer_info_t *info, cnn_data_t *in, cnn_data_t *out)
{
    int feature_out ;
    short *cur_para;
    short *data_out = NULL;
    cnn_kernel_t *kernel = &info->cnn_kernel;
    int k = in->width * in->height  ;

    cnn_data_alloc_output_mem(out, in->width, in->height, kernel->feature_out, data_out)
    out->used_cnt += kernel->feature_out;

    for (feature_out = 0; feature_out < kernel->feature_out; feature_out++)
    {
        cur_para = cnn_layer_info_get_weight(info, feature_out);
        cnn1x1(cur_para,in->data,in->width,in->height,in->used_cnt,data_out,current_mulx,kernel->bias[feature_out]<<current_mulx);
        relu_in_place(data_out,k);
        data_out += k;
    }


}
#else
void cnn_k11_s11_p00_relu(cnn_layer_info_t *info, cnn_data_t *in, cnn_data_t *out)
{
    int feature_out, w, h;
    short *in_data;
    short *cur_para;
    short *data_out = NULL;
    short cache;
    int page_size = in->width * in->height;
    int width, height;
    cnn_kernel_t *kernel = &info->cnn_kernel;
    int feature_in = 0;
    long long sum = 0;
    short *page_start_pixel;
    short *a_kernel;

    width = in->width;
    height = in->height;
    cnn_data_alloc_output_mem(out, width, height, kernel->feature_out, data_out)
    out->used_cnt += kernel->feature_out;

    for (feature_out = 0; feature_out < kernel->feature_out; feature_out++)
    {
        cur_para =  cnn_layer_info_get_weight(info, feature_out);
        in_data = in->data;
        for (h = 0; h < out->height; h++)
        {
            for (w = 0; w < out->width; w++)
            {
                sum = 0;
                page_start_pixel = in_data;
                a_kernel = cur_para;
                for (feature_in = 0; feature_in < in->used_cnt; feature_in++)
                {
                    sum += a_kernel[0] * page_start_pixel[0];
                    CHECK_IN_INT_RANGE(sum);
                    page_start_pixel += page_size;
                    a_kernel++;
                }

                cache  = (sum >> current_mulx) + kernel->bias[feature_out];
                *data_out = ACTIVE_VAlUE(cache);
                data_out++;
                in_data++;
            }
        }
    }
}
#endif



#if ASM_SPEEDUP
/*带有RELU激活函数*/
void cnn_k33_s22_p00_relu(cnn_layer_info_t *info, cnn_data_t *in, cnn_data_t *out)
{
    int feature_out ;
    short *cur_para;
    short *data_out = NULL;
    cnn_kernel_t *kernel = &info->cnn_kernel;
    int k =((in->width - 1) / 2)*((in->height - 1) / 2);

    cnn_data_alloc_output_mem(out, (in->width - 1) / 2, (in->height - 1) / 2, kernel->feature_out, data_out)
    out->used_cnt += kernel->feature_out;

    for (feature_out = 0; feature_out < kernel->feature_out; feature_out++)
    {
        cur_para = cnn_layer_info_get_weight(info, feature_out);
        cnn3x3_stride2_valid(cur_para,
                             in->data,
                             in->width,
                             in->height,
                             in->used_cnt,
                             data_out,
                             current_mulx,
                             kernel->bias[feature_out]<<current_mulx);
        relu_in_place(data_out,k);
        data_out += k;
    }
}
#else
/*带有RELU激活函数*/
void cnn_k33_s22_p00_relu(cnn_layer_info_t *info, cnn_data_t *in, cnn_data_t *out)
{
    int feature_out, w, h;
    short *in_data;
    short *cur_para;
    short *data_out = NULL;
    short cache;
    int page_size = in->width * in->height;
    int width, height;
    cnn_kernel_t *kernel = &info->cnn_kernel;
    int line_pad = 2 - (in->width % 2);

    width = (in->width - 1) >> 1;
    height = (in->height - 1) >> 1;
    cnn_data_alloc_output_mem(out, width, height, kernel->feature_out, data_out)
    out->used_cnt += kernel->feature_out;

    for (feature_out = 0; feature_out < kernel->feature_out; feature_out++)
    {
        cur_para = cnn_layer_info_get_weight(info, feature_out);
        in_data = in->data;
        for (h = 0; h < out->height; h++)
        {
            for (w = 0; w < out->width; w++)
            {
                /*
                   cnn+relu
                 */
                cache  = cnn_calc_n_k33(cur_para,
                                        in_data,
                                        in->width,
                                        in->used_cnt,
                                        page_size);
                cache += kernel->bias[feature_out];
                cache = ACTIVE_VAlUE(cache);
                *data_out = cache;
                data_out++;
                in_data += 2;
            }
            in_data += line_pad + in->width;
        }
    }
}
#endif



#if ASM_SPEEDUP
/*带有RELU激活函数*/
void cnn_k33_s11_p00_relu(cnn_layer_info_t *info, cnn_data_t *in, cnn_data_t *out)
{
    int feature_out ;
    short *cur_para;
    short *data_out = NULL;
    cnn_kernel_t *kernel = &info->cnn_kernel;
    int k =(in->width - 2)*(in->height - 2);

    cnn_data_alloc_output_mem(out, (in->width - 2), (in->height - 2), kernel->feature_out, data_out)
    out->used_cnt += kernel->feature_out;

    for (feature_out = 0; feature_out < kernel->feature_out; feature_out++)
    {
        cur_para = cnn_layer_info_get_weight(info, feature_out);
        cnn3x3(cur_para,
               in->data,
               in->width,
               in->height,
               in->used_cnt,
               data_out,
               current_mulx,
               kernel->bias[feature_out]<<current_mulx);
        relu_in_place(data_out,k);
        data_out += k;
    }
}
#else
/*带有RELU激活函数*/
void cnn_k33_s11_p00_relu(cnn_layer_info_t *info, cnn_data_t *in, cnn_data_t *out)
{
    int feature_out, w, h;
    short *in_data;
    short *cur_para;
    short *data_out = NULL;
    short cache;
    int page_size = in->width * in->height;
    int width, height;
    cnn_kernel_t *kernel = &info->cnn_kernel;

    width = (in->width - 2);
    height = (in->height - 2);
    cnn_data_alloc_output_mem(out, width, height, kernel->feature_out, data_out)
    out->used_cnt += kernel->feature_out;

    for (feature_out = 0; feature_out < kernel->feature_out; feature_out++)
    {
        cur_para = cnn_layer_info_get_weight(info, feature_out);
        in_data = in->data;
        for (h = 0; h < out->height; h++)
        {
            for (w = 0; w < out->width; w++)
            {
                /*
                   cnn+relu
                 */
                cache  = cnn_calc_n_k33(cur_para,
                                        in_data,
                                        in->width,
                                        in->used_cnt,
                                        page_size);
                cache += kernel->bias[feature_out];
                cache = ACTIVE_VAlUE(cache);
                *data_out = cache;
                data_out++;
                in_data++;
            }
            in_data += 2;
        }
    }
}
#endif


FORCE_INLINE  short  cnn_calc_pad_3_3(short *a_kernel,
                                      short *pixel,
                                      int line_size,
                                      int page_size,
                                      int feature_cnt,
                                      int height_pad,
                                      int width_pad,
                                      int bias)
{
    int feature_in;
    long long sum =0;
    int i=0,j=0;
    int  start_width=0,width_len;
    int  start_height=0,height_len;
    short *calc_kernel=a_kernel, *calc_pixel=pixel;
    int result;

    if(width_pad < 0)
    {
        start_width = abs(width_pad);
    }
    width_len = 3 - abs(width_pad);

    if(height_pad < 0)
    {
        start_height = abs(height_pad);
    }
    height_len = 3 - abs(height_pad);

    for(feature_in=0; feature_in<feature_cnt; feature_in++)
    {
        calc_pixel = pixel;
        for(i=0; i < height_len; i++)
        {
            calc_kernel = a_kernel + 3*(start_height+i)+start_width;
            for(j=0; j < width_len; j++)
            {
                sum +=calc_kernel[j] * calc_pixel[j];
                CHECK_IN_INT_RANGE(sum);
            }
            calc_pixel += line_size;
        }
        pixel+= page_size;
        a_kernel += 9;
    }
    result = (sum >> current_mulx) + bias;
    result = ACTIVE_VAlUE(result);
    return result;
}


#if ASM_SPEEDUP
void cnn_k33_s11_p11_relu(cnn_layer_info_t *info, cnn_data_t *in, cnn_data_t *out)
{
    int feature_out ;
    short *cur_para;
    short *data_out = NULL;
    cnn_kernel_t *kernel = &info->cnn_kernel;
    int k =in->width * in->height;

    cnn_data_alloc_output_mem(out, in->width, in->height, kernel->feature_out, data_out)
    out->used_cnt += kernel->feature_out;

    for (feature_out = 0; feature_out < kernel->feature_out; feature_out++)
    {
        cur_para = cnn_layer_info_get_weight(info, feature_out);
        cnn3x3_pad1(cur_para,
                    in->data,
                    in->width,
                    in->height,
                    in->used_cnt,
                    data_out,
                    current_mulx,
                    kernel->bias[feature_out]<<current_mulx);
        relu_in_place(data_out,k);
        data_out += k;

    }
}
#else
void cnn_k33_s11_p11_relu(cnn_layer_info_t *info, cnn_data_t *in, cnn_data_t *out)
{
    int feature_out, w, h;
    short *in_data;
    short *cur_para;
    short *data_out = NULL;
    short cache;
    int page_size = in->width * in->height;
    int width, height;
    cnn_kernel_t *kernel = &info->cnn_kernel;

    width =  in->width;
    height = in->height;
    cnn_data_alloc_output_mem(out, width, height, kernel->feature_out, data_out)
    out->used_cnt += kernel->feature_out;

    for (feature_out = 0; feature_out < kernel->feature_out; feature_out++)
    {
        cur_para = cnn_layer_info_get_weight(info, feature_out);
        in_data = in->data;
        *data_out = cnn_calc_pad_3_3(cur_para,
                                     in_data,
                                     in->width,
                                     page_size,
                                     in->used_cnt,
                                     -1,
                                     -1,
                                     kernel->bias[feature_out]);
        data_out++;
        for (w = 1; w < (out->width - 1); w++)
        {
            *data_out = cnn_calc_pad_3_3(cur_para,
                                         in_data,
                                         in->width,
                                         page_size,
                                         in->used_cnt,
                                         -1,
                                         0,
                                         kernel->bias[feature_out]);
            data_out++;
            in_data++;
        }
        *data_out =  cnn_calc_pad_3_3(cur_para,
                                      in_data,
                                      in->width, page_size,
                                      in->used_cnt,
                                      -1,
                                      1,
                                      kernel->bias[feature_out]);
        data_out++;
        in_data = in->data;
        for (h = 1; h < (out->height - 1); h++)
        {
            *data_out = cnn_calc_pad_3_3(cur_para,
                                         in_data,
                                         in->width,
                                         page_size,
                                         in->used_cnt,
                                         0,
                                         -1,
                                         kernel->bias[feature_out]);
            data_out++;
            for (w = 1; w < (out->width - 1); w++)
            {
                cache  = cnn_calc_n_k33(cur_para,
                                        in_data,
                                        in->width,
                                        in->used_cnt,
                                        page_size);
                cache += kernel->bias[feature_out];
                cache = ACTIVE_VAlUE(cache);
                *data_out = cache;
                data_out++;
                in_data++;
            }
            *data_out = cnn_calc_pad_3_3(cur_para,
                                         in_data,
                                         in->width,
                                         page_size,
                                         in->used_cnt,
                                         0,
                                         1,
                                         kernel->bias[feature_out]);
            data_out++;
            in_data += 2;
        }
        *data_out = cnn_calc_pad_3_3(cur_para,
                                     in_data,
                                     in->width,
                                     page_size,
                                     in->used_cnt,
                                     1,
                                     -1, kernel->bias[feature_out]);

        data_out++;
        for (w = 1; w < (out->width - 1); w++)
        {
            *data_out = cnn_calc_pad_3_3(cur_para,
                                         in_data,
                                         in->width,
                                         page_size,
                                         in->used_cnt,
                                         1,
                                         0,
                                         kernel->bias[feature_out]);
            data_out++;
            in_data++;
        }
        *data_out = cnn_calc_pad_3_3(cur_para,
                                     in_data,
                                     in->width,
                                     page_size,
                                     in->used_cnt,
                                     1,
                                     1,
                                     kernel->bias[feature_out]);

        data_out++;
    }
}
#endif


FORCE_INLINE  int  cnn_calc_pad_1_x_line(short *a_kernel,
        short *pixel,
        short *out,
        int cnt,
        int page_size,
        int line_size,
        int pad,
        int kernel_len,
        int bias)
{
    int feature_in,w=0;
    long long sum =0;
    int i=0;
    int len=0;
    short *calc_kernel=a_kernel, *calc_pixel;

    if(pad < 0)
    {
        a_kernel += abs(pad);
    }

    len  = kernel_len - abs(pad);

    for(w=0; w< line_size; w++)
    {
        calc_pixel = pixel;
        calc_kernel = a_kernel;
        for(feature_in=0; feature_in<cnt; feature_in++)
        {
            for(i=0; i < len; i++)
            {
                sum +=calc_kernel[i] * calc_pixel[i*line_size];
                CHECK_IN_INT_RANGE(sum);
            }
            calc_pixel += page_size;
            calc_kernel+=kernel_len;
        }
        *out = (sum >> current_mulx) + bias;
        sum = 0;
        out++;
        pixel++;
    }
    return 0;
}


#if ASM_SPEEDUP
void cnn_colunm_5_1_1_2_ge6_5(cnn_layer_info_t *info, cnn_data_t *in, cnn_data_t *out)
{
    int feature_out ;
    short *cur_para;
    short *data_out = NULL;
    cnn_kernel_t *kernel = &info->cnn_kernel;
    int k =in->width * in->height;

    cnn_data_alloc_output_mem(out, in->width, in->height, kernel->feature_out, data_out)
    out->used_cnt += kernel->feature_out;

    for (feature_out = 0; feature_out < kernel->feature_out; feature_out++)
    {
        cur_para = cnn_layer_info_get_weight(info, feature_out);
        cnn5x1_pad2(cur_para,
                    in->data,
                    in->width,
                    in->height,
                    in->used_cnt,
                    data_out,
                    current_mulx,
                    kernel->bias[feature_out]<<current_mulx);
        data_out += k;
    }
}
#endif
void cnn_colunm_5_1_1_2_x(cnn_layer_info_t *info, cnn_data_t *in, cnn_data_t *out)
{
    long long sum = 0;
    int feature_out, w, h, feature_in;    /*四个循环变量*/
    short *temp_data = NULL, *in_data = NULL, *data_out = NULL;
    short *cur_para;
    int  page_size = 0;
    int width, height;
    short *page_start_pixel;
    cnn_kernel_t *kernel = &info->cnn_kernel;
    short *a_input_kernel = NULL;
    int offset[MAX_DIGIT_CNT];

    page_size = in->width * in->height;
    width = in->width;
    height = in->height;
    cnn_data_alloc_output_mem(out, width, height, kernel->feature_out, data_out)
    out->used_cnt += kernel->feature_out;

    offset[0] = 0;
    offset[1] = in->width;
    offset[2] = in->width << 1;
    offset[3] = in->width * 3;
    offset[4] = in->width << 2;

    for (feature_out = 0; feature_out < kernel->feature_out; feature_out++)
    {
        cur_para = cnn_layer_info_get_weight(info, feature_out);
        in_data = in->data;

        cnn_calc_pad_1_x_line(cur_para,
                              in_data,
                              data_out,
                              in->used_cnt,
                              page_size,
                              in->width,
                              -2,
                              5,
                              kernel->bias[feature_out]);

        data_out += width;
        cnn_calc_pad_1_x_line(cur_para,
                              in_data,
                              data_out,
                              in->used_cnt,
                              page_size,
                              in->width,
                              -1,
                              5,
                              kernel->bias[feature_out]);
        data_out += width;
        for (h = 2; h < (out->height - 2); h++)
        {
            for (w = 0; w < out->width; w++)
            {
                a_input_kernel = cur_para;
                page_start_pixel = in_data;
                sum = 0;
                for (feature_in = 0; feature_in < in->used_cnt; feature_in++)
                {
                    temp_data = page_start_pixel;
                    sum += a_input_kernel[0] * temp_data[offset[0]];
                    CHECK_IN_INT_RANGE(sum);
                    sum += a_input_kernel[1] * temp_data[offset[1]];
                    CHECK_IN_INT_RANGE(sum);
                    sum += a_input_kernel[2] * temp_data[offset[2]];
                    CHECK_IN_INT_RANGE(sum);
                    sum += a_input_kernel[3] * temp_data[offset[3]];
                    CHECK_IN_INT_RANGE(sum);
                    sum += a_input_kernel[4] * temp_data[offset[4]];
                    CHECK_IN_INT_RANGE(sum);
                    page_start_pixel += page_size;
                    a_input_kernel += 5;
                }
                *data_out  = (sum >> current_mulx) +  kernel->bias[feature_out];
                data_out++;
                in_data++;
            }
        }

        cnn_calc_pad_1_x_line(cur_para,
                              in_data,
                              data_out,
                              in->used_cnt,
                              page_size,
                              in->width,
                              1,
                              5,
                              kernel->bias[feature_out]);
        in_data += width;
        data_out += width;
        cnn_calc_pad_1_x_line(cur_para,
                              in_data,
                              data_out,
                              in->used_cnt,
                              page_size,
                              in->width,
                              2,
                              5,
                              kernel->bias[feature_out]);
        in_data += width;
        data_out += width;
    }
}


#if ASM_SPEEDUP
void cnn_colunm_k51_s11_p20(cnn_layer_info_t *info, cnn_data_t *in, cnn_data_t *out)
{

    if ((in->width < 6)||(in->height < 5))
    {
        cnn_colunm_5_1_1_2_x(info,in,out);;
    }
    else
    {
        cnn_colunm_5_1_1_2_ge6_5(info,in,out);
    }
}
#else
void cnn_colunm_k51_s11_p20(cnn_layer_info_t *info, cnn_data_t *in, cnn_data_t *out)
{
    cnn_colunm_5_1_1_2_x(info,in,out);;
}
#endif


#if ASM_SPEEDUP
void cnn_colunm_k71_s11_p30(cnn_layer_info_t *info, cnn_data_t *in, cnn_data_t *out)
{
    int feature_out ;
    short *cur_para;
    short *data_out = NULL;
    cnn_kernel_t *kernel = &info->cnn_kernel;
    int k =in->width * in->height;

    cnn_data_alloc_output_mem(out, in->width, in->height, kernel->feature_out, data_out)
    out->used_cnt += kernel->feature_out;

    for (feature_out = 0; feature_out < kernel->feature_out; feature_out++)
    {
        cur_para = cnn_layer_info_get_weight(info, feature_out);
        cnn7x1_pad3(cur_para,
                    in->data,
                    in->width,
                    in->height,
                    in->used_cnt,
                    data_out,
                    current_mulx,
                    kernel->bias[feature_out]<<current_mulx);
        data_out += k;
    }
}
#else
void cnn_colunm_k71_s11_p30(cnn_layer_info_t *info, cnn_data_t *in, cnn_data_t *out)
{
    long long sum = 0;
    int feature_out, w, h, feature_in;    /*四个循环变量*/
    short *temp_data = NULL, *in_data = NULL, *data_out = NULL;
    short *cur_para;
    int  page_size = 0;
    int width, height;
    short *page_start_pixel;
    cnn_kernel_t *kernel = &info->cnn_kernel;
    short *a_input_kernel = NULL;
    int offset[MAX_DIGIT_CNT];

    page_size = in->width * in->height;
    width = in->width;
    height = in->height;
    cnn_data_alloc_output_mem(out, width, height, kernel->feature_out, data_out)
    out->used_cnt += kernel->feature_out;

    offset[0] = 0;
    offset[1] = in->width;
    offset[2] = in->width << 1;
    offset[3] = in->width * 3;
    offset[4] = in->width << 2;
    offset[5] = in->width * 5;
    offset[6] = in->width * 6;

    for (feature_out = 0; feature_out < kernel->feature_out; feature_out++)
    {
        cur_para = cnn_layer_info_get_weight(info, feature_out);
        in_data = in->data;

        cnn_calc_pad_1_x_line(cur_para,
                              in_data,
                              data_out,
                              in->used_cnt,
                              page_size,
                              in->width,
                              -3,
                              7,
                              kernel->bias[feature_out]);

        data_out += width;
        cnn_calc_pad_1_x_line(cur_para,
                              in_data,
                              data_out,
                              in->used_cnt,
                              page_size,
                              in->width,
                              -2,
                              7,
                              kernel->bias[feature_out]);
        data_out += width;
        cnn_calc_pad_1_x_line(cur_para,
                              in_data,
                              data_out,
                              in->used_cnt,
                              page_size,
                              in->width,
                              -1,
                              7,
                              kernel->bias[feature_out]);
        data_out += width;
        for (h = 3; h < (out->height - 3); h++)
        {
            for (w = 0; w < out->width; w++)
            {
                a_input_kernel = cur_para;
                page_start_pixel = in_data;
                sum = 0;
                for (feature_in = 0; feature_in < in->used_cnt; feature_in++)
                {
                    temp_data = page_start_pixel;
                    sum += a_input_kernel[0] * temp_data[offset[0]];
                    CHECK_IN_INT_RANGE(sum);
                    sum += a_input_kernel[1] * temp_data[offset[1]];
                    CHECK_IN_INT_RANGE(sum);
                    sum += a_input_kernel[2] * temp_data[offset[2]];
                    CHECK_IN_INT_RANGE(sum);
                    sum += a_input_kernel[3] * temp_data[offset[3]];
                    CHECK_IN_INT_RANGE(sum);
                    sum += a_input_kernel[4] * temp_data[offset[4]];
                    CHECK_IN_INT_RANGE(sum);
                    sum += a_input_kernel[5] * temp_data[offset[5]];
                    CHECK_IN_INT_RANGE(sum);
                    sum += a_input_kernel[6] * temp_data[offset[6]];
                    CHECK_IN_INT_RANGE(sum);
                    page_start_pixel += page_size;
                    a_input_kernel += 7;
                }
                *data_out  = (sum >> current_mulx) +  kernel->bias[feature_out];
                data_out++;
                in_data++;
            }
        }

        cnn_calc_pad_1_x_line(cur_para,
                              in_data,
                              data_out,
                              in->used_cnt,
                              page_size,
                              in->width,
                              1,
                              7,
                              kernel->bias[feature_out]);
        in_data += width;
        data_out += width;
        cnn_calc_pad_1_x_line(cur_para,
                              in_data,
                              data_out,
                              in->used_cnt,
                              page_size,
                              in->width,
                              2,
                              7,
                              kernel->bias[feature_out]);
        in_data += width;
        data_out += width;
        cnn_calc_pad_1_x_line(cur_para,
                              in_data,
                              data_out,
                              in->used_cnt,
                              page_size,
                              in->width,
                              3,
                              7,
                              kernel->bias[feature_out]);
        in_data += width;
        data_out += width;
    }
}
#endif


FORCE_INLINE  int  cnn_calc_pad_x_1(short *a_kernel,
                                    short *pixel,
                                    int cnt,
                                    int page_size,
                                    int pad,
                                    int kernel_len)
{
    int feature_in;
    long long sum =0;
    short *page_start_pixel = pixel;
    int i=0;
    int calc_len = kernel_len - abs(pad);

    if(pad < 0)
    {
        a_kernel += abs(pad);
    }

    for(feature_in=0; feature_in<cnt; feature_in++)
    {
        pixel = page_start_pixel;
        for(i =0; i < calc_len; i++)
        {
            sum +=a_kernel[i] * pixel[i];
            CHECK_IN_INT_RANGE(sum);
        }
        page_start_pixel += page_size;
        a_kernel+=kernel_len;
    }
    return sum >> current_mulx;
}


void cnn_row_k15_s11_p02_relu(cnn_layer_info_t *info, cnn_data_t *in, cnn_data_t *out)
{
    short cache = 0;
    long long sum = 0;
    int feature_out, w, h, feature_in;    /*四个循环变量*/
    short *temp_data = NULL, *in_data = NULL, *data_out = NULL;
    short *cur_para;
    int  page_size = 0;
    int width, height;
    short *page_start_pixel;
    cnn_kernel_t *kernel = &info->cnn_kernel;
    short *a_input_kernel = NULL;

    page_size = in->width * in->height;
    width = in->width;
    height = in->height;
    cnn_data_alloc_output_mem(out, width, height, kernel->feature_out, data_out)
    out->used_cnt += kernel->feature_out;

    for (feature_out = 0; feature_out < kernel->feature_out; feature_out++)
    {
        cur_para = cnn_layer_info_get_weight(info, feature_out);
        in_data = in->data;
        for (h = 0; h < out->height; h++)
        {
            *data_out = cnn_calc_pad_x_1(cur_para, in_data, in->used_cnt, page_size, -2, 5) + kernel->bias[feature_out];
            *data_out = ACTIVE_VAlUE(*data_out);
            data_out++;
            *data_out = cnn_calc_pad_x_1(cur_para, in_data, in->used_cnt, page_size, -1, 5) + kernel->bias[feature_out];
            *data_out = ACTIVE_VAlUE(*data_out);
            data_out++;
            for (w = 2; w < (out->width - 2); w++)
            {
                a_input_kernel = cur_para;
                page_start_pixel = in_data;
                sum = 0;
                for (feature_in = 0; feature_in < in->used_cnt; feature_in++)
                {
                    temp_data = page_start_pixel;
                    sum += a_input_kernel[0] * temp_data[0];
                    CHECK_IN_INT_RANGE(sum);
                    sum += a_input_kernel[1] * temp_data[1];
                    CHECK_IN_INT_RANGE(sum);
                    sum += a_input_kernel[2] * temp_data[2];
                    CHECK_IN_INT_RANGE(sum);
                    sum += a_input_kernel[3] * temp_data[3];
                    CHECK_IN_INT_RANGE(sum);
                    sum += a_input_kernel[4] * temp_data[4];
                    CHECK_IN_INT_RANGE(sum);
                    page_start_pixel += page_size;
                    a_input_kernel += 5;
                }
                cache = sum >> current_mulx; ;
                cache += kernel->bias[feature_out];
                *data_out = cache;
                *data_out  = ACTIVE_VAlUE(*data_out);
                data_out++;
                in_data++;
            }
            *data_out = cnn_calc_pad_x_1(cur_para, in_data, in->used_cnt, page_size, 1, 5) + kernel->bias[feature_out];
            *data_out = ACTIVE_VAlUE(*data_out);;
            in_data++;
            data_out++;
            *data_out = cnn_calc_pad_x_1(cur_para, in_data, in->used_cnt, page_size, 2, 5) + kernel->bias[feature_out];
            *data_out = ACTIVE_VAlUE(*data_out);;
            in_data++;
            data_out++;
            in_data += 2;
        }
    }
}

#if ASM_SPEEDUP
void cnn_row_1_7_1_3_relu_ge10(cnn_layer_info_t *info, cnn_data_t *in, cnn_data_t *out)
{
    int feature_out ;
    short *cur_para;
    short *data_out = NULL;
    cnn_kernel_t *kernel = &info->cnn_kernel;
    int k =in->width * in->height;

    cnn_data_alloc_output_mem(out, in->width, in->height, kernel->feature_out, data_out)
    out->used_cnt += kernel->feature_out;

    for (feature_out = 0; feature_out < kernel->feature_out; feature_out++)
    {
        cur_para = cnn_layer_info_get_weight(info, feature_out);
        cnn1x7_pad3(cur_para,
                    in->data,
                    in->width,
                    in->height,
                    in->used_cnt,
                    data_out,
                    current_mulx,
                    kernel->bias[feature_out]<<current_mulx);
        relu_in_place(data_out,k);
        data_out += k;

    }
}
#endif

void cnn_row_1_7_1_3_relu_x(cnn_layer_info_t *info, cnn_data_t *in, cnn_data_t *out)
{
    short cache = 0;
    long long sum = 0;
    int feature_out, w, h, feature_in;    /*四个循环变量*/
    short *temp_data = NULL, *in_data = NULL, *data_out = NULL;
    short *cur_para;
    int  page_size = 0;
    int width, height;
    short *page_start_pixel;
    cnn_kernel_t *kernel = &info->cnn_kernel;
    short *a_input_kernel = NULL;

    page_size = in->width * in->height;

    width = in->width;
    height = in->height;
    cnn_data_alloc_output_mem(out, width, height, kernel->feature_out, data_out)
    out->used_cnt += kernel->feature_out;

    for (feature_out = 0; feature_out < kernel->feature_out; feature_out++)
    {
        cur_para = cnn_layer_info_get_weight(info, feature_out);
        in_data = in->data;
        for (h = 0; h < out->height; h++)
        {
            *data_out = cnn_calc_pad_x_1(cur_para, in_data, in->used_cnt, page_size, -3, 7) + kernel->bias[feature_out];
            *data_out = ACTIVE_VAlUE(*data_out);
            data_out++;
            *data_out = cnn_calc_pad_x_1(cur_para, in_data, in->used_cnt, page_size, -2, 7) + kernel->bias[feature_out];
            *data_out = ACTIVE_VAlUE(*data_out);
            data_out++;
            *data_out = cnn_calc_pad_x_1(cur_para, in_data, in->used_cnt, page_size, -1, 7) + kernel->bias[feature_out];
            *data_out = ACTIVE_VAlUE(*data_out);
            data_out++;
            for (w = 3; w < (out->width - 3); w++)
            {
                a_input_kernel = cur_para;
                page_start_pixel = in_data;
                sum = 0;
                for (feature_in = 0; feature_in < in->used_cnt; feature_in++)
                {
                    temp_data = page_start_pixel;
                    sum += a_input_kernel[0] * temp_data[0];
                    CHECK_IN_INT_RANGE(sum);
                    sum += a_input_kernel[1] * temp_data[1];
                    CHECK_IN_INT_RANGE(sum);
                    sum += a_input_kernel[2] * temp_data[2];
                    CHECK_IN_INT_RANGE(sum);
                    sum += a_input_kernel[3] * temp_data[3];
                    CHECK_IN_INT_RANGE(sum);
                    sum += a_input_kernel[4] * temp_data[4];
                    CHECK_IN_INT_RANGE(sum);
                    sum += a_input_kernel[5] * temp_data[5];
                    CHECK_IN_INT_RANGE(sum);
                    sum += a_input_kernel[6] * temp_data[6];
                    CHECK_IN_INT_RANGE(sum);
                    page_start_pixel += page_size;
                    a_input_kernel += 7;
                }
                CHECK_IN_INT_RANGE(sum);
                cache = sum >> current_mulx;
                cache += kernel->bias[feature_out];
                *data_out = cache;
                *data_out  = ACTIVE_VAlUE(*data_out);
                data_out++;
                in_data++;
            }
            *data_out = cnn_calc_pad_x_1(cur_para, in_data, in->used_cnt, page_size, 1, 7) + kernel->bias[feature_out];
            *data_out = ACTIVE_VAlUE(*data_out);
            in_data++;
            data_out++;
            *data_out = cnn_calc_pad_x_1(cur_para, in_data, in->used_cnt, page_size, 2, 7) + kernel->bias[feature_out];
            *data_out = ACTIVE_VAlUE(*data_out);
            in_data++;
            data_out++;
            *data_out = cnn_calc_pad_x_1(cur_para, in_data, in->used_cnt, page_size, 3, 7) + kernel->bias[feature_out];
            *data_out = ACTIVE_VAlUE(*data_out);
            data_out++;
            in_data += 4;
        }
    }
}
#if ASM_SPEEDUP
void cnn_row_k17_s11_p03_relu(cnn_layer_info_t *info, cnn_data_t *in, cnn_data_t *out)
{

//not right,use asm to calculate 4 item a group
    if (in->width < 4+7-1)
    {
        cnn_row_1_7_1_3_relu_x(info,in,out);
    }
    else

    {
        cnn_row_1_7_1_3_relu_ge10(info,in,out);
    }
}
#else
void cnn_row_k17_s11_p03_relu(cnn_layer_info_t *info, cnn_data_t *in, cnn_data_t *out)
{
    cnn_row_1_7_1_3_relu_x(info,in,out);
}
#endif


#if ASM_SPEEDUP
void  pool_max_k33_s22_p00(cnn_layer_info_t *info, cnn_data_t *in, cnn_data_t *out)
{
    int j,k,ink;
    short *outdata;
    cnn_data_alloc_output_mem(out, (in->width - 1) / 2,  (in->height - 1) / 2, in->used_cnt, outdata);

    k = ((in->width - 1) / 2)*((in->height - 1) / 2);
    ink = in->width * in->height;

    for (j = 0; j < in->used_cnt ; j++)
    {
        pool3x3max_stride2_valid(&outdata[j*k], &in->data[j*ink], in->width, in->height);
    }
    out->used_cnt += in->used_cnt;
}

#else
void  pool_max_k33_s22_p00(cnn_layer_info_t *info, cnn_data_t *in, cnn_data_t *out)
{
    int i = 0, width, height;
    int dst_width =  (in->width - 1) / 2;
    int dst_height =  (in->height - 1) / 2;
    short *outdata;
    short *in_temp_data;
    short *in_data;
    short max_value = -32768;
    int x_pad = 2 - (in->width % 2);
    in_data = in->data;

    cnn_data_alloc_output_mem(out, dst_width, dst_height, in->used_cnt, outdata);

    for (i = 0; i < in->used_cnt; i++)
    {
        in_data = in->data + i * in->height * in->width;
        for (height = 0; height < dst_height; height++)
        {
            for (width = 0; width < dst_width; width++)
            {
                in_temp_data = in_data;
                max_value = in_temp_data[0];
                max_value = max(in_temp_data[1], max_value);
                max_value = max(in_temp_data[2], max_value);
                in_temp_data += in->width;
                max_value = max(in_temp_data[0], max_value);
                max_value = max(in_temp_data[1], max_value);
                max_value = max(in_temp_data[2], max_value);
                in_temp_data += in->width;
                max_value = max(in_temp_data[0], max_value);
                max_value = max(in_temp_data[1], max_value);
                max_value = max(in_temp_data[2], max_value);
                *outdata = max_value;
                outdata++;
                in_data += 2;
            }
            in_data +=  x_pad + in->width;
        }
        out->used_cnt++;
    }
}
#endif


#if ASM_SPEEDUP
void  pool_avg_k33_s22_p00(cnn_layer_info_t *info, cnn_data_t *in, cnn_data_t *out)
{
    int j,k,ink;
    short *out_data;

    cnn_data_alloc_output_mem(out, (in->width - 1) / 2, (in->height - 1) / 2, in->used_cnt, out_data);

    k = ((in->width - 1) / 2)*((in->height - 1) / 2);
    ink = in->width * in->height;

    for (j = 0; j < in->used_cnt ; j++)
    {
        pool3x3avg_stride2_valid(&out_data[j*k], &in->data[j*ink], in->width, in->height);
    }
    out->used_cnt += in->used_cnt;
}
#else
void  pool_avg_k33_s22_p00(cnn_layer_info_t *info, cnn_data_t *in, cnn_data_t *out)
{
    int i = 0, width, height;
    int dst_width =  (in->width - 1) / 2;
    int dst_height =  (in->height - 1) / 2;
    short *out_data;
    short *in_temp_data;
    short *in_data;
    int sum = 0;
    int line_pad = 2 - (in->width % 2);
    in_data = in->data;

    cnn_data_alloc_output_mem(out, dst_width, dst_height, in->used_cnt, out_data);

    for (i = 0; i < in->used_cnt; i++)
    {
        in_data = in->data + i * in->height * in->width;
        for (height = 0; height < dst_height; height++)
        {
            for (width = 0; width < dst_width; width++)
            {
                sum = 0;
                in_temp_data = in_data;
                sum += in_temp_data[0];
                CHECK_IN_INT_RANGE(sum);
                sum += in_temp_data[1];
                CHECK_IN_INT_RANGE(sum);
                sum += in_temp_data[2];
                CHECK_IN_INT_RANGE(sum);
                in_temp_data += in->width;
                sum += in_temp_data[0];
                CHECK_IN_INT_RANGE(sum);
                sum += in_temp_data[1];
                CHECK_IN_INT_RANGE(sum);
                sum += in_temp_data[2];
                CHECK_IN_INT_RANGE(sum);
                in_temp_data += in->width;
                sum += in_temp_data[0];
                CHECK_IN_INT_RANGE(sum);
                sum += in_temp_data[1];
                CHECK_IN_INT_RANGE(sum);
                sum += in_temp_data[2];
                CHECK_IN_INT_RANGE(sum);
                *out_data =  (sum + 5) / 9;
                out_data++;
                in_data += 2;
            }
            // stride 为2
            in_data += line_pad + in->width;
        }

        out->used_cnt++;
    }
}
#endif



#if ASM_SPEEDUP
void  pool_avg_k33_s11_p00(cnn_layer_info_t *info, cnn_data_t *in, cnn_data_t *out)
{
    int j,k,ink;
    short *outdata;
    cnn_data_alloc_output_mem(out,   in->width - 2, in->height - 2, in->used_cnt, outdata);

    k = (in->width - 2)*(in->height - 2);
    ink = in->width * in->height;

    for (j = 0; j < in->used_cnt ; j++)
    {
        pool3x3avg_valid(&outdata[j*k], &in->data[j*ink], in->width, in->height);
    }
    out->used_cnt += in->used_cnt;
}
#else
void  pool_avg_k33_s11_p00(cnn_layer_info_t *info, cnn_data_t *in, cnn_data_t *out)
{
    int i = 0, width, height;
    int dst_width =  in->width - 2;
    int dst_height = in->height - 2;
    short *out_data;
    short *in_temp_data;
    short *in_data;
    int sum = 0;
    in_data = in->data;

    cnn_data_alloc_output_mem(out, dst_width, dst_height, in->used_cnt, out_data);

    for (i = 0; i < in->used_cnt; i++)
    {
        in_data = in->data + i * in->height * in->width;
        for (height = 0; height < dst_height; height++)
        {
            for (width = 0; width < dst_width; width++)
            {
                sum = 0;
                in_temp_data = in_data;
                sum += in_temp_data[0];
                sum += in_temp_data[1];
                sum += in_temp_data[2];
                in_temp_data += in->width;
                sum += in_temp_data[0];
                sum += in_temp_data[1];
                sum += in_temp_data[2];
                in_temp_data += in->width;
                sum += in_temp_data[0];
                sum += in_temp_data[1];
                sum += in_temp_data[2];
                *out_data =  (sum + 5) / 9;
                out_data++;
                in_data++;
            }
            in_data += 2;
        }

        out->used_cnt++;
    }
}
#endif


void  pool_avg_k22_s22_p00(cnn_layer_info_t *info, cnn_data_t *in, cnn_data_t *out)
{
    int i = 0, width, height;
    int dst_width =  in->width/2;
    int dst_height = in->height/2;
    short *out_data;
    short *in_temp_data;
    short *in_data;
    int sum = 0;
    in_data = in->data;

    ASSERT(0 == in->width%2);
    ASSERT(0 == in->height%2);

    cnn_data_alloc_output_mem(out, dst_width, dst_height, in->used_cnt, out_data);

    for (i = 0; i < in->used_cnt; i++)
    {
        in_data = in->data + i * in->height * in->width;
        for (height = 0; height < dst_height; height++)
        {
            for (width = 0; width < dst_width; width++)
            {
                sum = 0;
                in_temp_data = in_data;
                sum += in_temp_data[0];
                sum += in_temp_data[1];
                in_temp_data = in_data + in->width;
                sum += in_temp_data[0];
                sum += in_temp_data[1];
                *out_data = (sum + 2) >>2;
                out_data++;
                in_data+=2;
            }
            in_data += in->width;
        }

        out->used_cnt++;
    }
}


void  pool_avg_k13_s11_p00(cnn_layer_info_t *info, cnn_data_t *in, cnn_data_t *out)
{
    int i = 0, width, height;
    int dst_width =  in->width - 2;
    int dst_height = in->height;
    short *out_data;
    short *in_temp_data;
    short *in_data;
    int sum = 0;
    in_data = in->data;

    cnn_data_alloc_output_mem(out, dst_width, dst_height, in->used_cnt, out_data);

    for (i = 0; i < in->used_cnt; i++)
    {
        in_data = in->data + i * in->height * in->width;
        for (height = 0; height < dst_height; height++)
        {
            for (width = 0; width < dst_width; width++)
            {
                sum = 0;
                in_temp_data = in_data;
                sum += in_temp_data[0];
                sum += in_temp_data[1];
                sum += in_temp_data[2];
                *out_data =  (sum + 1) / 3;
                out_data++;
                in_data++;
            }
            in_data += 2;
        }

        out->used_cnt++;
    }
}



FORCE_INLINE  int  cnn_calc_n_k33_p1x_d2x(short *a_kernel,
        short *pixel,
        int line_size,
        int feature_cnt,
        int page_size,
        int pad_h,
        int pad_w,
        int dialte_w)
{
    int feature_in;
    long long sum =0;
    int h=0,w=0;
    int  start_width=0,width_len;
    int  start_height=0,height_len;
    short *calc_kernel=a_kernel, *calc_pixel=pixel;

    if(pad_w < 0)
    {
        start_width = abs(pad_w);
    }
    width_len = 3 - abs(pad_w);

    if(pad_h < 0)
    {
        start_height = abs(pad_h);
    }
    height_len = 3 - abs(pad_h);

    for(feature_in=0; feature_in<feature_cnt; feature_in++)
    {
        for(h=0; h < height_len; h++)
        {
            calc_kernel = a_kernel + 3*(start_height+h) + start_width;
            calc_pixel = pixel + line_size*(start_height+h) + start_width*dialte_w;
            for(w=0; w < width_len; w++)
            {
                sum +=calc_kernel[w] * calc_pixel[w*dialte_w];
                CHECK_IN_INT_RANGE(sum);
            }
        }
        pixel += page_size;
        a_kernel += 9;
    }
    return sum >> current_mulx;
}

int cnn_get_current_pad(int current, int pad, int dilate,int kernel,int stride,int total,int in_width)
{
    if(0 == pad)
    {
        return 0;
    }

    if(current == 0)
    {
        return  -1;
    }
    else if(current == total-1)
    {
        int k_temp = dilate*kernel-1;
        int need_size = k_temp + stride*current;
        if(need_size <= (in_width+pad))
        {
            return 0;
        }
        else
        {
            return 1;
        }
    }
    else
    {
        return 0;
    }
}

void cnn_k33_s22_p1x_d2x_relu(cnn_layer_info_t *info, cnn_data_t *in, cnn_data_t *out)
{
    int feature_out, w, h;
    short *in_data;
    short *cur_para;
    short *data_out = NULL;
    short cache;
    int page_size = in->width * in->height;
    int width, height;
    cnn_kernel_t *kernel = &info->cnn_kernel;
    int pad_h;
    int pad_w;
    /*(width - (kernel-1))/2 并向上取整 */
    width = (in->width + kernel->pad[1]*2 - (kernel->width*kernel->dilate[1]-1) + 1 +1) >> 1;
    height = (in->height + kernel->pad[0]*2 - (kernel->height*kernel->dilate[0]-1) + 1 +1) >> 1;
    cnn_data_alloc_output_mem(out, width, height, kernel->feature_out, data_out)
    out->used_cnt += kernel->feature_out;

    for (feature_out = 0; feature_out < kernel->feature_out; feature_out++)
    {
        cur_para = cnn_layer_info_get_weight(info, feature_out);
        in_data = in->data;
        for (h = 0; h < out->height; h++)
        {
            pad_h = cnn_get_current_pad(h,1,2,3,2,out->height,in->height);
            for (w = 0; w < out->width; w++)
            {
                pad_w = cnn_get_current_pad(w,kernel->pad[1],kernel->dilate[1],3,2,out->width,in->width);
                /*
                   cnn+relu
                 */
                in_data = in->data + (h*2 - kernel->pad[0])*in->width + (w*2 - kernel->pad[1]);
                cache  = cnn_calc_n_k33_p1x_d2x(cur_para,
                                                in_data,
                                                in->width*2,
                                                in->used_cnt,
                                                page_size,
                                                pad_h,
                                                pad_w,
                                                kernel->dilate[1]);
                cache += kernel->bias[feature_out];
                cache = ACTIVE_VAlUE(cache);
                *data_out = cache;
                data_out++;
            }
        }
    }
}



void cbam_channel(cnn_layer_info_t *info,cnn_data_t *in,cnn_data_t *out)
{
    int i=0, w=0, channel_size = in->width *in->height;
    int sum = 0, max_value=0;
    short *data =NULL;
    short *mid_data1=NULL;
    int *mid_data2=NULL;
    signed short *current_weight = NULL;
    signed int *bias_weight = NULL;
    short *out_data;
    long long temp,temp2;
    int pad = channel_size/2;

    data = in->data;
    mid_data1 = mem_add_extra_size(in->data,in->used_cnt*2*2 + in->used_cnt*4*2);
    mid_data2 = (int *)(mid_data1 + in->used_cnt*2);

    current_weight =  mem_add_extra_size(in->data,info->full_connect.weight_size);
    dataflash_read(info->full_connect.weight_offset, current_weight, info->full_connect.weight_size);
    bias_weight = mem_add_extra_size(in->data,info->full_connect.bias_size);
    dataflash_read(info->full_connect.bias_offset, bias_weight, info->full_connect.bias_size);

    // avg and max
    for(i =0 ; i < in->used_cnt; i++)
    {
        sum = 0;
        data = in->data + i*channel_size;
        max_value = data[0];
        for(w=0; w<channel_size; w++)
        {
            sum += data[w];
            max_value = max(max_value, data[w]);
        }
        mid_data1[i] = (sum+pad)/channel_size;
        mid_data1[in->used_cnt+i] = max_value>>3;
    }

    update_cbam_mult(FULLCONNECT_MULX, info->mulx);
    // fc1 + relu
    for(i=0; i < in->used_cnt; i++)
    {
        // calc average
        temp = mult_short_short_to_int(current_weight,
                                       mid_data1,
                                       in->used_cnt);
        //fullconnect的参数比cnn参数小2^4, bias扩大最终参数的倍数
        temp = (temp >> current_mulx) + (bias_weight[i]);
        mid_data2[i] = 	ACTIVE_VAlUE(temp);
        temp = mult_short_short_to_int((current_weight),
                                       mid_data1+in->used_cnt,
                                       in->used_cnt);
        // calc max
        temp = (temp >> (current_mulx)) + bias_weight[i];
        mid_data2[i+ in->used_cnt] = ACTIVE_VAlUE(temp);
        current_weight += in->used_cnt;
    }

    bias_weight += in->used_cnt;

    //fc2 + add + tanh
    for(i=0; i < in->used_cnt; i++)
    {
        temp = mult_short_int_to_ll(current_weight,
                                    mid_data2,
                                    in->used_cnt);
        temp = (temp >> (current_mulx)) + bias_weight[i];
        temp2 = mult_short_int_to_ll(current_weight,
                                     mid_data2+in->used_cnt,
                                     in->used_cnt);
        temp2 = (temp2 >> (current_mulx)) + bias_weight[i];
        temp += temp2;
        mid_data1[i] = (tanh_int(temp,info->mulx)+(1<<info->mulx))>>1;
        current_weight += in->used_cnt;
    }

    mem_reduce_extra_size(in->data, info->full_connect.bias_size + info->full_connect.weight_size);

    cnn_data_alloc_output_mem(out, in->width, in->height,  in->used_cnt, out_data);
    out->used_cnt = in->used_cnt;

    // mult
    data = in->data;
    for(i =0 ; i < in->used_cnt; i++)
    {
        for(w=0; w < in->width*in->height; w++)
        {
            temp =  (mid_data1[i] * *data++);
            *out_data++ = temp>>info->mulx;
        }
    }

    restore_mult();
}


void cbam_spatial_avg_max(cnn_layer_info_t *info,cnn_data_t *in,cnn_data_t *out)
{
    int i=0, w=0;
    int sum = 0, max_value=0;
    short *data =NULL, *calc_data=NULL;
    int page_size = in->width * in->height;
    short *average_output=NULL, *max_output=NULL;

    cnn_data_alloc_output_mem(out,  in->width, in->height, 2, average_output);
    out->used_cnt = 2;

    max_output = average_output + page_size;
    data = in->data;

    for(w=0; w<page_size; w++)
    {
        calc_data = data;
        max_value = calc_data[0];
        sum = 0;
        for(i=0 ; i<in->used_cnt; i++)
        {
            sum += calc_data[0];
            max_value = max(max_value, calc_data[0]);
            calc_data += page_size;
        }
        average_output[0] = sum/in->used_cnt;
        max_output[0] = max_value>>3;
        average_output++;
        max_output++;
        data++;
    }
}


const cnn_operator_t  cnn_operator[] = {{OPERATOR_POOL_MAX_K33_S22_P00,pool_max_k33_s22_p00},
    {OPERATOR_POOL_AVG_K33_S22_P00,pool_avg_k33_s22_p00},
    {OPERATOR_POOL_AVG_K33_S11_P00,pool_avg_k33_s11_p00},
    {OPERATOR_POOL_AVG_K13_S11_P00,pool_avg_k13_s11_p00},
    {OPERATOR_CNN_K11_S11_P00_D11_RELU,cnn_k11_s11_p00_relu},
    {OPERATOR_CNN_K51_S11_P20_D11,cnn_colunm_k51_s11_p20},
    {OPERATOR_CNN_K15_S11_P02_D11_RELU,cnn_row_k15_s11_p02_relu},
    {OPERATOR_CNN_K71_S11_P30,cnn_colunm_k71_s11_p30},
    {OPERATOR_CNN_K17_S11_P03_D11_RELU,cnn_row_k17_s11_p03_relu},
    {OPERATOR_CNN_K33_S22_P00_D11_RELU,cnn_k33_s22_p00_relu},
    {OPERATOR_CNN_K33_S11_P00_D11_RELU,cnn_k33_s11_p00_relu},
    {OPERATOR_CNN_K33_S11_P11_D11_RELU,cnn_k33_s11_p11_relu},
    {OPERATOR_CNN_K33_S22_P11_D22_RELU,cnn_k33_s22_p1x_d2x_relu},
    {OPERATOR_CNN_K33_S22_P10_D21_RELU,cnn_k33_s22_p1x_d2x_relu},
    {OPERATOR_FULLCNNT, fullconnect_layer},
    {OPERATOR_CBAM_CHANNEL, cbam_channel},
    {OPERATOR_SPATIAL_AVG_MAX, cbam_spatial_avg_max},
    {OPERATOR_POOL_AVG_K22_S22_P00, pool_avg_k22_s22_p00},
};

void cnn_layer_calc_feature(cnn_layer_info_t *info,cnn_data_t *in,cnn_data_t *out)
{
    unsigned int i =0;

    for(i =0 ; i < ARRAY_SIZE(cnn_operator); i ++)
    {
        if(info->type == cnn_operator[i].type)
        {
            cnn_operator[i].operator(info, in,out);
            return;
        }
    }
    CNN_LOG_INT("not impelete operator %d",info->type);
}


