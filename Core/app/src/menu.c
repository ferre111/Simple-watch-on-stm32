/*
 * menu.c
 *
 *  Created on: Jan 13, 2021
 *      Author: Karol Witusik
 */
#include "menu.h"
#include "acceleration_page.h"
#include "compass_page.h"
#include "gyroscope_page.h"
#include "RTC_page.h"
#include "temperature_page.h"
#include "magnetometer_page.h"
#include "altitude_page.h"
#include "spirit_level_page.h"
#include "images.h"

#define BATERRY_ICON_X_POS 113
#define BATERRY_ICON_Y_POS 4

#define PADLOCK_ICON_X_POS 0
#define PADLOCK_ICON_Y_POS 2

//----------------------------------------------------------------------

/* Array of structure with pointer to functions associate with appropriate page*/
struct page page_tab[menu_pages_END] =
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

static struct menu_ctx ctx = {.page = menu_pages_START + 1, .page_tab = page_tab};

//----------------------------------------------------------------------

static void draw_current_page(void);
static void check_page_flag(void);

//----------------------------------------------------------------------

void menu_set_next_page_flag(void)
{
	if (ctx.enable_page_change)
	{
		ctx.next_page_flag = true;
	}
}

//----------------------------------------------------------------------

void menu_set_prev_page_flag(void)
{

	if (ctx.enable_page_change)
	{
		ctx.prev_page_flag = true;
	}
}

//----------------------------------------------------------------------

void menu_toggle_flag_enable_page_change(void)
{
	ctx.enable_page_change = !ctx.enable_page_change;

	OLED_deleteObject(ctx.padlock_image_id);
	if (ctx.enable_page_change) {
		OLED_createImage(&ctx.padlock_image_id, PADLOCK_ICON_X_POS, PADLOCK_ICON_Y_POS, padlock_open);
	} else {
		OLED_createImage(&ctx.padlock_image_id, PADLOCK_ICON_X_POS, PADLOCK_ICON_Y_POS, padlock_close);
	}
}

//----------------------------------------------------------------------

void menu_process_init(void)
{
    ctx.page_tab[ctx.page].init_fun();  //init first page
    button_set_callback_double_press_function(menu_toggle_flag_enable_page_change);
    ctx.previous_battery_level = BATTERY_LEVEL_BELOW_25;
    OLED_createImage(&ctx.battery_level_image_id, BATERRY_ICON_X_POS, BATERRY_ICON_Y_POS, battery_zero);

    ctx.enable_page_change = false;
    OLED_createImage(&ctx.padlock_image_id, PADLOCK_ICON_X_POS, PADLOCK_ICON_Y_POS, padlock_close);
}

//----------------------------------------------------------------------

void menu_process(void)
{
	enum battery_level current_level;
    draw_current_page();    //draw actual set page
    check_page_flag();
    current_level = battery_managment_get_state();

    if (current_level != ctx.previous_battery_level) {

    	OLED_deleteObject(ctx.battery_level_image_id);
    	switch(current_level) {
    		case BATTERY_LEVEL_BELOW_25:
    			OLED_createImage(&ctx.battery_level_image_id, BATERRY_ICON_X_POS, BATERRY_ICON_Y_POS, battery_zero);
    			break;
    		case BATTERY_LEVEL_ABOVE_25:
    			OLED_createImage(&ctx.battery_level_image_id, BATERRY_ICON_X_POS, BATERRY_ICON_Y_POS, battery_quarter);
				break;
    		case BATTERY_LEVEL_ABOVE_50:
    			OLED_createImage(&ctx.battery_level_image_id, BATERRY_ICON_X_POS, BATERRY_ICON_Y_POS, battery_half);
				break;
    		case BATTERY_LEVEL_ABOVE_75:
    			OLED_createImage(&ctx.battery_level_image_id, BATERRY_ICON_X_POS, BATERRY_ICON_Y_POS, battery_full);
				break;
    	}
    	ctx.previous_battery_level = current_level;
    }
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
        if((HAL_GetTick() - timestamp) > TIME_BETWEEN_PAGE_CHANGE)              //if next_page_flag is set and time between previous page change and current flag set is greater than TIME_BETWEEN_PAGE_CHANGE then change page to next page
        {
            ctx.page_tab[ctx.page].exit_fun();                                  //execute function to deinit page

            if(++ctx.page == menu_pages_END) ctx.page = menu_pages_START + 1;   //increment page
            timestamp = HAL_GetTick();                                          //get new timestamp

            ctx.page_tab[ctx.page].init_fun();                                  //execute function to init new page
        }

        ctx.next_page_flag = false;
    }

    if(ctx.prev_page_flag)
    {
        if((HAL_GetTick() - timestamp) > TIME_BETWEEN_PAGE_CHANGE)              //if prev_page_flag is set and time between previous page change and current flag set is greater than TIME_BETWEEN_PAGE_CHANGE then change page to previous page
        {
            ctx.page_tab[ctx.page].exit_fun();                                  //execute function to deinit page

            if(--ctx.page == menu_pages_START) ctx.page = menu_pages_END - 1;   //increment page
            timestamp = HAL_GetTick();                                          //get new timestamp

            ctx.page_tab[ctx.page].init_fun();                                  //execute function to init new page
        }

        ctx.prev_page_flag = false;
    }
}

