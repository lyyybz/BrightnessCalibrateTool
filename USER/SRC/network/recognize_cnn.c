#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "config.h"
#ifdef KEIL
#include "delay.h"
#include "main.h"
#include "usart.h"
#endif // !KEIL 
#include "recognize_cnn.h"
#include "comfunc.h"
#include "recognize.h"
#include <stdlib.h>
#include "hardware_layer.h"
#include "recognize_tools.h"
#include "cnn_para.h"
#include "mem_manage.h"
#include "protcl.h"
#include "cnn_operator.h"
#include "bmp_arithmetric.h"


#define CNN_MAGIC   0x12345678

void cnn_data_init_and_alloc(cnn_data_t *data,
                             char width,
                             char height,
                             char cnt)
{
    data->width = width;
    data->height = height;
    data->used_cnt =0;
    data->data = mem_alloc(width * height * cnt * sizeof(short));
}


void cnn_data_free_data(cnn_data_t *data)
{
    mem_free(data->data);
}

int cnn_calc_max_simple(cnn_data_t *in)
{
    int i,idx = 0;
    long long *data = (long long *)in->data;
    long long  maxll = data[0];

    for ( i = 0;  i <  in->width; i++)
    {
        if (maxll < data[i])
        {
            maxll = data[i];
            idx = i;
        }
    }

    return idx;
}


long long cnn_sum_and_clr_around(cnn_data_t *in,int idx)
{
    unsigned int  *data_out =(unsigned int *)in->data;
    long long sum_around =  data_out[idx];
    data_out[idx] = 0;
    if(idx == (in->width-1))
    {
        sum_around += data_out[0];
        data_out[0] = 0;
    }
    else
    {
        sum_around += data_out[idx+1];
        data_out[idx+1] = 0;
    }

    if(idx == 0)
    {
        sum_around += data_out[in->width-1];
        data_out[in->width-1] = 0;
    }
    else
    {
        sum_around += data_out[idx-1];
        data_out[idx-1] = 0;
    }

    sum_around <<= LAYER2_MULX; // 将结果整体扩大8192倍
    return sum_around;
}

int cnn_calc_use_softmax(cnn_data_t *in,cnn_result_t *result)
{
    int i,idx = 0;
    unsigned int maxll = 0;
    unsigned int sum_all = 0;
    long long sum_around = 0;
    unsigned int probility;
    unsigned int  *data_out =(unsigned int *)in->data;

    layer_exp_activate(in);

    maxll = data_out[0];

    for ( i = 0;  i <  in->width; i++)
    {
        sum_all += data_out[i];
        if (maxll < data_out[i])
        {
            maxll = data_out[i];
            idx = i;
        }
    }

    if(sum_all == 0)
    {
        return -1;
    }

    maxll = data_out[0];
    sum_around = cnn_sum_and_clr_around(in,idx);
    probility =  sum_around/sum_all;
    result->max_probility  = probility*100/128;
    result->digit = idx;

    for ( i = 0;  i <  in->width; i++)
    {
        if (maxll < data_out[i])
        {
            maxll = data_out[i];
            idx = i;
        }
    }

    sum_around = cnn_sum_and_clr_around(in,idx);
    probility =  sum_around/sum_all;
    result->second_probility= probility*100/128;

    return result->digit;
}


