#include <stm32f10x.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <Nvic_Exit.h>
#include "config.h"
#include "hardware_layer.h"
#include "delay.h"
#include "i2c.h"
#include "camera_dev.h"
#include "usart.h"
#include "alloter.h"
#include "uart_fifo.h"
#include "protcl.h"
#include "protcl_7e.h"
#include "main.h"
#include "camera.h"
#include "device_info.h"
#include "comfunc.h"
#include "stm32f10x_iwdg.h"
#include "alloter.h"
/*
硬件驱动概述:
timer2:用作测量程序运行时间。
*/
void led_init(void);

#if CONFIG_PERFORM
void timer2_perf_init(void)
{

    TIM_TimeBaseInitTypeDef		TIM_TimeBaseStructure;

    /* Time Base configuration ticks for 1ms  最大测量 12s */
    TIM_TimeBaseStructure.TIM_Prescaler = 10800*2-1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_Period = 0xffff;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;

    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

    TIM_ARRPreloadConfig(TIM2, DISABLE);

    TIM_Cmd(TIM2, ENABLE);


}
unsigned short int get_delt_ticks(void)
{
    static unsigned short int last = 0;
    unsigned short int cur,result;
    cur = TIM2->CNT;
    result = cur - last;
    last = cur;
    return (result)/5;
}
#endif


void NVIC_Configuration(void)
{
    /* Configure the NVIC Preemption Priority Bits */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
}
void init_clk(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB
                            |RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD, ENABLE);


    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

    RCC_APB1PeriphClockCmd( RCC_APB1Periph_PWR
                            |RCC_APB1Periph_TIM2
                            |RCC_APB1Periph_TIM3
                            | RCC_APB1Periph_TIM4, ENABLE);         //as xclk

    RCC_APB2PeriphClockCmd( RCC_APB2Periph_SPI1, ENABLE);

    RCC_APB2PeriphClockCmd( RCC_APB2Periph_USART1, ENABLE);

    /* DMA clock enable */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1 | RCC_AHBPeriph_SRAM | RCC_AHBPeriph_CRC | RCC_AHBPeriph_FLITF, ENABLE);

}
void init_clk_lp(void)
{

    static uint32_t apb2enr,apb1enr;
    static uint8_t first = 1;
    if (first)
    {
        apb2enr = RCC->APB2ENR;
        apb1enr = RCC->APB1ENR;
        first = 0;
    }
    else
    {
        RCC->APB2ENR = apb2enr;
        RCC->APB1ENR = apb1enr;
    }

#if CONFIG_LOW_POWER_470
    RCC_APB1PeriphClockCmd( RCC_APB1Periph_PWR
                            |RCC_APB1Periph_TIM2, ENABLE);

#endif

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB,ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_SPI1, ENABLE);
}


#define CMOS_PWDN_PIN        GPIO_Pin_12
#define DCDC_ENABLE_PIN      GPIO_Pin_0

void set_cmos_pwdn(int lvl)
{
    if (lvl)
    {
        GPIO_SetBits(GPIOB,CMOS_PWDN_PIN);
    }
    else
    {
        GPIO_ResetBits(GPIOB,CMOS_PWDN_PIN);
    }
}

#define CMOS_RST_PIN        GPIO_Pin_6

/*v1.03 is pf6, v1.04 is pb6*/
void set_cmos_rst(int lvl)
{
    if (lvl)
    {
        GPIO_SetBits(GPIOB,CMOS_RST_PIN);
    }
    else
    {
        GPIO_ResetBits(GPIOB,CMOS_RST_PIN);
    }
}


//moter drive pins
static   GPIO_TypeDef * const motor_ports[] = {GPIOD,GPIOC,GPIOC,GPIOC};
static const int motor_pins[] = { GPIO_Pin_0,GPIO_Pin_15,GPIO_Pin_14,GPIO_Pin_13};

  

