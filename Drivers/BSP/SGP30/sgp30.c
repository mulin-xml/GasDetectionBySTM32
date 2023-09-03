#include "sgp30.h"
#include "./SYSTEM/delay/delay.h"

void SGP30_GPIO_Init(void) {
	GPIO_InitTypeDef gpio_init_struct;

	gpio_init_struct.Pin = SGP30_SCL_GPIO_PIN; /* RST???? */
	gpio_init_struct.Mode = GPIO_MODE_OUTPUT_PP; /* ??????? */
	gpio_init_struct.Speed = GPIO_SPEED_FREQ_MEDIUM; /* ???? */
	HAL_GPIO_Init(SGP30_SCL_GPIO_PORT, &gpio_init_struct); /* RST?????????? */

	gpio_init_struct.Pin = SGP30_SDA_GPIO_PIN; /* RST???? */
	gpio_init_struct.Mode = GPIO_MODE_OUTPUT_PP; /* ??????? */
	gpio_init_struct.Speed = GPIO_SPEED_FREQ_MEDIUM; /* ???? */
	HAL_GPIO_Init(SGP30_SDA_GPIO_PORT, &gpio_init_struct); /* RST?????????? */
}

void SDA_OUT(void) {
	GPIO_InitTypeDef gpio_init_struct;
	gpio_init_struct.Pin = SGP30_SDA_GPIO_PIN;
	gpio_init_struct.Mode = GPIO_MODE_OUTPUT_PP;
	gpio_init_struct.Speed = GPIO_SPEED_FREQ_MEDIUM;
	HAL_GPIO_Init(SGP30_SDA_GPIO_PORT, &gpio_init_struct);
}

void SDA_IN(void) {
	GPIO_InitTypeDef gpio_init_struct;
	gpio_init_struct.Pin = SGP30_SDA_GPIO_PIN;
	gpio_init_struct.Mode = GPIO_MODE_AF_INPUT;
	gpio_init_struct.Speed = GPIO_SPEED_FREQ_MEDIUM;
	HAL_GPIO_Init(SGP30_SDA_GPIO_PORT, &gpio_init_struct);
}

