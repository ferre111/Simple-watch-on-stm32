/*
 * temperature_page.c
 *
 *  Created on: Jan 13, 2021
 *      Author: Karol Witusik
 */

#include "temperature_page.h"

#define PRINT_TEMP(temp) temp >= 0 ? "%d.%d'C" : "-%d.%d'C"

static uint32_t timestamp;
static bool     display_data_flag;                              // flag indicating if there is temperature or pressure on the display

/* variables to store IDs of drawable OLED objects and printed strings */
static uint8_t title_id;
static char title_txt[20];

static uint8_t line_id;

static uint8_t data_id;
static char data_txt[20];

static int32_t temp;                                            // temperature
static int32_t pres;                                            // pressure

void temperature_page_init(void)
{
    timestamp = HAL_GetTick();
    display_data_flag = false;

    /* create all the drawable objects present on this page */
    OLED_createTextField(&title_id, 36, 0, title_txt, 2);
    OLED_createLine(&line_id, 0, 18, 127, 18);
    OLED_createTextField(&data_id, 12, 33, data_txt, 3);
    snprintf(title_txt, 20, "METEO");
}

void temperature_page_draw(void)
{
    /* once CHANGING_DISP_DATA_TIME elapses, the display content is switched between temperature and pressure info */
    if((HAL_GetTick() - timestamp) > CHANGING_DISP_DATA_TIME)
    {
        timestamp = HAL_GetTick();
        display_data_flag ^= 1;

        OLED_deleteObject(data_id);

        if(display_data_flag)
        {
            OLED_createTextField(&data_id, 12, 33, data_txt, 3);
        }
        else
        {
            OLED_createTextField(&data_id, 12, 33, data_txt, 3);
        }
    }

    pressure_sensor_read_temp_and_pres();
    if(display_data_flag)
    {
        /* display pressure info */
        pressure_sensor_get_pres(&pres);
        snprintf(data_txt, 20, "%dhPa", (int16_t)(pres / 100));
    }
    else
    {
        /* display temperature info */
        pressure_sensor_get_temp(&temp);
        snprintf(data_txt, 20, PRINT_TEMP(temp), abs(temp) / 10, abs(temp) % 10);
    }
}

void temperature_page_exit(void)
{
    /* delete objects present on screen at this page */
    OLED_deleteObject(title_id);
    OLED_deleteObject(line_id);
    OLED_deleteObject(data_id);
}