#if !defined ESSN_WCR_P
static int  _pre_handle_bmp(cnn_data_t *out,unsigned char *in)
{
    int maxv = 0xff, minv=0,mean_gray=0,min_enarge;
    int y;
    int sum =0, ret =0;
    short *data=out->data;
    int temp=0;
    out->used_cnt =1;

    ret = equalize_hist(in,
                        DIGIT_WIDTH,
                        DIGIT_HEIGHT,
                        DIGIT_WIDTH);
    if(ret)
    {
        return -1;
    }

    for (y = 0 ; y<DIGIT_HEIGHT*DIGIT_WIDTH; y++)
    {
        data[y] =  (maxv - in[y])<< BMP_ENLARGE;
        sum += data[y];
    }

    mean_gray = (sum)/(DIGIT_HEIGHT*DIGIT_WIDTH);
    data = out->data;
    for (y = 0 ; y < DIGIT_HEIGHT*DIGIT_WIDTH; y++)
    {
        *data -= mean_gray;
        data++;
    }
    CNN_LOG_FEATUEE(NULL,out);
    return 0;
}
#else
static int  _pre_handle_bmp(cnn_data_t *out,unsigned char *in)
{
    int maxv = 0xff, minv=0,mean_gray=0,min_enarge;
    int y;
    int sum =0;
    short *data=out->data;
    out->used_cnt =1;

    maxv = in[0];
    minv = maxv;
    for (y=0 ; y<DIGIT_HEIGHT*DIGIT_WIDTH; y++)
    {
        maxv = max(maxv, in[y]);
        minv = min(minv, in[y]);
        data[y] = in[y] << (BMP_ENLARGE+8);
    }

    if(maxv==minv)
    {
        return -1;
    }
    min_enarge = minv << (BMP_ENLARGE+8);
    for (y = 0 ; y < DIGIT_HEIGHT*DIGIT_WIDTH; y++)
    {
        *data = ((unsigned short)(*data) - min_enarge+(maxv-minv)/2)/(maxv-minv);
        *data = (1<<(BMP_ENLARGE+8)) - *data;
        data++;
    }

    data = out->data;
    for (y = 0 ; y< DIGIT_HEIGHT*DIGIT_WIDTH; y++)
    {
        sum += data[y];
    }

    mean_gray = (sum)/(DIGIT_HEIGHT*DIGIT_WIDTH);
    for (y = 0 ; y< DIGIT_HEIGHT*DIGIT_WIDTH; y++)
    {
        *data -= mean_gray;
        data++;
    }

    CNN_LOG_FEATUEE(NULL,out);
    return 0;
}
#endif

void cnn_calc_road(deep_structure_t *struture,cnn_para_index_t *index,cnn_data_t *in, cnn_data_t *out)
{
    int i =0;
    cnn_layer_info_t info;
    cnn_data_t temp_in=CNN_NULL_DATA;
    cnn_data_t temp_out=CNN_NULL_DATA;
    int depth = struture->depth_cnt[index->layer][index->road];
    cnn_layer_info_t first_layer_info;

    ASSERT(depth >=1);
    index->depth =0;
    if(1 == depth)
    {
        cnn_layer_info_sync_load(struture,index,&info,in,in->used_cnt);
        cnn_layer_calc_feature(&info,in,out);
        CNN_LOG_FEATUEE(index,out);
        cnn_layer_info_free_para(&info,in);
    }
    else if(2 == depth)
    {
        cnn_layer_info_sync_load(struture,index,&first_layer_info,out,in->used_cnt);
        mem_set_mode(MEM_INNER_MODE, depth,in->data);
        cnn_layer_calc_feature(&first_layer_info,in,&temp_in);
        CNN_LOG_FEATUEE(index,&temp_in);
        index->depth =1;
        cnn_layer_info_sync_load(struture,index,&info,&temp_in,temp_in.used_cnt);
        mem_set_mode(MEM_OUTER_MODE,depth,in->data);
        cnn_layer_info_free_para(&first_layer_info,out);
        cnn_layer_calc_feature(&info,&temp_in,out);
        CNN_LOG_FEATUEE(index,out);
        //前换到外层内存时，内层在下次使用的时候便会被复用，不需要亦无法手动释放。
    }
    else
    {
        cnn_layer_info_sync_load(struture,index,&first_layer_info,out,in->used_cnt);
        mem_set_mode(MEM_INNER_MODE, depth,in->data);
        cnn_layer_calc_feature(&first_layer_info,in,&temp_in);
        CNN_LOG_FEATUEE(index,&temp_in);

        /*中间层特征*/
        for(i=1; i < (depth-1); i++)
        {
            index->depth =i;
            memset(&temp_out,0,sizeof(temp_out));
            cnn_layer_info_sync_load(struture,index,&info,&temp_in,temp_in.used_cnt);
            cnn_layer_calc_feature(&info,&temp_in,&temp_out);
            CNN_LOG_FEATUEE(index,&temp_out);
            cnn_layer_info_free_para(&info,&temp_in);
            cnn_data_free_data(&temp_in);
            temp_in = temp_out;
        }

        index->depth = depth-1;
        cnn_layer_info_sync_load(struture,index,&info,&temp_in,temp_in.used_cnt);
        mem_set_mode(MEM_OUTER_MODE,depth,NULL);
        cnn_layer_info_free_para(&first_layer_info,out);
        cnn_layer_calc_feature(&info,&temp_in,out);
        CNN_LOG_FEATUEE(index,out);
    }

    mem_free_inner_memory();
}

