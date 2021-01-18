/*
 * altitude_page.c
 *
 *  Created on: Jan 15, 2021
 *      Author: Karol Witusik
 */

#include "altitude_page.h"

#define AVR 1 //set count of sample to averaging initial pressure

static int32_t init_pres;
static int32_t actual_pres;
static float alt;

static uint8_t title_id;
static char title_txt[20];

static uint8_t line_id;

static uint8_t data_id;
static char data_txt[20];

void altitude_page_init(void)
{
    OLED_createTextField(&title_id, 47, 0, title_txt, 2);
    OLED_createLine(&line_id, 0, 18, 127, 18);
    OLED_createTextField(&data_id, 0, 33, data_txt, 3);
    snprintf(title_txt, 20, "ALT");

    //set initial pressure
    init_pres = 0;
    for(uint8_t i = 0; i < AVR; i++)
    {
        static int32_t tmp_init_pres;

        pressure_sensor_read_temp_and_pres();
        pressure_sensor_get_pres(&tmp_init_pres);

        init_pres += tmp_init_pres;
    }
    init_pres /= AVR;
}

void altitude_page_draw(void)
{
    pressure_sensor_read_temp_and_pres();
    pressure_sensor_get_pres(&actual_pres);
    pressure_sensor_calc_dif_alt(init_pres, actual_pres, &alt);
    snprintf(data_txt, 20, "%.1fm", alt);
}

void altitude_page_exit(void)
{
    OLED_deleteObject(title_id);
    OLED_deleteObject(line_id);
    OLED_deleteObject(data_id);
}
