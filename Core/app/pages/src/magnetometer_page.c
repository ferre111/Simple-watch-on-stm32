/*
 * magnetometer_page.c
 *
 *  Created on: Jan 14, 2021
 *      Author: Karol Witusik
 */

#include "magnetometer_page.h"
#include "menu.h"

//----------------------------------------------------------------------

#define PRINT_MAG(mag, axi) mag >= 0  ? #axi ": %d.%.3dG" : #axi ": -%d.%.3dG"

//----------------------------------------------------------------------

static uint8_t title_id;
static char title_txt[20];

static uint8_t line_id;

static uint8_t mag_data_id[3];
static char mag_data_txt[3][20];

static struct QMC5883L_mag_data mag_data;

//----------------------------------------------------------------------

void magnetometer_page_init(void)
{
    OLED_createTextField(&title_id, 47, 0, title_txt, 2);
    OLED_createLine(&line_id, 0, 18, 127, 18);
    OLED_createTextField(&mag_data_id[x_axis], 0, 21, mag_data_txt[x_axis], 1);
    OLED_createTextField(&mag_data_id[y_axis], 0, 29, mag_data_txt[y_axis], 1);
    OLED_createTextField(&mag_data_id[z_axis], 0, 37, mag_data_txt[z_axis], 1);
    snprintf(title_txt, 20, "MAG");
}

//----------------------------------------------------------------------

void magnetometer_page_draw(void)
{
    QMC5883L_get_mag_data(&mag_data);
    snprintf(mag_data_txt[x_axis], 20, PRINT_MAG(mag_data.x, x), abs(mag_data.x) / 1000, abs(mag_data.x) % 1000);
    snprintf(mag_data_txt[y_axis], 20, PRINT_MAG(mag_data.y, y), abs(mag_data.y) / 1000, abs(mag_data.y) % 1000);
    snprintf(mag_data_txt[z_axis], 20, PRINT_MAG(mag_data.z, z), abs(mag_data.z) / 1000, abs(mag_data.z) % 1000);
}

//----------------------------------------------------------------------

void magnetometer_page_exit(void)
{
    OLED_deleteObject(title_id);
    OLED_deleteObject(line_id);
    OLED_deleteObject(mag_data_id[x_axis]);
    OLED_deleteObject(mag_data_id[y_axis]);
    OLED_deleteObject(mag_data_id[z_axis]);
}
