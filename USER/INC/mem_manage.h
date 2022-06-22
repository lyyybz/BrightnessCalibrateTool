#ifndef __MEM_MANAGE_H
#define __MEM_MANAGE_H

#ifdef __cplusplus
 extern "C" {
#endif

enum
{
  MEM_INNER_MODE,
  MEM_OUTER_MODE
};

#define MEM_SIZE  (1024*16+1000)
//#define MEM_SIZE  (1024*50+1000)

/*
 将16k内存分为两部分，头部和尾部。
 因为图像处理像管道一样，
 同时只会同时缓存输入和输出两个buff，
 所以这种简化版本的内存管理方式比较适合。
*/

#define MEM_MAX_BLOCK  2

typedef struct mem_manage_type  mem_manage_t;
typedef struct mem_block_type mem_block_t;

struct mem_block_type 
{
	char is_used;
	int size;
	void *data;
};

struct mem_manage_type
{
	int  left_size;
	int  size;
	unsigned char  first_alloc;
	unsigned char  tail_first; 
	mem_block_t blocks[MEM_MAX_BLOCK];
	unsigned char *buff;
};


void  mem_init(void);

void *mem_alloc(int size);
void  mem_free(void *data);

/*
 * add or reduce memory size for alloced data, the function may modify data pointer
*/
void *mem_add_extra_size(void *data,int size);
void  mem_reduce_extra_size(void *data,int size);


/*
 * helper function for cnn
*/
void  mem_set_mode(unsigned char mode,unsigned char depth, void *in_data);
void  mem_free_inner_memory(void);
void  *mem_expand_feature(void *data, int size);

#ifdef __cplusplus
  }
#endif

#endif 