//pa0 is output test pin.
void camera_read_dma_reinit(void)
{
#if ONLY_MONO_CHANEL
    DMA1_Channel5->CNDTR  = it_fifo_get_width();
#else
    DMA1_Channel5->CNDTR  = it_fifo_get_width()*2;
#endif

    //DMA1_Channel5->CNDTR =DMA_BUF_SZ*2;
    /* Write to DMAy Channelx CMAR */
    DMA1_Channel5->CMAR = (unsigned int)SRC_Buffer;
}
void DMA_Configuration(void)
{
    DMA_InitTypeDef DMA_InitStructure;

    /* DMA1 Channel5 Config */
    DMA_DeInit(DMA1_Channel5);


    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(GPIOA->IDR);

    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)SRC_Buffer;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;

#if ONLY_MONO_CHANEL
    DMA_InitStructure.DMA_BufferSize = it_fifo_get_width();
#else
    DMA_InitStructure.DMA_BufferSize = it_fifo_get_width()*2;
#endif
    //DMA1_Channel5->CNDTR =DMA_BUF_SZ*2;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

    DMA_Init(DMA1_Channel5, &DMA_InitStructure);

    /* DMA1 Channel5 DISABLE */
    DMA_Cmd(DMA1_Channel5, DISABLE);

}


/*
   将timer设置时钟设置为摄像头的PCLK,周期设置为1，触发DMA
   DMA计数到一行的像素之后，触发中断，将本行数据输入到缓冲区
*/
void timer1_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_ICInitTypeDef  	TIM_ICInitStructure;

    TIM_TimeBaseInitTypeDef		TIM_TimeBaseStructure;
    //pa12 is pclk,time1_etr
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    //pa8 is href ,time1_ch1
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    TIM_DeInit(TIM1);

    TIM_ETRClockMode2Config(TIM1, TIM_ExtTRGPSC_OFF, TIM_ExtTRGPolarity_NonInverted, 0);

    TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
    TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
    TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
    TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    TIM_ICInitStructure.TIM_ICFilter = 0x0;

    TIM_ICInit(TIM1, &TIM_ICInitStructure);

    //trigger mode is high level
    TIM_SelectSlaveMode(TIM1, TIM_SlaveMode_Gated);
    TIM_SelectInputTrigger(TIM1, TIM_TS_TI1FP1);

    //dts no div
    TIM_SetClockDivision(TIM1, TIM_CKD_DIV1);

    /* Time base configuration */
    TIM_TimeBaseStructure.TIM_Period = 1;
    TIM_TimeBaseStructure.TIM_Prescaler = 0;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_CenterAligned3;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;

    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

    TIM_ARRPreloadConfig(TIM1, DISABLE);
    // TIM_ARRPreloadConfig(TIM1, ENABLE);

    DMA_Configuration();

    TIM_DMACmd(TIM1, TIM_DMA_Update, ENABLE);  //dma1 ch5

    /* TIM1 DISABLE counter */
    TIM_Cmd(TIM1, DISABLE);
}

#define CHECK_PIN        GPIO_Pin_3
#define LED1_PIN         GPIO_Pin_9
#define RELAY_PIN        GPIO_Pin_15

void set_led1(int on)
{
    BitAction bit = on ? Bit_RESET : Bit_SET; // 低电平开灯
    GPIO_WriteBit(GPIOB, LED1_PIN, bit);
}

int check_read(void)
{
    return GPIO_ReadInputDataBit(GPIOB, CHECK_PIN);
}

void set_relay(int on)
{
    BitAction bit = on ? Bit_SET : Bit_RESET;
    GPIO_WriteBit(GPIOA, RELAY_PIN, bit);
}

#define INTERVAL 0
static int flash_led_enable;
static int flash_led_interval;
int on = 0;
void flash_led(void) {
    if (flash_led_enable) {
        flash_led_interval--;
        if (flash_led_interval < 0) {
            // int light = GPIO_ReadInputDataBit(GPIOB, LED1_PIN); // 有问题，状态读不回来
            set_led1(!on);
            on = on ? 0 : 1;
            flash_led_interval = INTERVAL;
        }
    }
}

void flash_led_start(void) {
    flash_led_interval = 0;
    flash_led_enable = 1;
}

void flash_led_stop(void) {
    flash_led_enable = 0;
}

static int time_count_enable;
static int time_count_count;
void time_count(){
		if(time_count_enable){
				time_count_count++;
		}
}

void time_count_start(){
		time_count_count = 0;
		time_count_enable = 1;
}

void time_count_stop(){
		time_count_count = 0;
		time_count_enable = 0;
}

