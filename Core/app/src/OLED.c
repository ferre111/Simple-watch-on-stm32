/*
 * OLED.c
 *
 *  Created on: Nov 19, 2020
 *      Author: Wiktor Lechowicz
 */
#include "OLED.h"
#include "i2c.h"


// init byteStream

uint8_t initSequence[32] = {

		0x20,					// set memory addressing mode
		0x02,					// page addressing
		//0x01,					// vertical increment

		0x22,					// set page addressing
		0x00,					// start
		OLED_NUM_OF_PAGES - 1,	// stop

		0x21,					// set column addressing
		0x00,					// start
		OLED_X_SIZE - 1,		// stop

		0xB0,					// set initial page 0

		0x00,					// set initial column 0
		0x10,

		0xD5,					// set display clock frequency and prescaler
		0x80,

		0xD9,					// set pre-charge period
		0x22,

		0xA8,					// set MUX ratio
		0x3F,

		0xD3,					// set display offset
		0x00,

		0x40,					// set display start line

		0xA1,					// mirror horizontally
		// 0xA0

		0xC0,					// set COM scan direction

		0xDA,					// set com pins hw configuration
		0x12,

		0x81,					// set contrast
		0x9F,

		0xC8,					// mirror vertically
		//0xC0

		OLED_CMD_EntireDisplayOnPreserve,

		OLED_CMD_SetNotInversedDisplay,

		OLED_CMD_EnableChargePumpDuringDisplay,
		OLED_CMD_SetDisplayON,
		OLED_CMD_SetDisplayON,

};
/**
  * @brief send command to SSD1306 driver
  * @param OLED_CMD... command
  * @retval void
  */
void OLED_sendCommand(uint8_t command){
	HAL_I2C_Mem_Write(&OLED_I2C_HANDLE, OLED_ADDRESS, 0x00, 1, &command, 1, HAL_MAX_DELAY);
}

/**
 *  @brief send stream of commands to SSD1306 driver
 *  @param stream array of commands
 *  @param streamLength array length
 *  @retval void
 */
void OLED_sendCommandStream(uint8_t stream[], uint8_t streamLength){
	HAL_I2C_Mem_Write(&OLED_I2C_HANDLE, OLED_ADDRESS, 0x01, 1, stream, streamLength, HAL_MAX_DELAY);
}

/**
 *  @brief set pointer to column
 *  @param page - uint8_t between 0 and OLED_NUM_OF_PAGES - 1
 *  @param column - uint8_t between 0 and OLED_X_SIZE - 1
 *  @retval void
 */
void OLED_setAddress(uint8_t page, uint8_t column){
	// set address
	uint8_t stream[3] = {
		0xB0 | (page & 0x07),
		column & 0x0F,
		0x10 | ((0xF0 & column) >> 4),
	};
	HAL_I2C_Mem_Write(&OLED_I2C_HANDLE, OLED_ADDRESS, OLED_CONTROL_BYTE_ | _OLED_COMMAND | _OLED_MULTIPLE_BYTES, 1,
			stream, 3, HAL_MAX_DELAY);
}

/**
 *  @brief write data to column specified by OLED_setAddress function
 *  @param data byte written to column
 *  @retval void
 */
void OLED_writeData(uint8_t data)
{
	HAL_I2C_Mem_Write(&OLED_I2C_HANDLE, OLED_ADDRESS, OLED_CONTROL_BYTE_ | _OLED_DATA | _OLED_SINGLE_BYTE,
			1, &data, 1, HAL_MAX_DELAY);
}



/**
  * @brief initialize SSD1306 driver according to initSequence
  * @return void
  */
void OLED_Init(){

	OLED_sendCommandStream(initSequence, 32);
	OLED_clearScreen();
	OLED_update();
}

/**
 *  @brief update display with values from buffer
 *  @return void
 */
void OLED_update(){

	for(uint8_t p = 0; p < OLED_NUM_OF_PAGES; p++){
		OLED_setAddress(p, 0);
		HAL_I2C_Mem_Write(&OLED_I2C_HANDLE, OLED_ADDRESS, OLED_CONTROL_BYTE_ | _OLED_DATA | _OLED_MULTIPLE_BYTES,
				1, OLED_displayBuffer[p], OLED_X_SIZE, HAL_MAX_DELAY);
	}
}

// TODO make this work with buffer
void OLED_setPixel(uint8_t x, uint8_t y){
	// set address
	uint8_t stream[3] = {
		0xB0 | ( ((uint8_t) (y / 8)) & 0x07),
		x & 0x0F,
		0x10 | ((0xF0 & x) >> 4),
	};
	HAL_I2C_Mem_Write(&OLED_I2C_HANDLE, OLED_ADDRESS, OLED_CONTROL_BYTE_ | _OLED_COMMAND | _OLED_MULTIPLE_BYTES, 1,
			stream, 3, HAL_MAX_DELAY);
	uint8_t temp = 1 << (y % 8);
	HAL_I2C_Mem_Write(&OLED_I2C_HANDLE, OLED_ADDRESS, OLED_CONTROL_BYTE_ | _OLED_DATA | _OLED_SINGLE_BYTE, 1,
			&temp, 1, HAL_MAX_DELAY);
}

// TODO make work with buffer
void OLED_clearPixel(uint8_t x, uint8_t y){

}

// OK, writes to buffer
void OLED_clearScreen(){
	for(uint8_t p = 0; p < OLED_NUM_OF_PAGES; p++){
		for(uint8_t x = 0; x < OLED_X_SIZE; x++){
			OLED_displayBuffer[p][x] = 0;
		}
	}
}
