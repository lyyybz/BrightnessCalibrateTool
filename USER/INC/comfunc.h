#ifndef ES_SPC_H
#define ES_SPC_H

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdint.h>
#include "config.h"

#ifndef INT_MAX
    #define INT_MAX   0x7fffffff
    #define INT_MIN  (-2147483648)
#endif


#define usleep(us) delayus(us)

#define get_bits(val,x1,x2)   (((val)>>(x1))&((1<<((x2)-(x1)+1))-1))
	 
#define LITTLE_ENDIAN_BYTE_TO_SHORT(x)  ((*((uint8_t *)(x)+1))<<8 | (*((uint8_t *)(x)+0)))
#define LITTLE_ENDIAN_BYTE_TO_LONG(x)   ((*((uint8_t *)(x)+3))<<24 | (*((uint8_t *)(x)+2))<<16 |\
                                        (*((uint8_t *)(x)+1))<<8 | (*((uint8_t *)(x)+0)))

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
/*
 * These are non-NULL pointers that will result in page faults
 * under normal circumstances, used to verify that nobody uses
 * non-initialized list entries.
 */
#define LIST_POISON1  ((void *) 0x00100100)
#define LIST_POISON2  ((void *) 0x00200200)
#define HZ  100

//typedef uint32_t time_t;
/*
 * swap - swap value of @a and @b
 */
#ifdef KEIL
    #define reverse(buff, len) do {order_reverse(buff, len);}while (0)
#endif


#define min(a, b) ((a)<(b)?(a):(b))
#define max(a, b) ((a)>(b)?(a):(b))
#define MIN(a, b) min(a,b)
int myabs(int x);
#define time_after(a,b) ((long)(b) - (long)(a) < 0)
#define time_before(a,b)    time_after(b,a)
#define time_after_eq(a,b) ((long)(a) - (long)(b) >= 0)
#define time_before_eq(a,b)	time_after_eq(b,a)
#ifndef offsetof
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif
#define size_of(TYPE, MEMBER)  (sizeof(((TYPE*)0)->MEMBER))
/**
 * container_of - cast a member of a structure out to the containing structure
 * @ptr:	the pointer to the member.
 * @type:	the type of the container struct this is embedded in.
 * @member:	the name of the member within the struct.
 *
 */
#define container_of(ptr, type, member) ((type *)((char *)ptr - offsetof(type,member)))

#define BCD2BIN(x)  ((((x)>>4)*10)+((x) & 0x0F))
#define BIN2BCD(x)  ((((uint8_t)(x)/10)<<4)+((uint8_t)(x) % 10))

#define IS_AF(c)  ((c >= 'A') && (c <= 'F'))
#define IS_af(c)  ((c >= 'a') && (c <= 'f'))
#define IS_09(c)  ((c >= '0') && (c <= '9'))
#define ISVALIDHEX(c)  IS_AF(c) || IS_af(c) || IS_09(c)
#define ISVALIDDEC(c)  IS_09(c)
#define CONVERTDEC(c)  (c - '0')

#define CONVERTHEX_alpha(c)  (IS_AF(c) ? (c - 'A'+10) : (c - 'a'+10))
#define CONVERTHEX(c)   (IS_09(c) ? (c - '0') : CONVERTHEX_alpha(c))

#define get_nbits_val(val,x1,x2)            (((val)>>(x1))&((1<<((x2)-(x1)+1))-1))
#define HOLE(val, start, end) ((val) & (~(((1 << ((end) - (start) + 1)) - 1) << (start))))
#define FILL(set, start, end) (((set) & ((1 << ((end) - (start) + 1)) - 1))<< (start))
#define set_nbits_val(val, set, start, end) (val = HOLE(val, start, end) | FILL(set, start, end))

#define set_bit(x, bit) 	((x) |= 1 << (bit))
#define reset_bit(x, bit) 	((x) &= ~(1 << (bit)))
#define is_bit_set(x, bit) 	((x) & (1 << (bit)))

#define set_task(task_bit)		set_bit(task_monitor, task_bit)
#define is_task_set(task_bit)	is_bit_set(task_monitor, task_bit)
#define reset_task(task_bit) 	reset_bit(task_monitor, task_bit)
#define has_task_pending()		(task_monitor != 0)

//#define assert(expr) ((expr) ? (void)0 : assert_failed(__FILE__, __LINE__))
//void assert_failed(const char* file, unsigned long line);

uint8_t checksum (const void *data, int len);
int is_data_all_bcd(uint8_t data[], int len);
int is_data_all_xx(const unsigned char data[], int len, unsigned char x);
int is_all_xx(const void *s1, int val, int n);


/* num can be negative */
void memaddnum(uint8_t mem[], int num, int cnt);

int get_last_bit_seqno(unsigned int x);
 
#define STM_PAGE_SIZE		(1*1024L)

void order_reverse(void *buff, int len);
int mymin(int x,int y);
int mymax(int x,int y);

uint16_t UpdateCRC16(uint16_t crcIn, uint8_t byte);
uint16_t Cal_CRC16(const volatile uint8_t* data, uint32_t size);
 
int count_1bits(int x);
int get_last1_pos(unsigned int x);
int only_one_1(int x);

#define STM_PAGE_SIZE		(1*1024L)


#ifdef KEIL 
    //#define ASSERT(x)   if(!(x)) {while(1){}}
    #define ASSERT(x)  
    #define LOG_PC(str) 
#else 
	void log_pc(char *s);
	void assert_failed1(const char *exp,const char *func,const  char *file, int line);
	#define LOG_PC(str)  log_pc(str)
#define ASSERT(exp)	   { if(!(exp))  {\
                       assert_failed1(#exp,__FILE__,__FUNCTION__, __LINE__);\
                        while(1); \
                       }}
#endif 

void delay_ms(int ms);
void delay_us(int us); 



void put_be_val(uint32_t val, uint8_t * p, int bytes);
void put_le_val(uint32_t val, uint8_t * p, int bytes);
uint32_t get_le_val(const uint8_t * p, int bytes);
uint32_t get_be_val(const uint8_t * p, int bytes);
#ifdef __cplusplus
}
#endif

#endif
