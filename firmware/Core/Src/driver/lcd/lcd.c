/*
 * lcd.c
 *
 *  Created on: 2026. 10. 21.
 *      Author: rhoblack
 */

#include "lcd.h"

#define LCD_RS		0
#define LCD_RW		1
#define LCD_E		2
#define LCD_BL		3

I2C_HandleTypeDef *lcd_hi2c;

uint8_t lcdData = 0;
static uint8_t lcd_transfer_failed = 0;
static uint32_t lcd_last_failure_ms = 0;

#define LCD_I2C_ADDRESS (0x27U << 1)
#define LCD_I2C_TIMEOUT_MS 10U
#define LCD_RETRY_INTERVAL_MS 500U

void Lcd_sendData(uint8_t data)
{
	if (lcd_hi2c == NULL || lcd_transfer_failed) {
		return;
	}
	if (HAL_I2C_Master_Transmit(lcd_hi2c, LCD_I2C_ADDRESS, &data, 1,
	                            LCD_I2C_TIMEOUT_MS) != HAL_OK) {
		lcd_transfer_failed = 1;
		lcd_last_failure_ms = HAL_GetTick();
	}
}

void Lcd_cmdMode(void)
{
	lcdData &= ~(1 << LCD_RS);
}

void Lcd_dataMode(void)
{
	lcdData |= (1 << LCD_RS);
}

void Lcd_writeMode(void)
{
	lcdData &= ~(1 << LCD_RW);
}

void Lcd_enHigh(void)
{
	lcdData |= (1 << LCD_E);
	Lcd_sendData(lcdData); // send data into i2c interface.
}

void Lcd_enLow(void)
{
	lcdData &= ~(1 << LCD_E);
	Lcd_sendData(lcdData); // send data into i2c interface.
}

void Lcd_backlightOn(void)
{
	lcdData |= (1 << LCD_BL);
	Lcd_sendData(lcdData); // send data into i2c interface.
}

void Lcd_backlightOff(void)
{
	lcdData &= ~(1 << LCD_BL);
	Lcd_sendData(lcdData); // send data into i2c interface.
}

void Lcd_writeNibble(uint8_t data)
{
	Lcd_enHigh();
	lcdData = (lcdData & 0x0f) | (data & 0xf0);
	Lcd_sendData(lcdData); // send data into i2c interface.
	Lcd_enLow();
}

void Lcd_writeByte(uint8_t data)
{
	Lcd_writeNibble(data); // data 상위 4bit 전송
	data <<= 4;
	Lcd_writeNibble(data); // data 하위 4bit 전송
}

void Lcd_writeCmdData(uint8_t data)
{
	Lcd_cmdMode();
	Lcd_writeMode();
	Lcd_writeByte(data);
}

void Lcd_writeCharData(uint8_t data)
{
	Lcd_dataMode();
	Lcd_writeMode();
	Lcd_writeByte(data);
}

void Lcd_init(I2C_HandleTypeDef *hi2c)
{
	lcd_hi2c = hi2c;
	lcd_transfer_failed = 0;
	HAL_Delay(40);
	Lcd_cmdMode();
	Lcd_writeMode();
	Lcd_writeNibble(0x30);
	HAL_Delay(5);
	Lcd_writeNibble(0x30);
	HAL_Delay(1);
	Lcd_writeNibble(0x30);

	Lcd_writeNibble(0x20);
	Lcd_writeByte(0x28); // Fucntion set 4bit
	Lcd_writeByte(0x08); // display off
	Lcd_writeByte(0x01); // display clear
	Lcd_writeByte(0x06); // entry mode set
	Lcd_writeByte(0x0C); // display on
	Lcd_backlightOn();
}

void Lcd_writeString(char *str)
{
	for (int i=0; str[i] && !lcd_transfer_failed; i++) {
		Lcd_writeCharData(str[i]);
	}
}

void Lcd_gotoXY(uint8_t row, uint8_t col)
{
	col %= 16;
	row %= 2;

	uint8_t lcdRegAddr = (0x40 * row) + col;
	uint8_t cmd = 0x80 + lcdRegAddr;
	Lcd_writeCmdData(cmd);
}

void Lcd_writeStringXY(uint8_t row, uint8_t col, char *str)
{
	if (lcd_hi2c == NULL) {
		return;
	}
	if (lcd_transfer_failed) {
		if ((uint32_t)(HAL_GetTick() - lcd_last_failure_ms) < LCD_RETRY_INTERVAL_MS) {
			return;
		}
		if (HAL_I2C_IsDeviceReady(lcd_hi2c, LCD_I2C_ADDRESS, 1,
		                          LCD_I2C_TIMEOUT_MS) != HAL_OK) {
			lcd_last_failure_ms = HAL_GetTick();
			return;
		}
		Lcd_init(lcd_hi2c);
		if (lcd_transfer_failed) {
			return;
		}
	}
	Lcd_gotoXY(row, col);
	if (!lcd_transfer_failed) {
		Lcd_writeString(str);
	}
}












