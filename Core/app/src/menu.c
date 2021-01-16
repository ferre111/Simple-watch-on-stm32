/*
 * menu.c
 *
 *  Created on: Jan 13, 2021
 *      Author: Karol Witusik
 */
#include "menu.h"

//----------------------------------------------------------------------

struct page page_tab1[menu_pages_END] =
{
    [RTC_page]          = {.init_fun = RTC_page_init,           .draw_fun = RTC_page_draw,          .exit_fun = RTC_page_exit},
    [temperature_page]  = {.init_fun = temperature_page_init,   .draw_fun = temperature_page_draw,  .exit_fun = temperature_page_exit},
    [compass_page]      = {.init_fun = compass_page_init,       .draw_fun = compass_page_draw,      .exit_fun = compass_page_exit},
    [altitude_page]     = {.init_fun = altitude_page_init,      .draw_fun = altitude_page_draw,     .exit_fun = altitude_page_exit},
    [spirit_level_page] = {.init_fun = spirit_level_page_init,  .draw_fun = spirit_level_page_draw, .exit_fun = spirit_level_page_exit},
    [acceleration_page] = {.init_fun = acceleration_page_init,  .draw_fun = acceleration_page_draw, .exit_fun = acceleration_page_exit},
    [gyroscope_page]    = {.init_fun = gyroscope_page_init,     .draw_fun = gyroscope_page_draw,    .exit_fun = gyroscope_page_exit},
    [magnetometer_page] = {.init_fun = magnetometer_page_init,  .draw_fun = magnetometer_page_draw, .exit_fun = magnetometer_page_exit},
};

static struct menu_ctx ctx = {.page = menu_pages_START + 1, .page_tab = page_tab1};

//----------------------------------------------------------------------

static void draw_current_page(void);
static void check_page_flag(void);

//----------------------------------------------------------------------

void menu_set_next_page_flag(void)
{
    ctx.next_page_flag = true;
}

//----------------------------------------------------------------------

void menu_set_prev_page_flag(void)
{
    ctx.prev_page_flag = true;
}

//----------------------------------------------------------------------

void menu_process_init(void)
{
    ctx.page_tab[ctx.page].init_fun();
}

//----------------------------------------------------------------------

void menu_process(void)
{
    draw_current_page();
    check_page_flag();
}

//----------------------------------------------------------------------

static void draw_current_page(void)
{
    ctx.page_tab[ctx.page].draw_fun();
}

//----------------------------------------------------------------------

static void check_page_flag(void)
{
    static uint32_t timestamp;

    if(ctx.next_page_flag)
    {
        if((HAL_GetTick() - timestamp) > TIME_BETWEEN_PAGE_CHANGE)
        {
            ctx.page_tab[ctx.page].exit_fun();

            if(++ctx.page == menu_pages_END) ctx.page = menu_pages_START + 1;
            timestamp = HAL_GetTick();

            ctx.page_tab[ctx.page].init_fun();
        }

        ctx.next_page_flag = false;
    }

    if(ctx.prev_page_flag)
    {
        if((HAL_GetTick() - timestamp) > TIME_BETWEEN_PAGE_CHANGE)
        {
            ctx.page_tab[ctx.page].exit_fun();

            if(--ctx.page == menu_pages_START) ctx.page = menu_pages_END - 1;
            timestamp = HAL_GetTick();

            ctx.page_tab[ctx.page].init_fun();
        }

        ctx.prev_page_flag = false;
    }
}

