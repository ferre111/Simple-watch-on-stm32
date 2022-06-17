/*
 * menu.h
 *
 *  Created on: Jan 13, 2021
 *      Author: Karol Witusik
 */

#pragma once

#include <stdbool.h>
#include "main.h"
#include "battery_managment.h"

//----------------------------------------------------------------------

#define TIME_BETWEEN_PAGE_CHANGE    500 //in ms
//#define GYRO_REG_VAL_TO_CHANGE_PAGE 15360
#define GYRO_REG_VAL_TO_CHANGE_PAGE 6000

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
    bool enable_page_change;
    enum battery_level previous_battery_level;

    uint8_t battery_level_image_id;
    uint8_t padlock_image_id;

    struct page *page_tab;
};

//----------------------------------------------------------------------

extern struct MPU6050_ctx MPU6050;

//----------------------------------------------------------------------

void menu_process_init(void);

//----------------------------------------------------------------------

/*
 * @brief   This function should be insert in main loop.
 */
void menu_process(void);

//----------------------------------------------------------------------

void menu_toggle_flag_enable_page_change(void);

//----------------------------------------------------------------------

void menu_set_next_page_flag(void);

//----------------------------------------------------------------------

void menu_set_prev_page_flag(void);

