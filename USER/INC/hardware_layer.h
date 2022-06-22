#ifndef __HARDWARE_LAYER_H__
#define __HARDWARE_LAYER_H__

#ifdef __cplusplus
 extern "C" {
#endif

#include "config.h" 
#include <stdint.h>
#include "usart.h"
#include "alloter.h"
#include "stm32f10x.h"

void do_handle_performance(int is_clear);

#if defined(KEIL) && CONFIG_PERFORM
    #define store_performance() do_handle_performance(0)
    #define reset_performance() do_handle_performance(1)
#else
    #define store_performance()  
    #define reset_performance()
#endif

void PWR_Configuration(void);


void hardware_init_high_power(void);
void hardware_init_low_power(void);
int hardwate_get_speed_count(void);
	
/*
  0:使能摄像头
  1:禁止摄像头
*/
enum
{
	CMOS_ENABLE = 0,
	CMOS_DISABLE=1 ,
};
void set_cmos_pwdn(int lvl);
void camera_read_dma_reinit(void);



void SetSysClock_hp(void);
void SetSysClock_lp(void);
void SetSysClock_max108(void);
void mycmos_init(void);
void change_xclk_prescaler(int Pre);
void Get_ChipID(uint32_t *ChipUniqueID);
void set_motor_pin(int pin,int v);
void Check_Flash(void);


void watchdog_init(void);
void feed_dog(void);

/*
    time > 0: 
    time < 0: 
*/


void watchdog_50ms_tick(void);
void init_extern_watchdog(void);
void feed_extern_watchdog(void);
void start_extern_watchdog(void);

void hard_reset_now(void);

OS_CPU_SR OS_CPU_SR_Save(void);
void OS_CPU_SR_Restore(OS_CPU_SR cpu_sr);

void wfi(void);
void feed_dog(void);
void watchdog_set_free_feed_time(int second);
void watchdog_clr_free_feed_time(void);

void usart_it_disable(void);


void delay_us(int us);
void delay_ms(int ms);

void set_led1(int on);
int check_read(void);
void set_relay(int on);
void flash_led(void);
void flash_led_start(void);
void flash_led_stop(void);

void time_count(void);
void time_count_start(void);
void time_count_stop(void);
int is_device_ok(void);

void plc_time_count(void);
void plc_time_count_start(void);
void plc_time_count_stop(void);
int is_plc_timeout(void);

typedef signed long         int32;
typedef signed short        int16;
typedef signed char         int8;
typedef unsigned long       uint32;
typedef unsigned short      uint16;
typedef unsigned char       uint8;

typedef volatile signed long    vint32;
typedef volatile signed short   vint16;
typedef volatile signed char    vint8;

typedef volatile uint32         vuint32;
typedef volatile unsigned short vuint16;
typedef volatile unsigned char  vuint8;

#define	UART_CHAR_MAX_DELAY	30//100 ms unit
#define	CHN_PLC				0x00
#define HSE_VALUE ((u32)16000000)
#define HSI_VALUE ((u32)16000000) /* Value of the Internal oscillator in Hz*/
#define SYS_CLK						((u32)54000000)
#define TIM_CLK_FREQUENCY			SYS_CLK
#define TICK_TIMER_FREQUENCY		((u32)500)
enum
{
	MEASURE_START = 0, MEASURE_INPROGRESS, MEASURE_COMPLETED, MEASURE_FAILED, MEASURE_DISABLE
};
struct soft_uart
{
	vuint16 bit;
	vuint16 data;
	vint8 bitcnt;	 //tx or rx bit count
};

struct soft_uart_chn
{
	struct soft_uart tx;
	struct soft_uart rx;
	uint8 databitcnt;	//data bit count
	int bit_interval;
	struct _UART_FILE_INFOR* puart;
};

#define  soft_uart_chn_tx_cfg(state)     do{TIM_ITConfig(TIM2, TIM_IT_CC1, state);}while(0) 
#define  soft_uart_chn_rx_cfg(state)     do{TIM_ITConfig(TIM2, TIM_IT_CC2, state);}while(0)

#define GPIO_WriteHigh(port,pin) 		(port->ODR |= (uint8)pin)
#define GPIO_WriteLow(port,pin) 		(port->ODR &= (uint8)(~pin))
#define GPIO_ReadInputPin(port,pin) (port->IDR & pin)

#define	RX_VALUE()				GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_5)//GPIO_ReadInputPin(GPIOB, GPIO_Pin_5)
#define	TX_SET()					GPIO_WriteHigh(GPIOB, GPIO_Pin_4)
#define	TX_CLR()					GPIO_WriteLow(GPIOB, GPIO_Pin_4)

void my_uart_init(void);
void UART_CHN_Soft_Init(int chn);

int my_uart_peek_data(int chn, unsigned char buffer[], int len);
int my_uart_read(int chn, unsigned char buffer[],int len);
void my_uart_write(unsigned char buffer[], int len);

#define BaudRateUsed	9600
#define BitInterval TIM_CLK_FREQUENCY / BaudRateUsed

void soft_uart_init(void);
void GPIO_Configuration(void);
void EXTI_Configuration(void);
void EXTI_State_Change(FunctionalState state);
void NVIC_Configuration1(void);
void TIM2_Configuration(void);
void TIM2_chn2_capture(void);



#ifdef __cplusplus
}
#endif

#endif
