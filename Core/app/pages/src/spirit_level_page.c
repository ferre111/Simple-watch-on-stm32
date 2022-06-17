/*
 * spirit_level_page.c
 *
 *  Created on: Jan 15, 2021
 *      Author: Karol Witusik
 */

#include <stdio.h>
#include "spirit_level_page.h"
#include "menu.h"
#include "images.h"

//--------------------------------------------------------------------------------

#define WIDTH_MACRO(width)   (width) > 0 ? ((width) > 29  ? 29 : (width)) : ((width) < -29  ? -29 : (width))
#define HEIGHT_MACRO(height) (height) > 0 ? ((height) > 29  ? 29 : (height)) : ((height) < -29  ? -29 : (height))
#define PRINT_DEG(axi, deg) deg >= 0 ? #axi "= %d'" : #axi "=%d'"
#define INITIAL_POINTER_COORD_X 61
#define INITIAL_POINTER_COORD_Y 29

//--------------------------------------------------------------------------------

static uint8_t level_data_x_id;
static char level_data_x_txt[20];
static int16_t level_data_x;

static uint8_t level_data_y_id;
static char level_data_y_txt[20];
static int16_t level_data_y;

static uint8_t spirit_level_image_id;
static uint8_t spirit_level_pointer_id;

//--------------------------------------------------------------------------------

static struct MPU6050_acc_data data;

//--------------------------------------------------------------------------------

void spirit_level_page_init(void)
{
    /* create all the drawable objects present on this page */
    OLED_createTextField(&level_data_x_id, 92, 47, level_data_x_txt, 1);
    OLED_createTextField(&level_data_y_id, 92, 55, level_data_y_txt, 1);

    OLED_createImage(&spirit_level_pointer_id, INITIAL_POINTER_COORD_X, INITIAL_POINTER_COORD_Y, spirit_level_pointer);
    OLED_createImage(&spirit_level_image_id, 0, 0, spirit_level_image);
}

//--------------------------------------------------------------------------------

void spirit_level_page_draw(void)
{
    /* read data from senor and evaluate position of device */
    MPU6050_get_acc_data(&data);
    level_data_x = data.x * 45 / 500;
    level_data_y = data.y * 45 / 500;

    /* update printed values of angles */
    snprintf(level_data_x_txt, 20, PRINT_DEG(x, level_data_x), level_data_x);
    snprintf(level_data_y_txt, 20, PRINT_DEG(y, level_data_y), level_data_y);

    /* update position of pointer on display */
    int16_t width =  -(level_data_x * 31) / 45 ;
    int16_t height = (level_data_y * 31) / 45 ;

    OLED_moveObject(spirit_level_pointer_id, INITIAL_POINTER_COORD_X - (WIDTH_MACRO(width)), INITIAL_POINTER_COORD_Y - (WIDTH_MACRO(height)));
}

//--------------------------------------------------------------------------------


void spirit_level_page_exit(void)
{
    /* delete objects present on screen at this page */
    OLED_deleteObject(spirit_level_image_id);
    OLED_deleteObject(spirit_level_pointer_id);
    OLED_deleteObject(level_data_x_id);
    OLED_deleteObject(level_data_y_id);
}
