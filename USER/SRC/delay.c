include "system_stm32f10x.h"
#include "delay.h"


void delay_us(int us)
{
    volatile unsigned int k;
    k = (SystemCoreClock / 1000 /12)  * us / 1000;
    while (k-- > 0);
}

void delay_ms(int ms)
{
    delay_us(ms << 10);
}





































