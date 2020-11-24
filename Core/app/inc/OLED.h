/*
 * OLED.h
 *
 *  Created on: Nov 19, 2020
 *      Author: Wiktor Lechowicz
 *
 *      This is library for SSD1306 based OLED with I2C interface.
 *      To use this you have to specify I2C module in line 14
 */

#include "stm32f1xx.h"
#include "stm32f1xx_hal_i2c.h"

#ifndef OLED_H_
#define OLED_H_

#define OLED_I2C_HANDLE hi2c1

// HW specific defines
#define OLED_X_SIZE									128
#define OLED_Y_SIZE									64
#define OLED_NUM_OF_PAGES							8

#define OLED_ADDRESS 0b01111000

// Control byte options
#define OLED_CONTROL_BYTE_				0b00000000
	#define _OLED_SINGLE_BYTE					0b10000000
	#define _OLED_MULTIPLE_BYTES				0b00000000
	#define _OLED_COMMAND						0b00000000
	#define _OLED_DATA							0b01000000


// basic command set
#define	OLED_CMD_EntireDisplayOnPreserve 			0xA4
#define	OLED_CMD_EntireDisplayOn 					0xA5
#define OLED_CMD_SetNotInversedDisplay				0xA6
#define OLED_CMD_SetInversedDisplay					0xA7
#define OLED_CMD_SetDisplayON						0xAF
#define OLED_CMD_SetDisplayOFF						0xAE
#define OLED_CMD_EnableChargePumpDuringDisplay		0x8D


// scrolling command table
// TODO


/* display initialziation */
void OLED_Init();

/* These are functions for sending configuration commans */
void OLED_sendCommand(uint8_t command);

void OLED_sendCommandStream(uint8_t stream[], uint8_t streamLength);

/* function to set pointer to specific column of display */
void OLED_setAddress(uint8_t page, uint8_t column);

/* writes value to column specified by OLED_setAddress() */
void OLED_writeData(uint8_t data);

/* TODO */
void OLED_setPixel(uint8_t x, uint8_t y);
void OLED_clearPixel(uint8_t x, uint8_t y);

/* Clear entire screen */
void OLED_clearScreen();

/* update display with values from displayBUffer */
void OLED_update();

// display buffer 7 pages of 128 8-bit columns
//   -> 0 		1 		2 3 4 ... 127
//  |  [byte]   [byte]  . . .     .
//  v  [byte]	.		.
//  0	.				  .
//  1   .
//  .
//  .
//  7   .
uint8_t OLED_displayBuffer[OLED_NUM_OF_PAGES][OLED_X_SIZE];




#endif
