/**
 ****************************************************************************************************
 * @file        oled.c
 * @author      ??????????(ALIENTEK)
 * @version     V1.0
 * @date        2020-04-22
 * @brief       OLED ????????
 * @license     Copyright (c) 2020-2032, ?????????????????????
 ****************************************************************************************************
 * @attention
 *
 * ?????:??????? STM32F103??????
 * ???????:www.yuanzige.com
 * ???????:www.openedv.com
 * ??????:www.alientek.com
 * ??????:openedv.taobao.com
 *
 * ??????
 * V1.0 20200421
 * ????��???
 *
 ****************************************************************************************************
 */

#include "stdlib.h"
#include "oled.h"
#include "oledfont.h"

/*
 * OLED?????
 * ????????8??????, 128,?????128??, 8?????64??, ??��?????????.
 * ????:g_oled_gram[0][0],??????????,??1~8?��?????. g_oled_gram[0][0].0,?????????(0,0)
 * ?????: g_oled_gram[1][0].1,???????(1,1), g_oled_gram[10][1].2,???????(10,10),
 *
 * ?????????(??��?????????).
 * [0]0 1 2 3 ... 127
 * [1]0 1 2 3 ... 127
 * [2]0 1 2 3 ... 127
 * [3]0 1 2 3 ... 127
 * [4]0 1 2 3 ... 127
 * [5]0 1 2 3 ... 127
 * [6]0 1 2 3 ... 127
 * [7]0 1 2 3 ... 127
 */
static uint8_t g_oled_gram[128][8];

/**
 * @brief       ??????��OLED
 * @param       ??
 * @retval      ??
 */
void oled_refresh_gram(void) {
	uint8_t i, n;

	for (i = 0; i < 8; i++) {
		oled_wr_byte(0xb0 + i, OLED_CMD); /* ??????????0~7?? */
		oled_wr_byte(0x00, OLED_CMD); /* ???????��?��??��??? */
		oled_wr_byte(0x10, OLED_CMD); /* ???????��?��??��??? */

		for (n = 0; n < 128; n++) {
			oled_wr_byte(g_oled_gram[n][i], OLED_DATA);
		}
	}
}

#if OLED_MODE == 1    /* ???8080????????OLED */

/**
 * @brief       ????????????OLED??????8��????
 * @param       data: ??????????
 * @retval      ??
 */
static void oled_data_out(uint8_t data)
{
    GPIOC->ODR = (GPIOC->ODR & 0XFF00) | (data & 0X00FF);
}

/**
 * @brief       ??OLED��????????
 * @param       data: ??????????
 * @param       cmd: ????/?????? 0,???????;1,???????;
 * @retval      ??
 */
static void oled_wr_byte(uint8_t data, uint8_t cmd)
{
    oled_data_out(data);
    OLED_RS(cmd);
    OLED_CS(0);
    OLED_WR(0);
    OLED_WR(1);
    OLED_CS(1);
    OLED_RS(1);
}

#else   /* ???SPI????OLED */

/**
 * @brief       ??OLED��????????
 * @param       data: ??????????
 * @param       cmd: ????/?????? 0,???????;1,???????;
 * @retval      ??
 */
static void oled_wr_byte(uint8_t data, uint8_t cmd) {
	uint8_t i;
	OLED_RS(cmd); /* ��???? */
	OLED_CS(0);

	for (i = 0; i < 8; i++) {
		OLED_SCLK(0);

		if (data & 0x80) {
			OLED_SDIN(1);
		} else {
			OLED_SDIN(0);
		}

		OLED_SCLK(1);
		data <<= 1;
	}

	OLED_CS(1);
	OLED_RS(1);
}

#endif

/**
 * @brief       ????OLED???
 * @param       ??
 * @retval      ??
 */
void oled_display_on(void) {
	oled_wr_byte(0X8D, OLED_CMD); /* SET DCDC???? */
	oled_wr_byte(0X14, OLED_CMD); /* DCDC ON */
	oled_wr_byte(0XAF, OLED_CMD); /* DISPLAY ON */
}

/**
 * @brief       ???OLED???
 * @param       ??
 * @retval      ??
 */