int is_device_ok(){
		// 50ms一次，5s为100次
		return time_count_count;
}

// 载波超时
static int plc_time_count_enable;
static int plc_time_count_count;
void plc_time_count(void){
		if(plc_time_count_enable){
				plc_time_count_count++;
		}
}
void plc_time_count_start(void){
		plc_time_count_count = 0;
		plc_time_count_enable = 1;
}
void plc_time_count_stop(void){
		plc_time_count_count = 0;
		plc_time_count_enable = 0;
}
int is_plc_timeout(void){
		return plc_time_count_count;
}

//输入为108M
void timer3_pwm_as_xclk(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef		TIM_TimeBaseStructure;
    TIM_OCInitTypeDef  TIM_OCInitStructure;

    //xclk is pb1,  is Tim3_ch4
    
	
    /* Configure pins as AF pushpull  54M */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 ;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP  ;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* Time Base configuration  54M*/
#if	CAPTURE_MAIN_CLK_108MHZ && ONLY_MONO_CHANEL
    TIM_TimeBaseStructure.TIM_Prescaler = 1;
#elif 	CAPTURE_MAIN_CLK_108MHZ && (!ONLY_MONO_CHANEL)
    TIM_TimeBaseStructure.TIM_Prescaler = 2;
#else
    TIM_TimeBaseStructure.TIM_Prescaler = 0;
#endif

    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_Period = 1;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;

    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

    TIM_ARRPreloadConfig(TIM3, DISABLE);

#if 1
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 1;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
    TIM_OC4Init(TIM3, &TIM_OCInitStructure);

    TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Disable); 
#endif

    TIM_Cmd(TIM3, ENABLE); 
}

void change_xclk_prescaler(int Pre)
{
    TIM_TimeBaseInitTypeDef		TIM_TimeBaseStructure;

    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_Period = 1;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseStructure.TIM_Prescaler = Pre;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
}

#define CMOS_VSYNC_PIN        GPIO_Pin_13

void init_gpio(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    EXTI_InitTypeDef   EXTI_InitStructure;

    // pA0 - pA7 is cmos data bus
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 |GPIO_Pin_1 |GPIO_Pin_2
                                  |GPIO_Pin_3 |GPIO_Pin_4 |GPIO_Pin_5
                                  |GPIO_Pin_6 |GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING  ;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    //pb10 is scl,pb11 is sda,sda is open drain driver
    GPIO_InitStructure.GPIO_Pin = SCL_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP  ;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = SDA_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD  ;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_SetBits(GPIOB,SCL_PIN | SDA_PIN);

    //PB1 is xclk
    timer3_pwm_as_xclk();

    GPIO_InitStructure.GPIO_Pin = CMOS_RST_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP  ;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    set_cmos_rst(1);
    set_cmos_rst(0);

    //pb12 is cmos pwdn
    GPIO_InitStructure.GPIO_Pin = CMOS_PWDN_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP  ;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    set_cmos_pwdn(CMOS_DISABLE);

    //pb13 is vsync
    GPIO_InitStructure.GPIO_Pin = CMOS_VSYNC_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU  ;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    /* Connect EXTI13 Line to PB13 pin */
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource13);
    /* Configure EXTI13 line */
    EXTI_InitStructure.EXTI_Line = EXTI_Line13;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;//EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

}

void mycmos_init(void)
{
    set_cmos_rst(0);
    delay_us(10000);
    set_cmos_rst(1);
    delay_us(1000);
    cmos_init();
}

void dma1_ch5_nvic_config(int state)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    /* USART1 IRQ Channel configuration */
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    if (state)
    {
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    }
    else
    {
        NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
    }
    NVIC_Init(&NVIC_InitStructure);
}

void PWR_Configuration(void)
{
    PWR_PVDLevelConfig(PWR_PVDLevel_2V8);
    PWR_PVDCmd(ENABLE);
}

volatile unsigned int watchdog_left_time = 0;
/*配置时间1s左右*/
void watchdog_init()
{
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    IWDG_SetPrescaler(IWDG_Prescaler_128);
    IWDG_SetReload(0x0FFF/12);
    IWDG_ReloadCounter();
    IWDG_Enable();
}

