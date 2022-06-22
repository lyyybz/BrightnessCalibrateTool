#include "camera.h"
#include "comfunc.h"
#include "dataflash.h"
#include "hardware_layer.h"

#if CONFIG_DEBUG

void test_timer()
{
	while(1)
	{
	    reset_performance();
		delay_ms(5000);
		store_performance();
	}

}

void test_camera_speed()
{
	int i;
	unsigned char buff[1024];
    camera_caputre_digit_rect();
	reset_performance();
	for ( i = 0 ; i <  DIGIT_HEIGHT;i++) 
	{
	    get_a_line_from_fifo(i,buff);
	} 
	store_performance();
	
}

void test_flash_speed()
{
	unsigned char data[640];
	int i=0;
	reset_performance();
    for(i=0;i < 120;i++)
    {
    	dataflash_read(0, data, sizeof(data));
    }
    store_performance();
}

int function_1(int a,int b)
{
    int i=0;

    if(a >5)
    {

		return a+b;
    }
    else
    {
		return 0;
    }
	
}


inline  int function_2(int a,int b)
{
    int i=0;

    if(a >5)
    {

		return a+b;
    }
    else
    {
		return 0;
    }

}



void test_inline()
{
	int i =0;
	int sum,sum1;
	int a=1,b=2;
	int (*function)();
	reset_performance();
	
    for(i=0;i < 100000;i++)
    {
       sum+= function_1(a,b);
    }
    store_performance();	

    for(i=0;i < 100000;i++)
    {
       sum+= function_2(a,b);
    }
    store_performance();


    for(i=0;i < 100000;i++)
    {
       sum1 = a+b;
       sum+= sum1;
    }
    
    store_performance();
}

void test_float_int_speed()
{
	int i,k1=5,k2=3,sum=0;
	float k1_f=3.0,k2_f=4.0,sum_f=0.0;
	long long k1_l=3, k2_l=0,sum_l=0.0;

	reset_performance();	
	for(i =0 ;i < 1000000; i++)
	{
	    k2 <<= 4;
		sum += k2 * k1;
	    k2 >>=5;
	}

	
	store_performance() ;

	for(i =0 ;i < 1000000; i++)
	{
		sum_f += k2_f * k1_f;
	} 
	store_performance() ;


	for(i =0 ;i < 1000000; i++)
	{
		sum_l += k1_l * k2_l;
	} 
	store_performance() ;

}



#if CONFIG_DEBUG
int self_check(void)
{
    uint32_t data[0x6],sz,crc,calc_crc,addr = 0;
    int i;

    for ( i = 0 ; i < 10 ; i++)
    {
        dataflash_read(addr,data,sizeof(int)*6);
        sz = data[0];
        crc = data[1];
        calc_crc = dataflash_calc_crc32(addr+sizeof(int)*2,sz);
        if (crc != calc_crc)
        {
            return(-1);
        }
        addr += sizeof(int)*2 ;
        addr += sz ;
    }
    return(0);
}


int test_mulitfy()
{
    int sum=0;
    int i= 0x1000,j=1000;
    hardware_init_high_power();
    reset_performance();
    while(j--)
    {
        short *p1=(short *)(0x20000000),*p2 = (short *)(0x20000010);
        i= 100;
        while(--i)
        {
             sum += p1[0] * p2[0];  
             sum += p1[1] * p2[1]; 
             sum += p1[2] * p2[2]; 
             sum += p1[3] * p2[3]; 
             sum += p1[4] * p2[4]; 
             sum += p1[5] * p2[5]; 
             sum += p1[6] * p2[6]; 
             sum += p1[7] * p2[7]; 
             sum += p1[8] * p2[8]; 
             sum += p1[9] * p2[9]; 
        }
    }
    
    store_performance() ;
    i++;
    if(sum>0)
    {
        sum += 5;
        return sum;
    }
    
    return sum;
}
#endif 


#endif 