void oled_display_off(void) {
	oled_wr_byte(0X8D, OLED_CMD); /* SET DCDC???? */
	oled_wr_byte(0X10, OLED_CMD); /* DCDC OFF */
	oled_wr_byte(0XAE, OLED_CMD); /* DISPLAY OFF */
}

/**
 * @brief       ????????,??????,?????????????!??????????!!!
 * @param       ??
 * @retval      ??
 */
void oled_clear(void) {
	uint8_t i, n;

	for (i = 0; i < 8; i++)
		for (n = 0; n < 128; n++)
			g_oled_gram[n][i] = 0X00;

	oled_refresh_gram(); /* ??????? */
}

/**
 * @brief       OLED????
 * @param       x  : 0~127
 * @param       y  : 0~63
 * @param       dot: 1 ??? 0,???
 * @retval      ??
 */
void oled_draw_point(uint8_t x, uint8_t y, uint8_t dot) {
	uint8_t pos, bx, temp = 0;

	if (x > 127 || y > 63)
		return; /* ??????��??. */

	pos = y / 8; /* ????GRAM?????y????????????, ?????????��8???????? */

	bx = y % 8; /* ?????,???????y????????????��??,????(y)��?? */
	temp = 1 << bx; /* ??��??????��?, ???y?????bit��??,????bit????1 */

	if (dot) /* ?????? */
	{
		g_oled_gram[x][pos] |= temp;
	} else /* ?????,??????? */
	{
		g_oled_gram[x][pos] &= ~temp;
	}
}

/**
 * @brief       OLED??????????
 *   @note:     ???:??????: x1<=x2; y1<=y2  0<=x1<=127  0<=y1<=63
 * @param       x1,y1: ???????
 * @param       x2,y2: ???????
 * @param       dot: 1 ??? 0,???
 * @retval      ??
 */
void oled_fill(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t dot) {
	uint8_t x, y;

	for (x = x1; x <= x2; x++) {
		for (y = y1; y <= y2; y++)
			oled_draw_point(x, y, dot);
	}

	oled_refresh_gram(); /* ??????? */
}

/**
 * @brief       ?????��???????????,???????????
 * @param       x   : 0~127
 * @param       y   : 0~63
 * @param       size: ??????? 12/16/24
 * @param       mode: 0,???????;1,???????
 * @retval      ??
 */
void oled_show_char(uint8_t x, uint8_t y, uint8_t chr, uint8_t size,
		uint8_t mode) {
	uint8_t temp, t, t1;
	uint8_t y0 = y;
	uint8_t *pfont = 0;
	uint8_t csize = (size / 8 + ((size % 8) ? 1 : 0)) * (size / 2); /* ?????????????????????????????? */
	chr = chr - ' '; /* ?????????,?????????????��??,???????????? */

	if (size == 12) /* ????1206???? */
	{
		pfont = (uint8_t*) oled_asc2_1206[chr];
	} else if (size == 16) /* ????1608???? */
	{
		pfont = (uint8_t*) oled_asc2_1608[chr];
	} else if (size == 24) /* ????2412???? */
	{
		pfont = (uint8_t*) oled_asc2_2412[chr];
	} else /* ??��???? */
	{
		return;
	}

	for (t = 0; t < csize; t++) {
		temp = pfont[t];

		for (t1 = 0; t1 < 8; t1++) {
			if (temp & 0x80)
				oled_draw_point(x, y, mode);
			else
				oled_draw_point(x, y, !mode);

			temp <<= 1;
			y++;

			if ((y - y0) == size) {
				y = y0;
				x++;
				break;
			}
		}
	}
}

/**
 * @brief       ???????, m^n
 * @param       m: ????
 * @param       n: ???
 * @retval      ??
 */
static uint32_t oled_pow(uint8_t m, uint8_t n) {
	uint32_t result = 1;

	while (n--) {
		result *= m;
	}

	return result;
}

/**
 * @brief       ???len??????
 * @param       x,y : ???????
 * @param       num : ???(0 ~ 2^32)
 * @param       len : ????????��??
 * @param       size: ??????? 12/16/24
 * @retval      ??
 */