void watchdog_50ms_tick()
{
    OS_CPU_SR cpu_sr;

    if(0 == watchdog_left_time)
    {
        return;
    }

    feed_dog();

    OS_ENTER_CRITICAL();
    watchdog_left_time--;
    OS_EXIT_CRITICAL();
}

void my_gpio_init(void) {
    //pb3 is check
    GPIO_InitTypeDef GPIO_InitStructure_Check;
    GPIO_InitStructure_Check.GPIO_Pin = CHECK_PIN;
    GPIO_InitStructure_Check.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure_Check.GPIO_Mode = GPIO_Mode_IPU; // 上拉输入，默认为高，可检测到由高到低的电平变化
    GPIO_Init(GPIOB, &GPIO_InitStructure_Check);

    //pb9 is led1
    GPIO_InitTypeDef GPIO_InitStructure_LED1;
    GPIO_InitStructure_LED1.GPIO_Pin = LED1_PIN;
    GPIO_InitStructure_LED1.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure_LED1.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure_LED1);

    /* Connect pin to Periph */
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
    // 调用这句 relay才能控制
    //pa15 is realy
    GPIO_InitTypeDef GPIO_InitStructure_Relay;
    GPIO_InitStructure_Relay.GPIO_Pin = RELAY_PIN;
    GPIO_InitStructure_Relay.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure_Relay.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure_Relay);
}

void hardware_init_high_power(void)
{
    reset_performance();
    __disable_irq();
    SetSysClock_hp();//48mhz
    SetSysClock_max108();
    NVIC_Configuration();

    init_clk();

    init_gpio();
    SysTick_Config(SystemCoreClock/20); //50ms
    SystemCoreClockUpdate();
    
    init_chn_pool_mgr();
    uart_chn_init(0);
    usart_init(9600,0);   // 载波串口初始化
    usart1_nvic_config(1);

    my_gpio_init();		// check relay led
		my_uart_init(); // 虚拟串口，与上位机通信

//delay_ms(10000);
#if CONFIG_PERFORM
    timer2_perf_init();
#endif
    timer1_init();

    dma1_ch5_nvic_config(1);
    store_performance();
    __enable_irq();
}

#define GD32_CFGR_PLL_Mask            ((uint32_t)0xF7C0FFFF)
void GD32_RCC_PLLConfig(uint32_t RCC_PLLSource, uint32_t RCC_PLLMul)
{
    uint32_t tmpreg = 0;

    tmpreg = RCC->CFGR;
    /* Clear PLLSRC, PLLXTPRE and PLLMUL[3:0] bits */
    tmpreg &= GD32_CFGR_PLL_Mask;
    /* Set the PLL configuration bits */
    tmpreg |= RCC_PLLSource | RCC_PLLMul;
    /* Store the new value */
    RCC->CFGR = tmpreg;
}

void RCC_Configuration(unsigned int pll)
{
    RCC_DeInit();
    RCC_HCLKConfig(RCC_SYSCLK_Div1);
    RCC_PCLK2Config(RCC_HCLK_Div1);
    RCC_PCLK1Config(RCC_HCLK_Div2);
    RCC_PLLConfig(RCC_PLLSource_HSI_Div2, pll);
    RCC_PLLCmd(ENABLE);
    while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
    {
    }
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
    while(RCC_GetSYSCLKSource() != 0x08)
    {
    }
}

void RCC_Low_Power_Configuration()
{
    RCC_DeInit();
    /*AHB Prescaler:RCC_SYSCLK_Div16:  500k
                    RCC_SYSCLK_Div8:   1M
                    RCC_SYSCLK_Div128: 25K
    */
    RCC_HCLKConfig(RCC_SYSCLK_Div16);

    /*APB2 Prescaler*/
    RCC_PCLK2Config(RCC_HCLK_Div1);

    RCC_PLLCmd(DISABLE);
    while(RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == RESET)
    {
    }

    RCC_SYSCLKConfig(RCC_SYSCLKSource_HSI);
    while(RCC_GetSYSCLKSource() != 0x00)
    {
    }
}
void disable_interupt_nvic(void)
{
    NVIC->ICER[0] = ~0;
    NVIC->ICPR[0] = ~0;
}

#define GD32_RCC_PLLMul_27   (0x08000000 | RCC_PLLMul_12)
#define GD32_RCC_PLLMul_24   (0x081C0000)



