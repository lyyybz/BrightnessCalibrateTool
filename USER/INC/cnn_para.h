#ifndef _CNN_PARA_H 
#define _CNN_PARA_H 

#ifdef __cplusplus
 extern "C" {
#endif

#define PARA_HEAD_SIZE   100


#define PARA_INDEX_SIZE  50

/*
head1:
    Flash存储分为三部分:数据头、参数索引区、和参数信息

    1 数据头:(0-50字节)
    网络1  特征标识符:日期(8) +  种类  + 序号 (12字节)
      201601020101  + copy flag + layer_cnt +road_cnt +depth_cnt+offset
    网络2  特征标识符:日期(8) +  种类  + 序号 (12字节)
      201601020101  + copy flag + layer_cnt +road_cnt +depth_cnt+offset

    2 参数索引区 para head index: head排放顺序为 layer , road ,depth 每个head占用30个字节。

    head的结构:type，类型详细信息

    3 参数信息

head2:
    种类分为三种：
    01：正常版本：
    02：special 86
*/
#define CNN_MULX  (10)


#ifdef ESSN_WCR_P
    #define BMP_LAYER_MULX (7)
    #define LAYER0_MULX   (7)
    #define LAYER1_MULX   (8)
    #define LAYER2_MULX   (7)
    #define FULLCONNECT_MULX (6)
    #define CNN_VERSION   "20190423"
#endif

#if defined(ESSN_WCR_A) || defined(ESSN_WCR_B)
    #define BMP_LAYER_MULX (10)
    #define LAYER0_MULX  (10)
    #define CNN_VERSION   "20190423"
    #define LAYER1_MULX  (8)
    #define LAYER2_MULX  (7)
    #define FULLCONNECT_MULX (6)
#endif


#define  LAYER_MAX_CNT          4
#define  LAYER_ROAD_MAX_DEPTH  8
#define  PARA_HEAD_SIZE  100

#ifdef KEIL
    #pragma anon_unions
#endif


//32
 typedef struct  cnn_kernel_type  cnn_kernel_t;
 struct cnn_kernel_type
 {
    int weight_offset;
    int weight_size;
	int bias_offset;
	int bias_size;
	short width;
    short height;
	int feature_out;
    unsigned char dilate[2];
    unsigned char pad[2];
	union
	{
		short (*kerenl3_3)[3][3];
		short (*kerenl5_1)[5][1];
		short (*kerenl1_5)[1][5];
		short (*kerenl1_7)[1][7];
	    short (*kerenl7_1)[7][1];
		short *front_weight;
    };
    short *back_weight;
    union
    {
    	short (*kerenl10)[10];
		short *bias;
    };
 };


 typedef  struct fullconnect_type  fullconnect_t;
 struct fullconnect_type 
 {
    int weight_offset;
    int weight_size;
	int bias_offset;
	int bias_size;
    int hidden;
    short *weight;
	short *bias;
 };



typedef struct batchnorm_type  batchnorm_t;
struct batchnorm_type
{
	int mean_offset;
	int mean_size;
	int ampifier_offset;
	int ampifier_size;
	int beta_offset;
	int beta_size;
	
	short *mean;
	int *ampifier;
	short *beta;

};

typedef struct pool_type  pool_t;
struct pool_type
{
  short width;
  short height;
  short stride;
};

typedef struct cnn_layer_info_type
{
	int type;
	int mulx;
	union
	{   
		cnn_kernel_t  cnn_kernel;
		fullconnect_t full_connect;
		batchnorm_t  batch_norm;
		pool_t     pool;
	};
	int alloc_size;
	int feature_in;
}cnn_layer_info_t;


typedef struct  deep_structure_type  deep_structure_t;
struct deep_structure_type
{
    char version[12]; //version and magic
    int index_offset;
	int layer_cnt;
    int road_cnt[LAYER_MAX_CNT];
	unsigned char  depth_cnt[LAYER_MAX_CNT][LAYER_ROAD_MAX_DEPTH];
	unsigned int used_head_cnt;
	unsigned int id;
};


typedef struct cnn_para_index_type  cnn_para_index_t;
struct cnn_para_index_type
{
	int layer;
    int road;
	int depth;
	int feature;
};


typedef struct  cnn_data_type  cnn_data_t;
struct cnn_data_type 
{
    int width;
    int height;
    int used_cnt;
    union
    {
		short *data;
		short (*data_60_32)[60][32];
		short (*data_29_15)[29][15];
	    short (*data_14_7)[6][14][7];
	    short (*data_12_5)[6][12][5];
        short (*data_8_8)[32][8][8];
        short (*data_21_21)[9][21][21];
        short (*data_10_10)[20][10][10];
	    int (*int_data)[100];
	};
};


int  cnn_para_check_and_load_postion(deep_structure_t *para,int id);


/*将参数加载到内存中
   feature  < 0 : 加载所有的参数
   data_add_to: NULL 表示不需要分配内存
*/

int  cnn_layer_info_sync_load(deep_structure_t *para,
                                    cnn_para_index_t *index,
                                    cnn_layer_info_t *info,
                                    cnn_data_t *data,
                                    int feature_in);


int  cnn_layer_info_load_data_as_para(deep_structure_t *para,
                                    cnn_para_index_t *index,
                                    cnn_layer_info_t *info,
                                    cnn_data_t *data,
                                    int feature_in);


short *cnn_layer_info_get_weight(cnn_layer_info_t *info,int feature);

void cnn_layer_info_free_para(cnn_layer_info_t *info,cnn_data_t *data_add_to);

void update_cbam_mult(int weight_mult, int layer_mult);
void restore_mult(void);

extern int current_mulx;


#ifdef __cplusplus
  }
#endif

#endif 