void oled_show_num(uint8_t x, uint8_t y, uint32_t num, uint8_t len,
		uint8_t size) {
	uint8_t t, temp;
	uint8_t enshow = 0;

	for (t = 0; t < len; t++) /* ???????��????? */
	{
		temp = (num / oled_pow(10, len - t - 1)) % 10; /* ??????��?????? */

		if (enshow == 0 && t < (len - 1)) /* ?????????,?????��???? */
		{
			if (temp == 0) {
				oled_show_char(x + (size / 2) * t, y, ' ', size, 1); /* ??????,?�� */
				continue; /* ????????�� */
			} else {
				enshow = 1; /* ?????? */
			}
		}

		oled_show_char(x + (size / 2) * t, y, temp + '0', size, 1); /* ?????? */
	}
}

/**
 * @brief       ????????
 * @param       x,y : ???????
 * @param       size: ??????? 12/16/24
 * @param       *p  : ????????,????????????
 * @retval      ??
 */
void oled_show_string(uint8_t x, uint8_t y, const char *p, uint8_t size) {
	while ((*p <= '~') && (*p >= ' ')) /* ?��???????????! */
	{
		if (x > (128 - (size / 2))) /* ?????? */
		{
			x = 0;
			y += size; /* ???? */
		}

		if (y > (64 - size)) /* ?????? */
		{
			y = x = 0;
			oled_clear();
		}

		oled_show_char(x, y, *p, size, 1); /* ????????? */
		x += size / 2; /* ASCII????????????????? */
		p++;
	}
}

/**
 * @brief       ?????OLED(SSD1306)
 * @param       ??
 * @retval      ??
 */
