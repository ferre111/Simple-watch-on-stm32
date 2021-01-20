/*
 * menu.h
 *
 *  Created on: Jan 13, 2021
 *      Author: Karol Witusik
 */

#pragma once

#include <stdbool.h>
#include "main.h"

#include "acceleration_page.h"
#include "compass_page.h"
#include "gyroscope_page.h"
#include "RTC_page.h"
#include "temperature_page.h"
#include "magnetometer_page.h"
#include "altitude_page.h"
#include "spirit_level_page.h"

//----------------------------------------------------------------------

#define TIME_BETWEEN_PAGE_CHANGE    500 //in ms
#define GYRO_REG_VAL_TO_CHANGE_PAGE 15360

//----------------------------------------------------------------------

enum menu_axes
{
    x_axis,
    y_axis,
    z_axis
};

//----------------------------------------------------------------------

enum menu_pages
{
    menu_pages_START,

    RTC_page,
    temperature_page,
    compass_page,
    altitude_page,
    spirit_level_page,
    acceleration_page,
    gyroscope_page,
    magnetometer_page,

    menu_pages_END
};

//----------------------------------------------------------------------

struct page
{
    void (*init_fun)(void);
    void (*draw_fun)(void);
    void (*exit_fun)(void);
};

struct menu_ctx
{
    enum menu_pages page;
    bool next_page_flag;
    bool prev_page_flag;

    struct page *page_tab;
};

//----------------------------------------------------------------------

void menu_process_init(void);

//----------------------------------------------------------------------

void menu_process(void);

//----------------------------------------------------------------------

void menu_set_next_page_flag(void);

//----------------------------------------------------------------------

void menu_set_prev_page_flag(void);
