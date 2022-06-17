/*
 * compass_page.c
 *
 *  Created on: Jan 13, 2021
 *      Author: Wiktor Lechowicz
 */

#include "compass_page.h"
#include "images.h"
#include "QMC5883L.h"
#include <math.h>

//--------------------------------------------------------------------------------

#define PI                  3.141f
#define COMPASS_CENTRE_X    63
#define COMPASS_CENTRE_Y    38
#define NEEDLE_RADIUS       8
#define LETTERS_RADIUS      15
#define LETTERS_OFFSET      2
#define OFFSET_Y            32
#define OFFSET_Z            33
#define AVR_LEN             8

/* MACRO for formatting string with degree value */
#define PRINT_DEG(deg) deg >= 0 ? " %d'" : "%d'"

//--------------------------------------------------------------------------------

/* variables to store IDs of object on display and arrays to store displayed strings */
static uint8_t title_text_field;
static char title_text[7] = "COMPASS";
static uint8_t circle_image_field;

static uint8_t deg_text_field;
static char deg_text[5];

static uint8_t n_text_field, e_text_field, s_text_field, w_text_field;
static char n_text[1] = "N", e_text[1] = "E", s_text[1] = "S", w_text[1] = "W";

static uint8_t needle_line_field;

//--------------------------------------------------------------------------------

static int32_t mag_y, mag_z;                // variables to store values from sensor after averaging
static float theta = 0.0f;                  // needle angle relative to north

//* buffers for averaging read data */
static int16_t mag_y_buff[AVR_LEN];
static int16_t mag_z_buff[AVR_LEN];
static uint8_t p = 0;

/* struct for readign data from sensor */
static struct QMC5883L_mag_data data;

//--------------------------------------------------------------------------------

void compass_page_init(void)
{
    /* create all the drawable objects present on this page */
    OLED_createTextField(&title_text_field, 22, 0, title_text, 2);

    OLED_createImage(&circle_image_field, COMPASS_CENTRE_X - compassImage[0]/2, COMPASS_CENTRE_Y - compassImage[1]/2 + 2, compassImage);

    OLED_createTextField(&deg_text_field, 10, 16, deg_text, 1);

    OLED_createTextField(&n_text_field, 0, 0, n_text, 1);
    OLED_createTextField(&e_text_field, 0, 0, e_text, 1);
    OLED_createTextField(&s_text_field, 0, 0, s_text, 1);
    OLED_createTextField(&w_text_field, 0, 0, w_text, 1);

    OLED_createLine(&needle_line_field, COMPASS_CENTRE_X, COMPASS_CENTRE_Y - 10, COMPASS_CENTRE_X, COMPASS_CENTRE_Y + 10);
}

//--------------------------------------------------------------------------------

void compass_page_draw(void)
{
    /* read data from sensor and get average of last AVR readings */
    QMC5883L_get_mag_data(&data);
    mag_y_buff[p] = data.x - OFFSET_Y;
    mag_z_buff[p] = -data.y - OFFSET_Z;

    p = (p + 1) % AVR_LEN;
    mag_z = 0;
    mag_y = 0;
    for(uint8_t i = 0; i < AVR_LEN; i++){
        mag_z += mag_z_buff[i];
        mag_y += mag_y_buff[i];
    }

    mag_z = mag_z / AVR_LEN;
    mag_y = mag_y / AVR_LEN;

    /* calculate needle angle based on readings */
    if(mag_z == 0)
    {
        if(mag_y > 0)
            theta = PI/2.0f;
        else
            theta = -PI/2.0f;
    } else
    {
        theta = -atan2f((float)mag_y,(float)mag_z);             // clockwise angle is positive in this convention
    }

    snprintf(deg_text, 5, PRINT_DEG((int16_t)(-180.0f*theta/PI)), (int16_t)(-180.0f*theta/PI));

    /* update needle orientation */
    OLED_moveObject(needle_line_field, COMPASS_CENTRE_X - NEEDLE_RADIUS*sin(theta), COMPASS_CENTRE_Y - NEEDLE_RADIUS*cos(theta));
    OLED_lineMoveEnd(needle_line_field, COMPASS_CENTRE_X + NEEDLE_RADIUS*sin(theta), COMPASS_CENTRE_Y + NEEDLE_RADIUS*cos(theta));

    /* update position of N, E, S, W markings */
    OLED_moveObject(n_text_field, COMPASS_CENTRE_X - LETTERS_RADIUS*sinf(theta) - LETTERS_OFFSET,
            COMPASS_CENTRE_Y - LETTERS_RADIUS*cosf(theta) - LETTERS_OFFSET);
    OLED_moveObject(e_text_field, COMPASS_CENTRE_X - (LETTERS_RADIUS)*sinf(theta - PI/2.0f) - LETTERS_OFFSET,
            COMPASS_CENTRE_Y - LETTERS_RADIUS*cosf(theta - PI/2.0f) - LETTERS_OFFSET);
    OLED_moveObject(s_text_field, COMPASS_CENTRE_X - LETTERS_RADIUS*sinf(theta - PI) - LETTERS_OFFSET,
            COMPASS_CENTRE_Y - LETTERS_RADIUS*cosf(theta - PI) - LETTERS_OFFSET);
    OLED_moveObject(w_text_field, COMPASS_CENTRE_X - LETTERS_RADIUS*sinf(theta - 3.0f/2.0f*PI) - LETTERS_OFFSET,
            COMPASS_CENTRE_Y - LETTERS_RADIUS*cosf(theta - 3.0f/2.0f*PI) - LETTERS_OFFSET);
}

//--------------------------------------------------------------------------------

void compass_page_exit(void)
{
    /* delete objects present on screen at this page */
    OLED_deleteObject(title_text_field);
    OLED_deleteObject(circle_image_field);
    OLED_deleteObject(deg_text_field);

    OLED_deleteObject(n_text_field);
    OLED_deleteObject(e_text_field);
    OLED_deleteObject(s_text_field);
    OLED_deleteObject(w_text_field);

    OLED_deleteObject(needle_line_field);
}
