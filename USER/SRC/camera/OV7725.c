
/* Includes ------------------------------------------------------------------*/
#include <ov7725.h>
#include <delay.h>
#include <Nvic_Exit.h>
#include <I2C.h>
/***************************************************************************
?    称：void set_Cmos7725reg(void)
功    能：CMOS寄???置
?口参?：无
?口参?：无
说    ?：
?用方法：set_Cmos7670reg();
***************************************************************************/
void set_Cmos7725reg(void)   //寄???置备份
{
    OV7725_WriteReg(0x32,0x00);
    OV7725_WriteReg(0x2a,0x00);

//	OV7725_WriteReg(0x11,0x00);
    OV7725_WriteReg(0x11,0x82);		 //DIV CLK

    OV7725_WriteReg(0x12,0x46);

    OV7725_WriteReg(0x42,0x7f);
    OV7725_WriteReg(0x4d,0x00);
    OV7725_WriteReg(0x63,0xf0);
    OV7725_WriteReg(0x64,0x1f);
    OV7725_WriteReg(0x65,0xff);
    OV7725_WriteReg(0x66,0x00);
    OV7725_WriteReg(0x67,0x00);
    OV7725_WriteReg(0x69,0x00);


    OV7725_WriteReg(0x13,0xff);
    OV7725_WriteReg(0x0d,0x01);
    OV7725_WriteReg(0x0f,0xc5);
    OV7725_WriteReg(0x14,0x11);
    OV7725_WriteReg(0x22,0xFF);
    OV7725_WriteReg(0x23,0x01);
    OV7725_WriteReg(0x24,0x76);
    OV7725_WriteReg(0x25,0x50);
    OV7725_WriteReg(0x26,0xa1);
    OV7725_WriteReg(0x2b,0x00);
    OV7725_WriteReg(0x6b,0xaa);
    OV7725_WriteReg(0x13,0xff);

    OV7725_WriteReg(0x90,0x0a);
    OV7725_WriteReg(0x91,0x01);
    OV7725_WriteReg(0x92,0x01);
    OV7725_WriteReg(0x93,0x01);

    OV7725_WriteReg(0x94,0x5f);
    OV7725_WriteReg(0x95,0x53);
    OV7725_WriteReg(0x96,0x11);
    OV7725_WriteReg(0x97,0x1a);
    OV7725_WriteReg(0x98,0x3d);
    OV7725_WriteReg(0x99,0x5a);
    OV7725_WriteReg(0x9a,0x9e);

    OV7725_WriteReg(0x9b,0x00);
    OV7725_WriteReg(0x9c,0x30);
    OV7725_WriteReg(0xa7,0x40);
    OV7725_WriteReg(0xa8,0x40);
    OV7725_WriteReg(0xa9,0x80);
    OV7725_WriteReg(0xaa,0x80);

    OV7725_WriteReg(0x9e,0x11);
    OV7725_WriteReg(0x9f,0x02);
    OV7725_WriteReg(0xa6,0x06);

    OV7725_WriteReg(0x7e,0x0c);
    OV7725_WriteReg(0x7f,0x16);
    OV7725_WriteReg(0x80,0x2a);
    OV7725_WriteReg(0x81,0x4e);
    OV7725_WriteReg(0x82,0x61);
    OV7725_WriteReg(0x83,0x6f);
    OV7725_WriteReg(0x84,0x7b);
    OV7725_WriteReg(0x85,0x86);
    OV7725_WriteReg(0x86,0x8e);
    OV7725_WriteReg(0x87,0x97);
    OV7725_WriteReg(0x88,0xa4);
    OV7725_WriteReg(0x89,0xaf);
    OV7725_WriteReg(0x8a,0xc5);
    OV7725_WriteReg(0x8b,0xd7);
    OV7725_WriteReg(0x8c,0xe8);
    OV7725_WriteReg(0x8d,0x20);

    OV7725_WriteReg(0x4e,0xef);
    OV7725_WriteReg(0x4f,0x10);
    OV7725_WriteReg(0x50,0x60);
    OV7725_WriteReg(0x51,0x00);
    OV7725_WriteReg(0x52,0x00);
    OV7725_WriteReg(0x53,0x24);
    OV7725_WriteReg(0x54,0x7a);
    OV7725_WriteReg(0x55,0xfc);

    OV7725_WriteReg(0x33,0x00);
    OV7725_WriteReg(0x22,0x99);
    OV7725_WriteReg(0x23,0x03);
    OV7725_WriteReg(0x4a,0x00);
    OV7725_WriteReg(0x49,0x13);
    OV7725_WriteReg(0x47,0x08);
    OV7725_WriteReg(0x4b,0x14);
    OV7725_WriteReg(0x4c,0x17);
    OV7725_WriteReg(0x46,0x05);
    OV7725_WriteReg(0x0e,0x75);
    OV7725_WriteReg(0x3d,0x82);

    OV7725_WriteReg(0x0c,0x50);
    OV7725_WriteReg(0x3e,0xe2);

    OV7725_WriteReg(0x29,0x50);
    OV7725_WriteReg(0x2c,0x78);
}


unsigned char Cmos7725_init(void)
{
    unsigned char mmm;


//	InitI2C0();
    OV7725_WriteReg(0xff, 0x01);
    mmm=0x80;
    if(0==OV7725_WriteReg(0x12, mmm))
    {
        return 0 ;
    }
    delay_ms(10);
    set_Cmos7725reg();
    return 1;
}

/**
  * @brief  Writes a byte at a specific Camera register
  * @param  Addr: OV7725 register address.
  * @param  Data: Data to be written to the specific register
  * @retval 0x00 if write operation is OK.
  *  0xFF if timeout condition occured (device not connected or bus error).
  */
uint8_t OV7725_WriteReg(uint16_t Addr, uint8_t Data)
{
    StartI2C0();
    if(0==I2CWrite0(OV7725_DEVICE_WRITE_ADDRESS))           //CMOS??地址（写）
    {
        StopI2C0();
        return(0);
    }
    delay_us(100);
    if(0==I2CWrite0(Addr))         //CMOS寄??地址
    {
        StopI2C0();
        return(0);
    }
    delay_us(100);
    if(0==I2CWrite0(Data))       //?写?指定寄??的值
    {
        StopI2C0();
        return(0);
    }
    StopI2C0();

    return(1);
}

/**
  * @brief  Reads a byte from a specific Camera register
  * @param  Addr: OV7725 register address.
  * @retval data read from the specific register or 0xFF if timeout condition
  *         occured.
  */
uint8_t OV7725_ReadReg(uint16_t Addr)
{
    unsigned char regDat;
    StartI2C0();
    if(0==I2CWrite0(OV7725_DEVICE_WRITE_ADDRESS))
    {
        StopI2C0();
        return(0);
    }
    delay_us(100);
    if(0==I2CWrite0(Addr))
    {
        StopI2C0();
        return(0);
    }
    StopI2C0();

    delay_us(100);

    StartI2C0();
    if(0==I2CWrite0(OV7725_DEVICE_READ_ADDRESS))
    {
        StopI2C0();
        return(0);
    }
    delay_us(100);
    regDat=I2CRead0();
    NoAck0();
    StopI2C0();
    return regDat;
}

