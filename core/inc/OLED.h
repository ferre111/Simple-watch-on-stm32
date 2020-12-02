/*
 * OLED.h
 *
 *  Created on: Nov 19, 2020
 *      Author: Wiktor Lechowicz
 *
 *      This is library for SSD1306 based OLED with I2C interface.
 *      To use this you have to specify I2C module ?where?
 */

#include "stm32f1xx.h"
#include "stm32f1xx_hal_i2c.h"

#ifndef OLED_H_
#define OLED_H_

// * DEFINES TO BE MODIFIED BY USER */
#define OLED_I2C_HANDLE hi2c2

#define OLED_X_SIZE                                 128
#define OLED_Y_SIZE                                 64
#define OLED_NUM_OF_PAGES                           8

#define OLED_PRESERVE_TRUE                          0
#define OLED_PRESERVE_FALSE                         1

/* API FUNCTIONS */

/**
  * @brief initialize SSD1306 driver according to initSequence
  * @return void
  */
void OLED_Init();


/**
 *  @brief Clear entire screen
 *  @reval void
 */
void OLED_clearScreen();


/**
 *  @brief Update display with values from buffer.
 *  @return void
 */
void OLED_update();

// ==================================================================================
// use OLED_update() functions to apply changes made by functions below on display
// ==================================================================================
/**
 *  @brief Print text on display in specified place(buffered)
 *  @param verse - verse number in range from 0 to OLED_NUM_OF_PAGES
 *  @param column - column number in range from 0 to OLED_X_SIZE
 *  @param text - text to be printed
 */
void OLED_printText(uint8_t verse, uint8_t column, char * text);

/** TODO
 * @brief Draw image on display (buffered)
 * @param verse - verse number of left upper corner in range from 0 to OLED_NUM_OF_PAGES
 * @param column - column number of left upper corner in range from 0 to LED_X_SIZE
 * @retval void
 */
//void OLED_draw(uint8_t verse, uint8_t column, char * )

/**
 * @brief draw line on display (buffered)
 * @param x0 - start point x coordinate in range from 0 to OLED_X_SIZE - 1
 * @param y0 - start point y coordinate in range from 0 to OLED_Y_SIZE - 1
 * @param x1 - end point x coordinate in range from 0 to IOLED_X_SIZE - 1
 * @param y1 - end point y coordinate in range from 0 to OLED_Y_SIZE - 1
 * @retval void
 */
void OLED_drawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);


void OLED_setDisplayOn();

void OLED_setDisplayOff();

void OLED_setInversed(uint8_t tf);

/**
 * @brief draw image on display in specified position ( buffered )
 * @param xPos - x coordinate for upper left corner of image
 * @param yPos - y coordinate for upper left corner of image
 * @param image - image array where first byte is horizontal size,
 * second byte is vertical size and following ( horizontal size ) * ( vertical size) / 8
 * bytes describe the image ( each byte describe one 8-bit column )
 */
void OLED_drawImage(uint8_t xPos, uint8_t yPos, uint8_t image[]);

#endif