void  cbam_spatial_active_mult(cnn_layer_info_t *info, cnn_data_t *in, cnn_data_t *para,  cnn_data_t *out)
{
    int i =0, w=0;
    short *data =NULL, *output_data=NULL;
    int page_size = in->width * in->height;
    short *para_data = NULL;
    int temp;

    cnn_data_alloc_output_mem(out,  in->width, in->height, in->used_cnt, output_data);
    out->used_cnt = in->used_cnt;

    para_data = para->data;

    for(i=0; i < page_size; i++)
    {
        para_data[i] = (tanh_int(para_data[i], info->mulx)+(1<<info->mulx))/2;
    }

    data = in->data;
    int index = 0;
    for(w=0; w<page_size; w++)
    {
        for(i=0; i<in->used_cnt; i++)
        {
            temp = data[i*page_size] * para_data[0];
            output_data[i*page_size] = temp>>info->mulx;

        }
        index++;
        data++;
        para_data++;
        output_data++;
    }
}


void cbam_spatial_active(deep_structure_t *struture, cnn_data_t *in, cnn_data_t *out, int layer)
{
    cnn_layer_info_t info, first_layer_info;
    cnn_para_index_t index = {0,0,0,0};
    int i=0;
    cnn_data_t temp_in=CNN_NULL_DATA;
    cnn_data_t temp_out=CNN_NULL_DATA;

    // max && average
    index.layer = layer;
    index.road = -2;
    index.depth = 0;
    cnn_layer_info_sync_load(struture, &index, &first_layer_info, out, in->used_cnt);
    mem_set_mode(MEM_INNER_MODE, 5, in->data);
    cnn_layer_calc_feature(&first_layer_info, in, &temp_in);
    CNN_LOG_FEATUEE(&index, &temp_in);

    update_cbam_mult(CNN_MULX, first_layer_info.mulx);

    /*中间层特征*/
    for(i=1; i < 4; i++)
    {
        memset(&temp_out,0,sizeof(temp_out));
        cnn_layer_info_sync_load(struture, &index, &info, &temp_in, temp_in.used_cnt);
        cnn_layer_calc_feature(&info, &temp_in, &temp_out);
        index.depth++;
        CNN_LOG_FEATUEE(&index,&temp_out);
        cnn_layer_info_free_para(&info,&temp_in);
        cnn_data_free_data(&temp_in);
        temp_in = temp_out;
    }
    mem_set_mode(MEM_OUTER_MODE, 3, NULL);
    cnn_layer_info_free_para(&first_layer_info, out);
    // consume one operator
    index.depth++;
    cnn_layer_info_sync_load(struture,&index,&info,&temp_in,temp_in.used_cnt);
    cbam_spatial_active_mult(&info, in, &temp_in,out);
    CNN_LOG_FEATUEE(&index,out);

    mem_free_inner_memory();
    restore_mult();
}


