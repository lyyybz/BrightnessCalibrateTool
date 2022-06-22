#include "comfunc.h"
#include "mem_manage.h"
#include "stdlib.h"
#include "string.h"

static mem_manage_t  outer_mem_manager;
static mem_manage_t  inner_mem_manager;
static mem_manage_t  *cur_manager=NULL;

void  mem_init()
{
    static 	unsigned char  buff[MEM_SIZE];
    memset(&outer_mem_manager,0,sizeof(outer_mem_manager));
    memset(&inner_mem_manager,0,sizeof(outer_mem_manager));
    cur_manager = &outer_mem_manager;
    outer_mem_manager.left_size = outer_mem_manager.size = sizeof(buff);
    outer_mem_manager.buff = buff;
}

static void *mem_alloc_head(int size)
{
    ASSERT(0 == cur_manager->blocks[0].is_used);
    ASSERT(cur_manager->left_size >= size);

    cur_manager->left_size -= size;
    cur_manager->blocks[0].is_used = 1;
    cur_manager->blocks[0].size = size;
    cur_manager->blocks[0].data = cur_manager->buff;
    return cur_manager->buff;
}

static void *mem_alloc_tail(int size)
{
    ASSERT(0 == cur_manager->blocks[1].is_used);
    ASSERT(cur_manager->left_size >= size);

    cur_manager->left_size -= size;
    cur_manager->blocks[1].is_used = 1;
    cur_manager->blocks[1].size = size;
    cur_manager->blocks[1].data = cur_manager->buff + (cur_manager->size - size);
    return  cur_manager->blocks[1].data;
}

static void  mem_free_head(void *data)
{
    cur_manager->left_size += cur_manager->blocks[0].size;
    cur_manager->blocks[0].is_used = 0;
}

static void  mem_free_tail(void *data)
{
    cur_manager->left_size += cur_manager->blocks[1].size;
    cur_manager->blocks[1].is_used = 0;
}

void *mem_alloc(int size)
{
    if(cur_manager == &outer_mem_manager)
    {
        ASSERT((cur_manager->left_size-inner_mem_manager.size+inner_mem_manager.left_size) >= size);
    }


    if(cur_manager->first_alloc && cur_manager->tail_first)
    {
        if(!cur_manager->blocks[1].is_used)
        {
            return mem_alloc_tail(size);
        }
        else if(!cur_manager->blocks[0].is_used)
        {
            return mem_alloc_head(size);
        }
        else
        {
            LOG_PC("no free mem block\n");
            ASSERT(0);
            return NULL;
        }
    }
    else
    {
        if(!cur_manager->blocks[0].is_used)
        {
            return mem_alloc_head(size);
        }
        else if(!cur_manager->blocks[1].is_used)
        {
            return mem_alloc_tail(size);
        }
        else
        {
            LOG_PC("no free mem block\n");
            ASSERT(0);
            return NULL;
        }
    }


}


void mem_free(void *data)
{
    if(cur_manager->blocks[0].data == data && cur_manager->blocks[0].is_used)
    {
        mem_free_head(data);
    }
    else if(cur_manager->blocks[1].data == data && cur_manager->blocks[1].is_used)
    {
        mem_free_tail(data);
    }
    else
    {
        ASSERT(0);
    }

}

void *mem_add_extra_size(void *data,int size)
{
    ASSERT(cur_manager->left_size >= size);
    if(cur_manager == &outer_mem_manager)
    {
        ASSERT(cur_manager->left_size >= (size+inner_mem_manager.size - inner_mem_manager.left_size));
    }

    void *re_data = NULL;

    if(cur_manager->blocks[0].data == data )
    {
        cur_manager->left_size -= size;
        re_data = cur_manager->buff  + cur_manager->blocks[0].size;
        cur_manager->blocks[0].size += size;
        return re_data;
    }
    else if(cur_manager->blocks[1].data == data)
    {
        cur_manager->left_size -= size;
        cur_manager->blocks[1].size += size;
        re_data =  cur_manager->buff + cur_manager->size - cur_manager->blocks[1].size;
        return re_data;
    }
    else
    {
        LOG_PC("mem add invalid data\n");
        ASSERT(0);
        return NULL;
    }
}



void *mem_expand_feature(void *data, int size)
{
    ASSERT(cur_manager->left_size >= size);
    if(cur_manager == &outer_mem_manager)
    {
        ASSERT(cur_manager->left_size >= (size+inner_mem_manager.size - inner_mem_manager.left_size));
    }


    void *re_data = NULL;

    if(cur_manager->blocks[0].data == data)
    {
        cur_manager->left_size -= size;
        re_data = cur_manager->buff  + cur_manager->blocks[0].size;
        cur_manager->blocks[0].size += size;
        return re_data;
    }
    else if(cur_manager->blocks[1].data == data)
    {
        cur_manager->left_size -= size;
        cur_manager->blocks[1].size += size;
        re_data =  cur_manager->buff + cur_manager->size - cur_manager->blocks[1].size;
        cur_manager->blocks[1].data = re_data;
        return re_data;
    }
    else if(NULL == data)
    {
        return mem_alloc(size);
    }
    else
    {
        LOG_PC("mem add invalid data\n");
        ASSERT(0);
        return NULL;

    }

}

void mem_reduce_extra_size(void *data,int size)
{
    if((cur_manager->blocks[0].data == data) && (cur_manager->blocks[0].size >=size) )
    {
        cur_manager->left_size += size;
        cur_manager->blocks[0].size -= size;

        if(0 ==  cur_manager->blocks[0].size)
        {
            cur_manager->blocks[0].is_used = 0;
        }
        return ;
    }
    else if((cur_manager->blocks[1].data == data) &&(cur_manager->blocks[1].size >=size))
    {
        cur_manager->left_size += size;
        cur_manager->blocks[1].size -= size;
        return ;
    }
    else
    {
        LOG_PC("mem reduce invalid data\n");
        ASSERT(0);
    }
}

void  mem_set_mode(unsigned char mode,unsigned char depth, void *in_data)
{
    ASSERT(depth>=2);
    if(MEM_INNER_MODE == mode)
    {
        memset(&inner_mem_manager, 0, sizeof(inner_mem_manager));
        inner_mem_manager.first_alloc = 1;
        inner_mem_manager.size = inner_mem_manager.left_size = outer_mem_manager.left_size;
        if (outer_mem_manager.blocks[0].is_used)
        {
            inner_mem_manager.buff = outer_mem_manager.buff  +  outer_mem_manager.blocks[0].size;
        }
        else
        {
            inner_mem_manager.buff = outer_mem_manager.buff;
        }

        cur_manager = &inner_mem_manager;
        unsigned char is_even = !(depth%2);//是否为偶数
        /*奇数和in异侧，
         3层: 异 同 出
         5层: 异 同 异 同 出
         1层: 同  出 (特殊处理，不会使用此种机制)
        偶数和in同侧*/
        if(depth >=3 )
        {
            if(( !is_even && (in_data == outer_mem_manager.blocks[0].data))
                    ||(is_even && (in_data == outer_mem_manager.blocks[1].data)) )
            {
                inner_mem_manager.tail_first = 1;
            }
        }
        /*tong chu */
        else if(2 == depth)
        {
            if(in_data == outer_mem_manager.blocks[1].data)
            {
                inner_mem_manager.tail_first = 1;
            }
        }

    }
    else
    {
        cur_manager = &outer_mem_manager;

    }
}

void  mem_free_inner_memory()
{
    memset(&inner_mem_manager, 0, sizeof(inner_mem_manager));
}
