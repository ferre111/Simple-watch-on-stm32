/*
 * compass_page.c
 *
 *  Created on: Jan 13, 2021
 *      Author: Karol Witusik
 */

#include "compass_page.h"

static uint8_t line1_id;
static char line1_txt[20];

void compass_page_init(void)
{
    OLED_createTextField(&line1_id, 0, 0, line1_txt, 1);
}

void compass_page_draw(void)
{
    snprintf(line1_txt, 20, "compass page");
}

void compass_page_exit(void)
{
    OLED_deleteObject(line1_id);
}