void SetSysClock_hp(void)
{
    RCC_Configuration(RCC_PLLMul_12);//高功耗 48MHZ
    SystemCoreClockUpdate();
}

void SetSysClock_max108(void)
{
    RCC_Configuration(GD32_RCC_PLLMul_27);//108MHZ
    SystemCoreClockUpdate();
}

void SetSysClock_lp(void)
{
    RCC_Low_Power_Configuration();
    SystemCoreClockUpdate();
}

int hardwate_get_speed_count()
{
    return  max(SystemCoreClock/500000,1) ;
}

void hardware_init_low_power(void)
{
    __disable_irq();

    init_clk_lp();
    SetSysClock_lp();
    disable_interupt_nvic();

    SystemCoreClockUpdate();
    SysTick_Config(SystemCoreClock /20); //50ms

    init_chn_pool_mgr();
    uart_chn_init(0);
    usart_init(9600,0);  // 载波串口初始化

    usart1_nvic_config(1);

    //pb12 is cmos pwdn
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = CMOS_PWDN_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    set_cmos_pwdn(CMOS_DISABLE);

    my_gpio_init();

    __enable_irq();
}

#if CONFIG_PERFORM
volatile unsigned short int delt_ms[0x10];
static int kidx= 0;
void do_handle_performance(int is_clear)
{
    if(1 == is_clear)
    {
        kidx = 0;
        memset((void *)delt_ms,0,sizeof(delt_ms));
        get_delt_ticks();
    }
    else
    {
        delt_ms[kidx++] = get_delt_ticks();
        kidx &= 0xf;
    }
}
#endif

void delay_us(int us)
{
    volatile long long k;
    k = (SystemCoreClock / 1000 /14);
    k = k* us / 1000;
    while (k-- > 0);
}

void delay_ms(int ms)
{
    delay_us(ms << 10);
}

void Get_ChipID(uint32_t *ChipUniqueID)
{
    ChipUniqueID[0] = *(__IO u32 *)(0x1FFFF7F0);
    ChipUniqueID[1] = *(__IO u32 *)(0x1FFFF7EC);
    ChipUniqueID[2] = *(__IO u32 *)(0x1FFFF7E8);
}

void hard_reset_now(void)
{
    NVIC_SystemReset();
    delay_ms(10);
}


#if CONFIG_DEBUG

void Check_Flash(void)
{
    FlagStatus status = RESET;
    status = FLASH_GetReadOutProtectionStatus();
    if(status != SET)
        FLASH_ReadOutProtection(ENABLE);
}

void init_extern_watchdog()
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP  ;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_WriteBit(GPIOB,GPIO_Pin_2,0);
}

void feed_extern_watchdog()
{
    static unsigned char value=0;
    value = !value;
    GPIO_WriteBit(GPIOB,GPIO_Pin_2,value);
}

void start_extern_watchdog()
{
    feed_extern_watchdog();
    delay_ms(20);
    feed_extern_watchdog();
}

#endif
//extern __GET_LAST_BIT_POS(unsigned int x);
OS_CPU_SR OS_CPU_SR_Save(void)
{
    OS_CPU_SR cpu_sr ;
    cpu_sr = __get_PRIMASK();
    __disable_irq();
    return(cpu_sr);
}
void OS_CPU_SR_Restore(OS_CPU_SR cpu_sr)
{
    __set_PRIMASK(cpu_sr);
}


void wfi(void)
{
    //ldo_enable(1);
    __WFI();
    //ldo_enable(1);
}

void feed_dog()
{
    IWDG_ReloadCounter();
}


void watchdog_set_free_feed_time(int second)
{
    OS_CPU_SR cpu_sr;
    OS_ENTER_CRITICAL();
    watchdog_left_time = second*(1000/50);
    OS_EXIT_CRITICAL();
}

void watchdog_clr_free_feed_time()
{
    OS_CPU_SR cpu_sr;

    OS_ENTER_CRITICAL();
    watchdog_left_time = 0;
    OS_EXIT_CRITICAL();
}


void usart_it_disable()
{
    USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
    USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
}


/********************************
模拟串口相关
*********************************/

