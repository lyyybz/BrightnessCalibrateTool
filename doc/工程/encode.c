#include <utils.h>
#include <config.h>
#include <types.h>
#include <os.h>
#include <stm32f0xx.h>
#include "encode.h"
#include "board.h"
#include "settings.h"

static void encode_gpio_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHBPeriphClockCmd(ENCODE_RCC_AHBPeriph, ENABLE);
	GPIO_InitStructure.GPIO_Pin   = ENCODE_PIN;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(ENCODE_PORT, &GPIO_InitStructure);
}

static void encode_timer_setmat(uint16_t value)
{
	TIM_SetAutoreload(TIM3, value);
	TIM_SetCounter(TIM3, 0);
	TIM_ClearITPendingBit(TIM3, TIM_FLAG_Update);
}

static void encode_timer_init(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	TIM_TimeBaseStructure.TIM_Period        = PERIOD;
	TIM_TimeBaseStructure.TIM_Prescaler     = PRESCALE - 1;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
	TIM_ClearITPendingBit(TIM3, TIM_FLAG_Update);
	TIM_Cmd(TIM3, ENABLE);
}
static void wait_t(void)
{
	while (!TIM_GetFlagStatus(TIM3, TIM_FLAG_Update));
	TIM_ClearITPendingBit(TIM3, TIM_FLAG_Update);
}

static void encode_release(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin   = ENCODE_PIN;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
	GPIO_Init(ENCODE_PORT, &GPIO_InitStructure);
	TIM_Cmd(TIM3, DISABLE);
}

static uint16_t _send_fmt(uint8_t x)
{
	uint16_t databit = x;

	databit |= 1 << 8;          //stop bit
	return (databit);
}

static uint8_t recv_chk(uint16_t x, uint16_t (*fun)(uint8_t))
{
	uint8_t data = (uint8_t)(x & 0xFF);

	return (x == fun(data));
}

#define MAX_BIT_CNT     (1 + 8 + 1)
static void put_char(uint8_t ch)
{
	uint16_t data;
	uint8_t i;

	data   = _send_fmt(ch);
	data <<= 0x01;
	data  &= (1 << 10) - 1;

	wait_t();
	for (i = 0; i < MAX_BIT_CNT; i++)
	{
		if (data & 0x01)
			TX_SDA_SET();
		else
			TX_SDA_CLR();
		data >>= 0x01;
		wait_t();
	}
}

static void send_to_sda(uint8_t buf[], uint8_t len)
{
	uint8_t i;

	ENCODE_SDA_OUT();
	wait_t();
	for (i = 0; i < len; i++)
	{
		put_char(buf[i]);
	}
}

#define MAX_TIME_OVER   (500)   ///n*100us debug modify 
static uint8_t recv_from_sda(uint8_t *c)
{
	uint16_t data         = 0, time_over = 0;
	volatile uint16_t bit = 1;
	uint8_t i;

	ENCODE_SDA_IN();
	while (RX_SDA_VALUE())
	{
		if (TIM_GetFlagStatus(TIM3, TIM_FLAG_Update) != RESET)
		{
			TIM_ClearITPendingBit(TIM3, TIM_FLAG_Update);
			time_over++;
			if (time_over >= MAX_TIME_OVER)
				return (0);
		}
		board_feed_wdg();
	}
	encode_timer_setmat(PERIOD >> 1);
	wait_t();
	encode_timer_setmat(PERIOD);
	for (i = 0; i < MAX_BIT_CNT - 1; i++)
	{
		wait_t();
		if (RX_SDA_VALUE())
			data |= bit;
		bit <<= 1;
	}
	if (recv_chk(data, _send_fmt))
	{
		*c = (uint8_t)(data & 0xFF);
		return (1);
	}
	return (0);
}

static struct CODE_FRAME *get_code_frame(uint8_t buf[], uint8_t len)
{
	uint8_t i, length;
	struct CODE_FRAME *pframe = NULL;

	for (i = 0; i < len; i++)
	{
start_lbl:
		if (STC == buf[i])
			break;
	}
	if (i >= len)
		return (NULL);

	pframe = (struct CODE_FRAME *)&buf[i];
	length = pframe->len;
	if ((length > len) || (pframe->data[length] != checksum(&buf[i], length + 2)))
	{
		i++;
		goto start_lbl;
	}
	return (pframe);
}

static uint8_t get_device_infor(uint8_t buf[], uint8_t len)
{
	struct CODE_FRAME *pframe;

	if (len < 2)
		return (0);

	pframe = get_code_frame(buf, len);
	if (!pframe || (ENCODE_LEN != pframe->len)) return (0);

	memcpy(&setting.encode, pframe->data, pframe->len);
	setting_save();
	setting_load();
	memcpy(pframe->data, &setting.encode, pframe->len);
	pframe->data[pframe->len] = checksum((uint8_t *)pframe, pframe->len + 2);
	send_to_sda((uint8_t *)pframe, pframe->len + 3);
	return (1);
}

extern uint8_t g_frame_buf[0x400];
void device_encode_opt(void)
{
	static uint8_t tmp_buf[4] = { 0x7E, 0x01, 0x00, 0x7F };
	uint8_t try_cnt           = 3, ret, len;

	encode_gpio_init();
	encode_timer_init();

    while (try_cnt--)
    {
        send_to_sda(tmp_buf, sizeof((tmp_buf)));

        len = 0;
        do
        {
            board_feed_wdg();
            ret = recv_from_sda(&g_frame_buf[len]);
            len++;
        }
        while (ret);

        if (get_device_infor(g_frame_buf, len)) break;
        board_feed_wdg();
    }
    encode_release();
}
