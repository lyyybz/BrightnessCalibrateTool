#include "cnn_para.h"
#include "recognize_cnn.h"
#include "comfunc.h"
#include "string.h"
#include "stdlib.h"
#include "mem_manage.h"
#include "config.h"
#include "cnn_operator.h"



int current_mulx = BMP_LAYER_MULX;
static int last_layer_mulx = BMP_LAYER_MULX;
static int  _backup_mulx = -1;

int  cnn_para_check_and_load_postion(deep_structure_t *para,int id)
{
    para->used_head_cnt = 0;
    para->id = id;
    dataflash_read(CNN_NET_ADDR+id*PARA_HEAD_SIZE,(void *)para,offsetof(deep_structure_t,used_head_cnt ));

    if(memcmp(para->version,CNN_VERSION,8))
    {
        return -1;
    }
    return 0;
}

void update_mulx(cnn_para_index_t *index,int mulx)
{
    /*
    	为每层feature制定info->mulx是为防止feature会因为数值太小导致计算不准确。
    	和数值太大导致计算过程中的溢出。
    	参数默认都扩大10倍。
    	feature扩大的倍数 = input扩大的倍数 +参数扩大的倍数 - feature缩小倍数。
    	所以:
    	feature缩小倍数 = input扩大的倍数 +参数扩大的倍数 - feature扩大的倍数
    */
    int weight_mulx = CNN_MULX;
    if(0 == index->depth)
    {
        switch(index->layer)
        {
        case 0:
            last_layer_mulx = BMP_LAYER_MULX;
            break;
        case 1:
            last_layer_mulx = LAYER0_MULX;
            break;
        case 2:
            last_layer_mulx = LAYER1_MULX;
            break;
        default:
            last_layer_mulx = LAYER2_MULX;
            break;
        }

    }

    if(FULLCONNECT_LAYER == index->layer)
    {
        weight_mulx = FULLCONNECT_MULX;
    }

    current_mulx = last_layer_mulx+weight_mulx- mulx;
    //ASSERT(current_mulx >=0);
    last_layer_mulx= mulx;

}


/*将参数加载到内存中*/
int  cnn_layer_info_sync_load(deep_structure_t *para,
                              cnn_para_index_t *index,
                              cnn_layer_info_t *info,
                              cnn_data_t *data,
                              int feature_in)

{
    unsigned char *kernel_buff = NULL;
    int head_address = para->index_offset+ para->used_head_cnt*PARA_INDEX_SIZE;

    para->used_head_cnt++;
    ASSERT((offsetof(cnn_layer_info_t,alloc_size)) <= PARA_INDEX_SIZE);
    memset(info,0,sizeof(cnn_layer_info_t));
    dataflash_read(head_address,info,offsetof(cnn_layer_info_t,alloc_size));

    // 小于0 使用cbam的mult
    if (_backup_mulx < 0)
        update_mulx(index,info->mulx);

    info->feature_in = feature_in;
    if(info->type < OPERATOR_FULLCNNT)
    {
        int weight_size = info->cnn_kernel.width*info->cnn_kernel.height*feature_in*2;
        int bias_size = info->cnn_kernel.bias_size;
        int weight_offset = info->cnn_kernel.weight_offset;
        int bias_offset = info->cnn_kernel.bias_offset;
        info->alloc_size  = weight_size*2  + DMA_EXTRA_SIZE*2 + bias_size;
        if(NULL == data->data)
        {
            kernel_buff = mem_alloc(info->alloc_size);
            data->data =  (short *)kernel_buff;
        }
        else
        {
            kernel_buff = mem_add_extra_size(data->data,info->alloc_size);
        }

        if(0 != bias_size)
        {
            info->cnn_kernel.bias  = (short *)(kernel_buff +weight_size*2+ DMA_EXTRA_SIZE*2);
            dataflash_read(bias_offset,info->cnn_kernel.bias,bias_size);
        }

        if(0 != weight_size)
        {
            info->cnn_kernel.front_weight = (short *)kernel_buff;
            info->cnn_kernel.back_weight = (short *)(kernel_buff + weight_size+ DMA_EXTRA_SIZE);
            dataflash_start_read_prefix(weight_offset,(uint8_t *)info->cnn_kernel.front_weight,weight_size);
        }

    }
    else if((OPERATOR_POOL_AVG_K33_S22_P00 == info->type) || (OPERATOR_POOL_MAX_K33_S22_P00==info->type) )
    {
        return 0;
    }
    else if(OPERATOR_NORM_RELU == info->type)
    {
        batchnorm_t *batch = &info->batch_norm;
        info->alloc_size  = info->batch_norm.ampifier_size + info->batch_norm.beta_size + info->batch_norm.mean_size;
        if(NULL == data->data)
        {
            kernel_buff = mem_alloc(info->alloc_size);
            data->data = (short *)kernel_buff;
        }
        else
        {
            kernel_buff = mem_add_extra_size(data->data,info->alloc_size);
        }

        batch->mean = (short *)kernel_buff;
        dataflash_read(batch->mean_offset,batch->mean,batch->mean_size);
        kernel_buff += batch->mean_size;
        batch->ampifier= (int *)kernel_buff;
        dataflash_read(batch->ampifier_offset,batch->ampifier,batch->ampifier_size);
        kernel_buff += batch->ampifier_size;
        batch->beta= (short *)kernel_buff;
        dataflash_read(batch->beta_offset,batch->beta,batch->beta_size);
    }

    return 0;
}

short *cnn_layer_info_get_weight(cnn_layer_info_t *info,int feature)
{
    short *temp;

    if(feature == 0)
    {
        temp = info->cnn_kernel.front_weight;
    }
    else
    {
        temp = info->cnn_kernel.back_weight;
        info->cnn_kernel.back_weight = info->cnn_kernel.front_weight;
        info->cnn_kernel.front_weight = temp;
    }

    dataflash_wait_read_end();

    if((feature+1) < info->cnn_kernel.feature_out)
    {
        int weight_size = info->cnn_kernel.width*info->cnn_kernel.height*2 *info->feature_in;
        int next_address = info->cnn_kernel.weight_offset +  weight_size*(feature+1);
        dataflash_start_read_prefix(next_address,(uint8_t *)info->cnn_kernel.back_weight,weight_size);
    }

    return temp+2;
}

void cnn_layer_info_free_para(cnn_layer_info_t *info,cnn_data_t *data)
{

    if(0 == info->alloc_size)
    {
        return;
    }

    if(0 == data->used_cnt)
    {
        mem_free(data->data);
        memset(data,0,sizeof(cnn_data_t));
    }
    else
    {
        mem_reduce_extra_size(data->data,info->alloc_size);
    }
}


void update_cbam_mult(int weight_mulx, int layer_mulx)
{
    _backup_mulx = current_mulx;
    current_mulx = weight_mulx;
}

void restore_mult()
{
    current_mulx = _backup_mulx;
    _backup_mulx = -1;
}
