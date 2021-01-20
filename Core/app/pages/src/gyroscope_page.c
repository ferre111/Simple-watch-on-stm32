/*
 * gyroscope_page.c
 *
 *  Created on: Jan 13, 2021
 *      Author: Karol Witusik
 */

#include "gyroscope_page.h"
#include "menu.h"

//----------------------------------------------------------------------

/* MACRO for formatting printed string with measured data */
#define PRINT_GYRO(gyro, axi) gyro >= 0 ? #axi ": %d.%.3ddeg/s" : #axi ":-%d.%.3ddeg/s"

//----------------------------------------------------------------------

/* variables to store objects IDs and char arrays containing printed strings */
static uint8_t title_id;
static char title_txt[20];

static uint8_t line_id;

static uint8_t gyro_data_id[3];
static char gyro_data_txt[3][20];

/* struct for reading data from sensor */
static struct MPU6050_gyro_data gyro_data;

//----------------------------------------------------------------------

void gyroscope_page_init(void)
{
    /* create all the drawable objects present on this page */
    OLED_createTextField(&title_id, 41, 0, title_txt, 2);
    OLED_createLine(&line_id, 0, 18, 127, 18);
    OLED_createTextField(&gyro_data_id[x_axis], 0, 21, gyro_data_txt[x_axis], 1);
    OLED_createTextField(&gyro_data_id[y_axis], 0, 29, gyro_data_txt[y_axis], 1);
    OLED_createTextField(&gyro_data_id[z_axis], 0, 37, gyro_data_txt[z_axis], 1);
    snprintf(title_txt, 20, "GYRO");
}

//----------------------------------------------------------------------

void gyroscope_page_draw(void)
{
    /* update displayed data with sensor readings */
    MPU6050_get_gyro_data(&gyro_data);
    snprintf(gyro_data_txt[x_axis], 20, PRINT_GYRO(gyro_data.x, x), abs(gyro_data.x) / 1000, abs(gyro_data.x) % 1000);
    snprintf(gyro_data_txt[y_axis], 20, PRINT_GYRO(gyro_data.y, y), abs(gyro_data.y) / 1000, abs(gyro_data.y) % 1000);
    snprintf(gyro_data_txt[z_axis], 20, PRINT_GYRO(gyro_data.z, z), abs(gyro_data.z) / 1000, abs(gyro_data.z) % 1000);
}

//----------------------------------------------------------------------

void gyroscope_page_exit(void)
{
    /* delete objects present on screen at this page */
    OLED_deleteObject(title_id);
    OLED_deleteObject(line_id);
    OLED_deleteObject(gyro_data_id[x_axis]);
    OLED_deleteObject(gyro_data_id[y_axis]);
    OLED_deleteObject(gyro_data_id[z_axis]);
}

