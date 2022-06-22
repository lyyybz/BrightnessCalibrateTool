#ifndef	_ENCODE_H_
#define	_ENCODE_H_

#include "proto_smart.h"

#define ENCODE_LEN                  (SN_LEN+DKEY_LEN+AID_LEN+PW_LEN)


#define ENCODE_PORT                 GPIOA   /* change depends on your mcu */
#define ENCODE_PIN                  GPIO_Pin_13 /* change depends on your mcu */ 
#define ENCODE_RCC_AHBPeriph        RCC_AHBPeriph_GPIOA /* change depends on your mcu */

#define ENCODE_UART_BAUD            9600    /* no need to change */
#define TARGET_CLK                  48000000 /* no need to change */
#define PRESCALE                    configSYS_CLOCK / TARGET_CLK
#define PERIOD                      TARGET_CLK / ENCODE_UART_BAUD

/* I/O's configuration depends on your mcu, but in general case, no need to change */
#define RX_SDA_VALUE()              GPIO_ReadInputDataBit(ENCODE_PORT, ENCODE_PIN)
#define TX_SDA_SET()                GPIO_SetBits(ENCODE_PORT, ENCODE_PIN)
#define TX_SDA_CLR()                GPIO_ResetBits(ENCODE_PORT, ENCODE_PIN)
#define ENCODE_SDA_IN() do{\
                            GPIO_InitTypeDef  GPIO_InitStructure;\
                            RCC_AHBPeriphClockCmd(ENCODE_RCC_AHBPeriph, ENABLE);\
                            GPIO_InitStructure.GPIO_Pin = ENCODE_PIN;\
                            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;\
                            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;\
                            GPIO_Init(ENCODE_PORT, &GPIO_InitStructure);\
                            }while(0);
#define ENCODE_SDA_OUT() do{\
                            GPIO_InitTypeDef  GPIO_InitStructure;\
                            RCC_AHBPeriphClockCmd(ENCODE_RCC_AHBPeriph, ENABLE);\
                            GPIO_InitStructure.GPIO_Pin = ENCODE_PIN;\
                            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;\
                            GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;\
                            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;\
                            GPIO_Init(ENCODE_PORT, &GPIO_InitStructure);\
                            }while(0);
struct CODE_FRAME
{
    uint8_t start;
    uint8_t len;
    uint8_t data[1];
};

struct ENCODE_PARAM
{
    uint8_t sn[SN_LEN];//12
    uint8_t dkey[DKEY_LEN];//8
    uint8_t id[AID_LEN];//4
    uint8_t pwd[PW_LEN];//2
};

void device_encode_opt(void);

#endif
