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

// modify these according to the maximum count of objects used simultaneously to save RAM
#define OLED_MAX_TEXT_FIELDS_COUNT              10
#define OLED_MAX_LINES_COUNT                    10
#define OLED_MAX_RECTANGLES_COUNT               10
#define OLED_MAX_IMAGES_COUNT                   10

#define OLED_TRUE                                   1
#define OLED_FALSE                                  0


enum OLED_Color
{
    WHITE,
    BLACK
};


/* API FUNCTIONS */

/**
  * @brief initialize SSD1306 driver according to initSequence
  * @return void
  */
void OLED_Init();

/**
 *  @brief Update display with values from buffer.
 *  @return void
 */
void OLED_update();

/**
 *  @brief Clear entire screen
 *  @reval void
 */
void OLED_clearScreen();

void OLED_setDisplayOn();

void OLED_setDisplayOff();

void OLED_setInversed(uint8_t tf);



// === TEXT FIELD ===
/*
 * @brief create a text field
 * @param id - pointer to variable where textField id will be stored
 * @param x0 - x coordinate of left side of the text field
 * @param verse - verse of text (0 to OLED_NUM_OF_VERSES)
 * @retval void
 */
void OLED_createTextField(uint8_t * id, uint8_t x0, uint8_t verse, char * text);

/*
 * @brief set text of given textField
 * @param id - textField id
 * @param text - string
 * @retval none
 */
void OLED_textFieldSetText(uint8_t id, char * text);

/*
 * @brief set text field position
 * @param x - column number of left side of text
 * @param verse - verse of text field ( 0 to OLED_NUM_OF_VERSES - 1 )
 * @retval void
 */
void OLED_textFieldSetPosition(uint8_t id, uint8_t x, uint8_t verse);

/*
 * @brief delete text field
 * @param id - text field id
 */
//void OLED_textFieldDelete(uint8_t * id )

// === LINE ===
/*
 * @brief create a lines
 * @param id - pointer to variable where the line id will be stored
 * @param x0 - start point x coordinate
 * @param y0 - start point y coordinate
 * @param x1 - end point x coordinate
 * @param y1 - end point y coordinate
 */
void OLED_createLine(uint8_t * id, uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);

/*
 * @brief move start point of line
 * @param id - line id
 * @param x0 - x coordinate
 * @param y0 - y coordinate
 * @retval void
 */
void OLED_lineMoveStart(uint8_t id, uint8_t x0, uint8_t y0);

/*
 * @brief move end point of line
 * @param id - line id
 * @param x1 - x coordinate
 * @param y1 - y coordinate
 * @retval void
 */
void OLED_lineMoveEnd(uint8_t id, uint8_t x1, uint8_t y1);

// === RECTANGLE ===
/*
 * @brief create a rectangle
 * @param id - pointer to variable where the rectangle id will be stored
 * @param x0 - upper left corner x coordinate
 * @param y0 - upper left corner y coordinate
 * @param width - rectangle width
 * @param height - rectangle height
 * @param color - BLACK or WHITE
 * @retval void
 */
void OLED_createRectangle(uint8_t * id,  uint8_t x0, uint8_t y0, uint8_t width, uint8_t height);

/*
 * @brief set rectangle position
 * @param id - rectangle id
 * @param x0 - upper left corner x coordinate
 * @param y0 - upper left corner y coordinate
 */
void OLED_rectangleSetPosition(uint8_t id, uint8_t x0, uint8_t y0);

/*
 * @brief set rectangle dimensions
 * @oaram d - rectangle id
 * @param width
 * @param height
 * @retval void
 */
void OLED_rectangleSetDimensions(uint8_t id, uint8_t width, uint8_t height);

// === IMAGE ===

/*
 * @brief create an image
 * @param id - pointer to variable where the image id will be stored
 * @param x0 - upper left corner x coordinate
 * @param y0 - upper left corner y coordinate
 * @param imageArray - pointer to image representation saved in array
 *                      in which the first byte is image width and second byte is
 *                      image height.
 * @retval void
 */
void OLED_createImage(uint8_t * id, uint8_t x0, uint8_t y0, uint8_t * imageArray);

/*
 * @brief move image
 * @param id - iamge id
 * @param x0 - upper left corner x coordinate
 * @param y0 - upper left corner y coordinate
 * @retval void
 */
void OLED_imageMove(uint8_t id, uint8_t x0, uint8_t y0);
#endif