void cbam_active(deep_structure_t *struture, cnn_data_t *in, cnn_data_t *out, int layer)
{
    cnn_layer_info_t info;
    cnn_para_index_t index = {0,0,0,0};
    cnn_data_t temp= CNN_NULL_DATA;

    //channel attention
    index.layer = layer;
    index.road = -1;
    index.depth = 0;
    cnn_layer_info_sync_load(struture, &index, &info, in, in->used_cnt);
    cnn_layer_calc_feature(&info, in, &temp);
    cnn_layer_info_free_para(&info, in);
    CNN_LOG_FEATUEE(&index, &temp);
    cnn_data_free_data(in);

    //spatial attention
    cbam_spatial_active(struture, &temp, out, layer);
    cnn_data_free_data(&temp);
    memset(&temp, 0, sizeof(temp));
}

static int _recognize_gray_cnn(cnn_data_t *in,deep_structure_t *struture,cnn_result_t *result)
{
    int i=0,j=0;
    cnn_para_index_t para_index;
    cnn_data_t temp_out=CNN_NULL_DATA;
    cnn_data_t temp_in= *in;
    cnn_layer_info_t info;

    for(i=0; i < struture->layer_cnt; i++)
    {
        para_index.layer = i;
        para_index.road = 0;
        para_index.depth =0;
        for(j=0; j < struture->road_cnt[i]; j++)
        {
            para_index.road = j;
            para_index.depth =0;
            cnn_calc_road(struture,&para_index,&temp_in,&temp_out);
        }
        cnn_data_free_data(&temp_in);
        temp_in = temp_out;
        memset(&temp_out,0,sizeof(temp_out));


        cbam_active(struture, &temp_in, &temp_out, i);
        temp_in = temp_out;
        memset(&temp_out,0,sizeof(temp_out));

    }

    para_index.road = 0;
    para_index.depth =0;
    para_index.layer = POOL_AVERGAE_LAYER;
    cnn_layer_info_sync_load(struture,&para_index,&info,&temp_in,temp_in.used_cnt);
    cnn_layer_calc_feature(&info,&temp_in,&temp_out);
    CNN_LOG_FEATUEE(&para_index,&temp_out);
    cnn_data_free_data(&temp_in);
    temp_in = temp_out;
    memset(&temp_out,0,sizeof(temp_out));
    para_index.road = 0;
    para_index.depth =0;
    para_index.layer = FULLCONNECT_LAYER;
    cnn_layer_info_sync_load(struture,&para_index,&info,&temp_in,temp_in.used_cnt);
    cnn_layer_calc_feature(&info,&temp_in,&temp_out);
    CNN_LOG_FEATUEE(&para_index,&temp_out);
    cnn_data_free_data(&temp_in);
    result->digit= cnn_calc_use_softmax(&temp_out,result);
    cnn_data_free_data(&temp_out);
    return  result->digit;
}

/*识别单个数字*/
int  recognize_a_gray_digit_cnn(int digit_id,int net_id,cnn_result_t *result)
{
    deep_structure_t cnn_para;
    int number;
    cnn_data_t data_pre_calc;
    unsigned char *raw_data = NULL;
    mem_init();
    if(cnn_para_check_and_load_postion(&cnn_para,net_id))
    {
        return ERROR_NO_PARA;
    }

    raw_data = mem_alloc(DIGIT_WIDTH*DIGIT_HEIGHT);
    read_bmp(raw_data,digit_id);

    cnn_data_init_and_alloc(&data_pre_calc,
                            DIGIT_WIDTH,
                            DIGIT_HEIGHT,
                            1);

    if(_pre_handle_bmp(&data_pre_calc,raw_data))
    {
        return ERROR_CAMERA;
    }

    mem_free(raw_data);
    data_pre_calc.used_cnt = 1;

    number = _recognize_gray_cnn(&data_pre_calc,&cnn_para,result);

    return  number;
}