//????IIC??????
void SGP30_IIC_Start(void) {
	SDA_OUT();
	HAL_GPIO_WritePin(SGP30_SDA_GPIO_PORT, SGP30_SDA_GPIO_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(SGP30_SCL_GPIO_PORT, SGP30_SCL_GPIO_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(SGP30_SCL_GPIO_PORT, SGP30_SCL_GPIO_PIN, GPIO_PIN_SET);
	delay_us(20);

	HAL_GPIO_WritePin(SGP30_SDA_GPIO_PORT, SGP30_SDA_GPIO_PIN, GPIO_PIN_RESET);	//START:when CLK is high,DATA change form high to low
	delay_us(20);
	HAL_GPIO_WritePin(SGP30_SCL_GPIO_PORT, SGP30_SCL_GPIO_PIN, GPIO_PIN_RESET); //??I2C????????????????????
}

//????IIC?????
void SGP30_IIC_Stop(void) {
	SDA_OUT();
	HAL_GPIO_WritePin(SGP30_SCL_GPIO_PORT, SGP30_SCL_GPIO_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(SGP30_SDA_GPIO_PORT, SGP30_SDA_GPIO_PIN, GPIO_PIN_RESET);	//STOP:when CLK is high DATA change form low to high
	delay_us(20);
	HAL_GPIO_WritePin(SGP30_SCL_GPIO_PORT, SGP30_SCL_GPIO_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(SGP30_SDA_GPIO_PORT, SGP30_SDA_GPIO_PIN, GPIO_PIN_SET);//????I2C??????????
	delay_us(20);
}

//????????????
//???????1????????????
//        0???????????
uint8_t SGP30_IIC_Wait_Ack(void) {
	uint8_t ucErrTime = 0;
	SDA_IN();
	HAL_GPIO_WritePin(SGP30_SDA_GPIO_PORT, SGP30_SDA_GPIO_PIN, GPIO_PIN_SET);
	delay_us(10);
	HAL_GPIO_WritePin(SGP30_SCL_GPIO_PORT, SGP30_SCL_GPIO_PIN, GPIO_PIN_SET);
	delay_us(10);
	while (SGP30_SDA_READ()) {
		ucErrTime++;
		if (ucErrTime > 250) {
			SGP30_IIC_Stop();
			return 1;
		}
	}
	HAL_GPIO_WritePin(SGP30_SCL_GPIO_PORT, SGP30_SCL_GPIO_PIN, GPIO_PIN_RESET); //??????0
	return 0;
}

//????ACK???
void SGP30_IIC_Ack(void) {
	HAL_GPIO_WritePin(SGP30_SCL_GPIO_PORT, SGP30_SCL_GPIO_PIN, GPIO_PIN_RESET);
	SDA_OUT();
	HAL_GPIO_WritePin(SGP30_SDA_GPIO_PORT, SGP30_SDA_GPIO_PIN, GPIO_PIN_RESET);
	delay_us(20);
	HAL_GPIO_WritePin(SGP30_SCL_GPIO_PORT, SGP30_SCL_GPIO_PIN, GPIO_PIN_SET);
	delay_us(20);
	HAL_GPIO_WritePin(SGP30_SCL_GPIO_PORT, SGP30_SCL_GPIO_PIN, GPIO_PIN_RESET);
}

//??????ACK???
void SGP30_IIC_NAck(void) {
	HAL_GPIO_WritePin(SGP30_SCL_GPIO_PORT, SGP30_SCL_GPIO_PIN, GPIO_PIN_RESET);
	SDA_OUT();
	HAL_GPIO_WritePin(SGP30_SDA_GPIO_PORT, SGP30_SDA_GPIO_PIN, GPIO_PIN_SET);
	delay_us(20);
	HAL_GPIO_WritePin(SGP30_SCL_GPIO_PORT, SGP30_SCL_GPIO_PIN, GPIO_PIN_SET);
	delay_us(20);
	HAL_GPIO_WritePin(SGP30_SCL_GPIO_PORT, SGP30_SCL_GPIO_PIN, GPIO_PIN_RESET);
}

//IIC??????????
//?????????????
//1???????
//0???????
void SGP30_IIC_Send_Byte(uint8_t txd) {
	uint8_t t;
	SDA_OUT();
	HAL_GPIO_WritePin(SGP30_SCL_GPIO_PORT, SGP30_SCL_GPIO_PIN, GPIO_PIN_RESET); //????????????????
	for (t = 0; t < 8; t++) {
		if ((txd & 0x80) >> 7)
			HAL_GPIO_WritePin(SGP30_SDA_GPIO_PORT, SGP30_SDA_GPIO_PIN,
					GPIO_PIN_SET);
		else
			HAL_GPIO_WritePin(SGP30_SDA_GPIO_PORT, SGP30_SDA_GPIO_PIN,
					GPIO_PIN_RESET);
		txd <<= 1;
		delay_us(20);
		HAL_GPIO_WritePin(SGP30_SCL_GPIO_PORT, SGP30_SCL_GPIO_PIN,
				GPIO_PIN_SET);
		delay_us(20);
		HAL_GPIO_WritePin(SGP30_SCL_GPIO_PORT, SGP30_SCL_GPIO_PIN,
				GPIO_PIN_RESET);
		delay_us(20);
	}
	delay_us(20);

}

//??1??????ack=1???????ACK??ack=0??????nACK
uint16_t SGP30_IIC_Read_Byte(uint8_t ack) {
	uint8_t i;
	uint16_t receive = 0;
	SDA_IN();
	for (i = 0; i < 8; i++) {
		HAL_GPIO_WritePin(SGP30_SCL_GPIO_PORT, SGP30_SCL_GPIO_PIN,
				GPIO_PIN_RESET);
		delay_us(20);
		HAL_GPIO_WritePin(SGP30_SCL_GPIO_PORT, SGP30_SCL_GPIO_PIN,
				GPIO_PIN_SET);
		receive <<= 1;
		if (SGP30_SDA_READ())
			receive++;
		delay_us(20);
	}
	if (!ack)
		SGP30_IIC_NAck(); 	    	//????nACK
	else
		SGP30_IIC_Ack(); //????ACK
	return receive;
}

//?????IIC???
void SGP30_Init(void) {
	SGP30_GPIO_Init();
	SGP30_Write(0x20, 0x03);
//	SGP30_ad_write(0x20,0x61);
//	SGP30_ad_write(0x01,0x00);
}

void SGP30_Write(uint8_t a, uint8_t b) {
	SGP30_IIC_Start();
	SGP30_IIC_Send_Byte(SGP30_write); //???????????+з╒???
	SGP30_IIC_Wait_Ack();
	SGP30_IIC_Send_Byte(a);		//??????????
	SGP30_IIC_Wait_Ack();
	SGP30_IIC_Send_Byte(b);
	SGP30_IIC_Wait_Ack();
	SGP30_IIC_Stop();
	delay_ms(100);
}

uint32_t SGP30_Read(void) {
	uint32_t dat;
	uint8_t crc;
	SGP30_IIC_Start();
	SGP30_IIC_Send_Byte(SGP30_read); //???????????+?????
	SGP30_IIC_Wait_Ack();
	dat = SGP30_IIC_Read_Byte(1);
	dat <<= 8;
	dat += SGP30_IIC_Read_Byte(1);
	crc = SGP30_IIC_Read_Byte(1); //crc????????
	crc = crc;  //?????ио????????
	dat <<= 8;
	dat += SGP30_IIC_Read_Byte(1);
	dat <<= 8;
	dat += SGP30_IIC_Read_Byte(0);
	SGP30_IIC_Stop();
	return (dat);
}