void soft_uart_init(void)
{
	GPIO_Configuration();
	EXTI_Configuration();
	NVIC_Configuration1();
  TIM2_Configuration();
}

void GPIO_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // TX PB4 发送 推免输出
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // RX PB5 接收 浮空输入
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void EXTI_Configuration(void)
{
		Exit_Init(GPIOB, GPIO_Pin_5, GPIO_Mode_IPU, EXTI_Trigger_Falling, 2, 2);
}

void EXTI_State_Change(FunctionalState state)
{
		EXTI_InitTypeDef EXTI_InitStructure;
		EXTI_InitStructure.EXTI_Line = EXTI_Line5;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = state;
    EXTI_Init(&EXTI_InitStructure);
}

void NVIC_Configuration1(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0000);
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void TIM2_Configuration(void)
{
    /* Time base configuration */
		TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

    TIM_DeInit(TIM2);

    TIM_TimeBaseStructure.TIM_Period = 0xFFFF;
    TIM_TimeBaseStructure.TIM_Prescaler = 1;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

    TIM_Cmd(TIM2, ENABLE);
}

void TIM2_chn2_capture(void)
{
    TIM_ICInitTypeDef TIM_ICInitStructure;
    TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;                  //选择通道2
    TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Falling;      //输入下降沿捕获 
    TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;   //通道方向选择  
    TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;             //每次检测到捕获输入就触发一次捕获
    TIM_ICInitStructure.TIM_ICFilter = 0x0f;                          //

    TIM_ICInit(TIM2, &TIM_ICInitStructure);
    TIM_ClearFlag(TIM2, TIM_FLAG_CC2);
		TIM_ITConfig(TIM2, TIM_IT_CC2, DISABLE);
}

volatile struct _UART_FILE_INFOR _uart_file_infor[1];
volatile struct soft_uart_chn _uart_chns[1];

void my_uart_init(void) {
   
    UART_CHN_Soft_Init(CHN_SOFT);
    memset(_uart_chns, 0x00, sizeof(_uart_chns));
    _uart_chns[CHN_SOFT].bit_interval = BitInterval;
    _uart_chns[CHN_SOFT].databitcnt = 1 + 8 + 1;
    _uart_chns[CHN_SOFT].puart = &_uart_file_infor[CHN_SOFT];
    soft_uart_init();//模拟串口初始化
    //soft_uart_chn_rx_cfg(ENABLE);
}

void mymemcpy(void* dst, void* src, uint8 len)
{
    while (len--)
    {
        *(char*)dst = *(char*)src;
        dst = (char*)dst + 1;
        src = (char*)src + 1;
    }
}

void UART_CHN_Soft_Init(int chn)
{
    struct _UART_FILE_INFOR* puart_infor;
    puart_infor = &_uart_file_infor[chn];
    puart_infor->rx_slot.data_cnt = 0x00;
    puart_infor->rx_slot.data_max = 0xF0;
    puart_infor->rx_slot.tx = INVALID_PTR;
    puart_infor->rx_slot.rx = INVALID_PTR;
    mymemcpy(&puart_infor->tx_slot, &puart_infor->rx_slot, sizeof(puart_infor->rx_slot));
    puart_infor->over_time_tick = UART_CHAR_MAX_DELAY;
}

int uart_chn_tx_byte(int chn, unsigned char buffer[])
{
    return(get_chn_bytes(&(_uart_file_infor[chn].tx_slot), buffer, 1));
}

int uart_chn_rx_byte(int chn, unsigned char c)
{
    return(put_chn_bytes(&(_uart_file_infor[chn].rx_slot), &c, 1));
}

uint8 count_bit_in_char(uint8 x)
{
    uint8 n = 0;
    if (x)
    {
        do
        {
            n++;
        } while ((x = x & (x - 1)));
    }
    return(n);
}
uint16 _send_fmt_even(uint8 x)
{
    uint8 cs;
    uint16 databit = x;
    cs = count_bit_in_char(x);
    if (cs & 0x01)
    {
        databit |= 1 << 8; //cs
    }
    databit |= 1 << 9; //stop bit
    return(databit);
}

uint8 recv_chk_even(unsigned int x)
{
    return(x == _send_fmt_even(x & 0xFF));
}

