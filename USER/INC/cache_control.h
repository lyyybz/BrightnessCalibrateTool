#ifndef CACHE_CONTROL_H
#define CACHE_CONTROL_H

#ifdef __cplusplus
 extern "C" {
#endif

struct cache_control_type
{
	unsigned char head;
    unsigned char start_line;
    char cached_cnt;
    unsigned char max_cnt;
    unsigned short line_size;
    unsigned char *buff;
    int (*get_next_line)(int line,unsigned char buff[],int size);
};

typedef struct cache_control_type cache_control_t;

void cache_control_init(
    cache_control_t *cache,
	unsigned char *buff,
	unsigned int buff_size,
	unsigned short line_size,
	int (*get_next_line)(int line,unsigned char buff[],int size));


unsigned char cache_get_peixl(cache_control_t *cache,int x,int y);									
unsigned char *cache_control_get_line(cache_control_t *cache, int line);
unsigned char cache_get_peixl_float(cache_control_t *cache,float x,float y,int width,int height);


void cache_control_reset(cache_control_t *cache);


#ifdef __cplusplus
  }
#endif

#endif // CACHE_CONTROL_H