void oled_init(void) {
	GPIO_InitTypeDef gpio_init_struct;
	__HAL_RCC_GPIOB_CLK_ENABLE(); /* ???PORTC??? */

#if OLED_MODE==1    /* ???8080?????? */

    /* PC0 ~ 7 ???? */
    gpio_init_struct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;                
    gpio_init_struct.Mode = GPIO_MODE_OUTPUT_PP;            /* ??????? */
    gpio_init_struct.Pull = GPIO_PULLUP;                    /* ???? */
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_MEDIUM;        /* ???? */
    HAL_GPIO_Init(GPIOC, &gpio_init_struct);                /* PC0 ~ 7 ???? */

    gpio_init_struct.Pin = GPIO_PIN_3|GPIO_PIN_6;           /* PD3, PD6 ???? */
    gpio_init_struct.Mode = GPIO_MODE_OUTPUT_PP;            /* ??????? */
    gpio_init_struct.Pull = GPIO_PULLUP;                    /* ???? */
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_MEDIUM;        /* ???? */
    HAL_GPIO_Init(GPIOD, &gpio_init_struct);                /* PD3, PD6 ???? */
    
    gpio_init_struct.Pin = GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
    gpio_init_struct.Mode = GPIO_MODE_OUTPUT_PP;            /* ??????? */
    gpio_init_struct.Pull = GPIO_PULLUP;                    /* ???? */
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_MEDIUM;        /* ???? */
    HAL_GPIO_Init(GPIOG, &gpio_init_struct);                /* WR/RD/RST?????????? */

    OLED_WR(1);
    OLED_RD(1);

#else               /* ???4??SPI ?????? */

	gpio_init_struct.Pin = OLED_SPI_RST_PIN; /* RST???? */
	gpio_init_struct.Mode = GPIO_MODE_OUTPUT_PP; /* ??????? */
	gpio_init_struct.Pull = GPIO_PULLUP; /* ???? */
	gpio_init_struct.Speed = GPIO_SPEED_FREQ_MEDIUM; /* ???? */
	HAL_GPIO_Init(OLED_SPI_RST_PORT, &gpio_init_struct); /* RST?????????? */

	gpio_init_struct.Pin = OLED_SPI_CS_PIN; /* CS???? */
	gpio_init_struct.Mode = GPIO_MODE_OUTPUT_PP; /* ??????? */
	gpio_init_struct.Pull = GPIO_PULLUP; /* ???? */
	gpio_init_struct.Speed = GPIO_SPEED_FREQ_MEDIUM; /* ???? */
	HAL_GPIO_Init(OLED_SPI_CS_PORT, &gpio_init_struct); /* CS?????????? */

	gpio_init_struct.Pin = OLED_SPI_RS_PIN; /* RS???? */
	gpio_init_struct.Mode = GPIO_MODE_OUTPUT_PP; /* ??????? */
	gpio_init_struct.Pull = GPIO_PULLUP; /* ???? */
	gpio_init_struct.Speed = GPIO_SPEED_FREQ_MEDIUM; /* ???? */
	HAL_GPIO_Init(OLED_SPI_RS_PORT, &gpio_init_struct); /* RS?????????? */

	gpio_init_struct.Pin = OLED_SPI_SCLK_PIN; /* SCLK???? */
	gpio_init_struct.Mode = GPIO_MODE_OUTPUT_PP; /* ??????? */
	gpio_init_struct.Pull = GPIO_PULLUP; /* ???? */
	gpio_init_struct.Speed = GPIO_SPEED_FREQ_MEDIUM; /* ???? */
	HAL_GPIO_Init(OLED_SPI_SCLK_PORT, &gpio_init_struct); /* SCLK?????????? */

	gpio_init_struct.Pin = OLED_SPI_SDIN_PIN; /* SDIN?????????? */
	gpio_init_struct.Mode = GPIO_MODE_OUTPUT_PP; /* ??????? */
	gpio_init_struct.Pull = GPIO_PULLUP; /* ???? */
	gpio_init_struct.Speed = GPIO_SPEED_FREQ_MEDIUM; /* ???? */
	HAL_GPIO_Init(OLED_SPI_SDIN_PORT, &gpio_init_struct); /* SDIN?????????? */

	OLED_SDIN(1);
	OLED_SCLK(1);
#endif
	OLED_CS(1);
	OLED_RS(1);

	OLED_RST(0);
	//delay_ms(100);
	HAL_Delay(100);
	OLED_RST(1);

	oled_wr_byte(0xAE, OLED_CMD); /* ?????? */
	oled_wr_byte(0xD5, OLED_CMD); /* ?????????????,????? */
	oled_wr_byte(80, OLED_CMD); /* [3:0],???????;[7:4],????? */
	oled_wr_byte(0xA8, OLED_CMD); /* ????????��?? */
	oled_wr_byte(0X3F, OLED_CMD); /* ???0X3F(1/64) */
	oled_wr_byte(0xD3, OLED_CMD); /* ?????????? */
	oled_wr_byte(0X00, OLED_CMD); /* ????0 */

	oled_wr_byte(0x40, OLED_CMD); /* ???????????? [5:0],????. */

	oled_wr_byte(0x8D, OLED_CMD); /* ???????? */
	oled_wr_byte(0x14, OLED_CMD); /* bit2??????/??? */
	oled_wr_byte(0x20, OLED_CMD); /* ??????????? */
	oled_wr_byte(0x02, OLED_CMD); /* [1:0],00???��????;01???��????;10,??????;???10; */
	oled_wr_byte(0xA1, OLED_CMD); /* ???????????,bit0:0,0->0;1,0->127; */
	oled_wr_byte(0xC8, OLED_CMD); /* ????COM??��??;bit3:0,?????;1,??????? COM[N-1]->COM0;N:????��?? */
	oled_wr_byte(0xDA, OLED_CMD); /* ????COM??????????? */
	oled_wr_byte(0x12, OLED_CMD); /* [5:4]???? */

	oled_wr_byte(0x81, OLED_CMD); /* ???????? */
	oled_wr_byte(0xEF, OLED_CMD); /* 1~255;???0X7F (????????,??????) */
	oled_wr_byte(0xD9, OLED_CMD); /* ???????????? */
	oled_wr_byte(0xf1, OLED_CMD); /* [3:0],PHASE 1;[7:4],PHASE 2; */
	oled_wr_byte(0xDB, OLED_CMD); /* ????VCOMH ??????? */
	oled_wr_byte(0x30, OLED_CMD); /* [6:4] 000,0.65*vcc;001,0.77*vcc;011,0.83*vcc; */

	oled_wr_byte(0xA4, OLED_CMD); /* ??????????;bit0:1,????;0,???;(????/????) */
	oled_wr_byte(0xA6, OLED_CMD); /* ??????????;bit0:1,???????;0,??????? */
	oled_wr_byte(0xAF, OLED_CMD); /* ??????? */
	oled_clear();
}