uint8 get_soft_uart_rx_data(struct soft_uart_chn* pchn)
{
    return(uart_chn_rx_byte(CHN_SOFT, pchn->rx.data));
}

uint8 get_soft_uart_tx_data(struct soft_uart_chn *pchn)
{
	unsigned char c;
	// int parity  = pchn->puart->uart_cfg.parity;

	if(uart_chn_tx_byte(CHN_SOFT,&c))		  //edit by du
	{
		//tx->bit_interval = TIM_CLK_FREQUENCY / 1200;/////ready to be configed
		//tx->bit_interval = TIM_CLK_FREQUENCY / uart_file_infor[chn].uart_cfg.baud;
		pchn->tx.data = _send_fmt_even(c); 
		pchn->tx.data  <<= 0x01;
		pchn->tx.data &= (1 << 11)-1;
		pchn->databitcnt =  1+ 8 + 1;         
		return(1);
	}
  return(0);
}

int my_uart_peek_data(int chn, unsigned char buffer[], int len)
{
    return(peek_chn_byte(&(_uart_file_infor[chn].rx_slot), buffer, len));
}

int my_uart_read(int chn, unsigned char buffer[],int len)
{
    return(get_chn_bytes(&(_uart_file_infor[chn].rx_slot), buffer, len));
}

void TIM_OC2Init1(TIM_TypeDef* TIMx, uint16 capture)
{
    TIM_OCInitTypeDef TIM_OCInitStructure;
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Timing;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_Pulse = capture;
    TIM_OC2Init(TIMx, &TIM_OCInitStructure);
}

struct soft_uart_chn* pchn;
void TIM2_IRQHandler(void)
{
    uint16 capture;

    /* chn rx chn 2 */
    if (TIM_GetITStatus(TIM2, TIM_IT_CC2))
    {
        TIM_ClearITPendingBit(TIM2, TIM_IT_CC2);
        pchn = &_uart_chns[CHN_SOFT];

        capture = TIM_GetCapture2(TIM2);
        if (0x00 == pchn->rx.bitcnt)
        {     
//            capture += pchn->bit_interval >> 2;
//            capture += pchn->bit_interval;
//            TIM_OC2Init1(TIM2, capture);
//            pchn->rx.bitcnt = 1;
//            pchn->rx.data = 0x00;
//            pchn->rx.bit = 0x01;
        }
        else if (pchn->rx.bitcnt < pchn->databitcnt - 1)
        {
            if (RX_VALUE())
            {
                pchn->rx.data |= pchn->rx.bit;
            }
            pchn->rx.bit <<= 0x01;
            pchn->rx.bitcnt++;
            capture += pchn->bit_interval;
            TIM_SetCompare2(TIM2, capture);
        }
        else
        {
            if (RX_VALUE())
            {
                //pchn->rx.data |= pchn->rx.bit;
            }
						//log_msg("recv byte: %02x \n", pchn->rx.data);
            if (get_soft_uart_rx_data(pchn))
            {
                uart_rx_hook(0);
            }

            TIM2_chn2_capture(); // 重新进入捕获模式
						EXTI_State_Change(ENABLE);
        }
    }
    /* chn tx */
    if (TIM_GetITStatus(TIM2, TIM_IT_CC1))
    {
        TIM_ClearITPendingBit(TIM2, TIM_IT_CC1);
        pchn = &_uart_chns[CHN_SOFT];

        capture = TIM_GetCapture1(TIM2);
        capture += pchn->bit_interval;
        TIM_SetCompare1(TIM2, capture);

        if (pchn->tx.bitcnt-- > 0)
        {
            if (pchn->tx.data & pchn->tx.bit)
            {
                TX_SET();
            }
            else
            {
                TX_CLR();
            }
            pchn->tx.bit <<= 0x01;
        }
        else
        {
            TX_SET();
            if (get_soft_uart_tx_data(pchn))
            {
                pchn->tx.bitcnt = pchn->databitcnt;
                pchn->tx.bit = 0x01;
            }
            else
            {
                soft_uart_chn_tx_cfg(DISABLE);
            }
        }
    }
}

void my_uart_write(unsigned char buffer[], int len)
{
    put_chn_bytes(&(_uart_file_infor[CHN_SOFT].tx_slot), buffer, len);
    soft_uart_chn_tx_cfg(ENABLE);
}
