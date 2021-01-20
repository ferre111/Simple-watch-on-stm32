/*
 * acceleration_page.c
 *
 *  Created on: Jan 13, 2021
 *      Author: Karol Witusik
 */

#include "acceleration_page.h"
#include "menu.h"

//----------------------------------------------------------------------

#define PRINT_ACC(acc, axi) acc >= 0  ? #axi ": %d.%.3dg" : #axi ":-%d.%.3dg"

//----------------------------------------------------------------------

static uint8_t title_id;
static char title_txt[20];

static uint8_t line_id;

static uint8_t acc_data_id[3];
static char acc_data_txt[3][20];

static struct MPU6050_acc_data acc_data;

//----------------------------------------------------------------------

void acceleration_page_init(void)
{
    OLED_createTextField(&title_id, 47, 0, title_txt, 2);
    OLED_createLine(&line_id, 0, 18, 127, 18);
    OLED_createTextField(&acc_data_id[x_axis], 0, 21, acc_data_txt[x_axis], 1);
    OLED_createTextField(&acc_data_id[y_axis], 0, 29, acc_data_txt[y_axis], 1);
    OLED_createTextField(&acc_data_id[z_axis], 0, 37, acc_data_txt[z_axis], 1);
    snprintf(title_txt, 20, "ACC");
}

//----------------------------------------------------------------------

void acceleration_page_draw(void)
{
    MPU6050_get_acc_data(&acc_data);
    snprintf(acc_data_txt[x_axis], 20, PRINT_ACC(acc_data.x, x), abs(acc_data.x) / 1000, abs(acc_data.x) % 1000);
    snprintf(acc_data_txt[y_axis], 20, PRINT_ACC(acc_data.y, y), abs(acc_data.y) / 1000, abs(acc_data.y) % 1000);
    snprintf(acc_data_txt[z_axis], 20, PRINT_ACC(acc_data.z, z), abs(acc_data.z) / 1000, abs(acc_data.z) % 1000);
}

//----------------------------------------------------------------------

void acceleration_page_exit(void)
{
    OLED_deleteObject(title_id);
    OLED_deleteObject(line_id);
    OLED_deleteObject(acc_data_id[x_axis]);
    OLED_deleteObject(acc_data_id[y_axis]);
    OLED_deleteObject(acc_data_id[z_axis]);
}

