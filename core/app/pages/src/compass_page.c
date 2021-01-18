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

#define PI   3.141f
#define COMPASS_CENTRE_X    63
#define COMPASS_CENTRE_Y    38
#define NEEDLE_RADIUS       8
#define LETTERS_RADIUS      15
#define LETTERS_OFFSET      2
#define OFFSET_Y            32
#define OFFSET_Z            33
#define AVR_LEN             8

#define PRINT_DEG(deg) deg >= 0 ? " %0.0f'" : "%0.0f'"


static uint8_t title_text_field;
static char title_text[7] = "COMPASS";
static uint8_t circle_image_field;

static uint8_t deg_text_field;
static char deg_text[5];

static uint8_t n_text_field, e_text_field, s_text_field, w_text_field;
static char n_text[1] = "N", e_text[1] = "E", s_text[1] = "S", w_text[1] = "W";

static uint8_t needle_line_field;


int32_t mag_y, mag_z;
uint8_t needle_Nx = 0, needle_Ny = 0, needle_Sx = 0, needle_Sy = 0;
double theta = 0;

static int16_t mag_y_buff[AVR_LEN];
static int16_t mag_z_buff[AVR_LEN];
static uint8_t p = 0;

static struct QMC5883L_mag_data data;


void compass_page_init(void)
{
    OLED_createTextField(&title_text_field, 22, 0, title_text, 2);

    OLED_createImage(&circle_image_field, COMPASS_CENTRE_X - compassImage[0]/2, COMPASS_CENTRE_Y - compassImage[1]/2 + 2, compassImage);

    OLED_createTextField(&deg_text_field, 20, 16, deg_text, 1);

    OLED_createTextField(&n_text_field, 0, 0, n_text, 1);
    OLED_createTextField(&e_text_field, 0, 0, e_text, 1);
    OLED_createTextField(&s_text_field, 0, 0, s_text, 1);
    OLED_createTextField(&w_text_field, 0, 0, w_text, 1);

    OLED_createLine(&needle_line_field, COMPASS_CENTRE_X, COMPASS_CENTRE_Y - 10, COMPASS_CENTRE_X, COMPASS_CENTRE_Y + 10);
}

void compass_page_draw(void)
{
    QMC5883L_get_mag_data(&data);
    mag_y_buff[p] = data.y - OFFSET_Y;
    mag_z_buff[p] = data.z - OFFSET_Z;

    p = (p + 1) % AVR_LEN;
    mag_z = 0;
    mag_y = 0;
    for(uint8_t i = 0; i < AVR_LEN; i++){
        mag_z += mag_z_buff[i];
        mag_y += mag_y_buff[i];
    }

    mag_z = mag_z / AVR_LEN;
    mag_y = mag_y / AVR_LEN;

    if(mag_z == 0)
    {
        if(mag_y > 0)
            theta = PI/2.0f;
        else
            theta = -PI/2.0f;
    } else
    {
        theta = -atan2((double)mag_y,(double)mag_z);        // clockwise angle is positive in this convention
    }

    snprintf(deg_text, 5, PRINT_DEG(-180*theta/PI), -180*theta/PI);

    // update needle orientation
    OLED_moveObject(needle_line_field, COMPASS_CENTRE_X - NEEDLE_RADIUS*sin(theta), COMPASS_CENTRE_Y - NEEDLE_RADIUS*cos(theta));
    OLED_lineMoveEnd(needle_line_field, COMPASS_CENTRE_X + NEEDLE_RADIUS*sin(theta), COMPASS_CENTRE_Y + NEEDLE_RADIUS*cos(theta));

    OLED_moveObject(n_text_field, COMPASS_CENTRE_X - LETTERS_RADIUS*sin(theta) - LETTERS_OFFSET,
            COMPASS_CENTRE_Y - LETTERS_RADIUS*cos(theta) - LETTERS_OFFSET);
    OLED_moveObject(e_text_field, COMPASS_CENTRE_X - (LETTERS_RADIUS)*sin(theta - PI/2.0f) - LETTERS_OFFSET,
            COMPASS_CENTRE_Y - LETTERS_RADIUS*cos(theta - PI/2.0f) - LETTERS_OFFSET);
    OLED_moveObject(s_text_field, COMPASS_CENTRE_X - LETTERS_RADIUS*sin(theta - PI) - LETTERS_OFFSET,
            COMPASS_CENTRE_Y - LETTERS_RADIUS*cos(theta - PI) - LETTERS_OFFSET);
    OLED_moveObject(w_text_field, COMPASS_CENTRE_X - LETTERS_RADIUS*sin(theta - 3.0f/2.0f*PI) - LETTERS_OFFSET,
            COMPASS_CENTRE_Y - LETTERS_RADIUS*cos(theta - 3.0f/2.0f*PI) - LETTERS_OFFSET);
}

void compass_page_exit(void)
{
    OLED_deleteObject(title_text_field);
    OLED_deleteObject(circle_image_field);
    OLED_deleteObject(deg_text_field);

    OLED_deleteObject(n_text_field);
    OLED_deleteObject(e_text_field);
    OLED_deleteObject(s_text_field);
    OLED_deleteObject(w_text_field);

    OLED_deleteObject(needle_line_field);
}
