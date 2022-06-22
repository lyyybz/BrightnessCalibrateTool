
#include<I2C.h>
#include<delay.h>

#if 1
void set_scl(int x)
{
    do {
        if(x) {
            GPIO_SetBits(GPIOB,SCL_PIN);
        }
        else {
            GPIO_ResetBits(GPIOB,SCL_PIN);
        }
    }
    while(0);
}

void set_sda(int x)
{
    do {
        if(x) {
            GPIO_SetBits(GPIOB,SDA_PIN);
        }
        else {
            GPIO_ResetBits(GPIOB,SDA_PIN);
        }
    }
    while(0);
}
#else
#define set_scl(x)    do{ if(x){ GPIO_SetBits(GPIOB,SCL_PIN);}else{GPIO_ResetBits(GPIOB,SCL_PIN);}}while(0)
#define set_sda(x)    do{ if(x){ GPIO_SetBits(GPIOB,SDA_PIN);}else{GPIO_ResetBits(GPIOB,SDA_PIN);}}while(0)
#endif
#define get_sda()     GPIO_ReadInputDataBit(GPIOB,SDA_PIN)

#define     I2C_OP_DELAY    2

void start_i2c(void)
{
    set_sda(1);
    delay_us(I2C_OP_DELAY);

    set_scl(1);
    delay_us(I2C_OP_DELAY);

    set_sda(0);
    delay_us(I2C_OP_DELAY);

    set_scl(0);
    delay_us(I2C_OP_DELAY);
}

void stop_i2c(void)
{
    set_sda(0);
    delay_us(I2C_OP_DELAY);

    set_scl(1);
    delay_us(I2C_OP_DELAY);

    set_sda(1);
    delay_us(I2C_OP_DELAY);
}

void i2c_no_ack(void)
{
    set_sda(1);
    delay_us(I2C_OP_DELAY);

    set_scl(1);
    delay_us(I2C_OP_DELAY);

    set_scl(0);
    delay_us(I2C_OP_DELAY);

    set_sda(0);
    delay_us(I2C_OP_DELAY);
}

void i2c_ack(void)
{
    set_sda(0);
    delay_us(I2C_OP_DELAY);
    set_scl(1);
    delay_us(I2C_OP_DELAY);
    set_scl(0);
    delay_us(I2C_OP_DELAY);
    set_sda(1);
}

int i2c_write(unsigned char b)
{
    int j,ret;

    for(j=0; j<8; j++)
    {
        if(b & 0x80)
        {
            set_sda(1);
        }
        else
        {
            set_sda(0);
        }
        b<<=1;
        delay_us(I2C_OP_DELAY);
        set_scl(1);
        delay_us(I2C_OP_DELAY);
        set_scl(0);
        delay_us(I2C_OP_DELAY);

    }
    set_sda(1);
    delay_us(I2C_OP_DELAY);
    set_scl(1);
    delay_us(I2C_OP_DELAY*5);
    if(get_sda())
    {
        ret=0;
    }
    else
    {
        ret=1;
    }
    set_scl(0);
    delay_us(I2C_OP_DELAY);
    return(ret);
}


int i2c_read(void)
{
    int ret = 0,j;
    set_sda(1);
    delay_us(I2C_OP_DELAY);
    for ( j = 0 ; j < 8 ; j++)
    {
        set_scl(1);
        delay_us(I2C_OP_DELAY);
        ret <<= 1;
        if(get_sda())
        {
            ret++;
        }
        set_scl(0);
        delay_us(I2C_OP_DELAY);
    }
    set_sda(1);
    delay_us(I2C_OP_DELAY);
    set_scl(1);
    delay_us(I2C_OP_DELAY);
    set_scl(0);
    delay_us(I2C_OP_DELAY);
    return(ret);
}

