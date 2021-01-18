/*
 * RTC_page.c
 *
 *  Created on: Jan 13, 2021
 *      Author: Karol Witusik
 */

#include "RTC_page.h"

#define BLINK_PERIOD 500

#define YEAR_MIN_VAL    0
#define YEAR_MAX_VAL    99
#define MONTH_MIN_VAL   1
#define MONTH_MAX_VAL   12
#define DATE_MIN_VAL    1
#define DATE_MAX_VAL    31
#define HOURS_MIN_VAL   0
#define HOURS_MAX_VAL   23
#define MINUTES_MIN_VAL 0
#define MINUTES_MAX_VAL 59
#define SECONDS_MIN_VAL 0
#define SECONDS_MAX_VAL 59

enum RTC_page_data
{
    RTC_page_data_START = 0,
    Date,
    Month,
    Year,
    Hours,
    Minutes,
    Seconds,
    RTC_page_data_END
};

static RTC_TimeTypeDef time_s;
static RTC_DateTypeDef date_s;

static uint8_t timeTextField = 0;
static char timeText[20];

static uint8_t dateTextField = 0;
static char dateText[20];

static enum RTC_page_data set_time_date_counter = RTC_page_data_START;

static void set_time_date_fun(void);
static void increment_time_date_fun(void);


void RTC_page_init(void)
{
    OLED_createTextField(&dateTextField, 38, 8, dateText, 1);
    OLED_createTextField(&timeTextField, 15, 24, timeText, 2);
    button_set_callback_press_function(increment_time_date_fun);
    button_set_callback_hold_function(set_time_date_fun);
}

void RTC_page_draw(void)
{
    static uint32_t timestamp;
    static bool blink_flag;

    if(HAL_GetTick() - timestamp > BLINK_PERIOD)
    {
        timestamp = HAL_GetTick();
        blink_flag ^= 1;
    }

    switch(set_time_date_counter)
    {
    case Date:
        if(blink_flag)
        {
            snprintf(timeText, 20, "%02d:%02d:%02d", time_s.Hours, time_s.Minutes, time_s.Seconds);
            snprintf(dateText, 20, "  /%02d/%02d", date_s.Month, date_s.Year);
        }
        else
        {
            snprintf(timeText, 20, "%02d:%02d:%02d", time_s.Hours, time_s.Minutes, time_s.Seconds);
            snprintf(dateText, 20, "%02d/%02d/%02d", date_s.Date, date_s.Month, date_s.Year);
        }
        break;
    case Month:
        if(blink_flag)
        {
            snprintf(timeText, 20, "%02d:%02d:%02d", time_s.Hours, time_s.Minutes, time_s.Seconds);
            snprintf(dateText, 20, "%02d/  /%02d", date_s.Date, date_s.Year);
        }
        else
        {
            snprintf(timeText, 20, "%02d:%02d:%02d", time_s.Hours, time_s.Minutes, time_s.Seconds);
            snprintf(dateText, 20, "%02d/%02d/%02d", date_s.Date, date_s.Month, date_s.Year);
        }
        break;
    case Year:
        if(blink_flag)
        {
            snprintf(timeText, 20, "%02d:%02d:%02d", time_s.Hours, time_s.Minutes, time_s.Seconds);
            snprintf(dateText, 20, "%02d/%02d/  ", date_s.Date, date_s.Month);
        }
        else
        {
            snprintf(timeText, 20, "%02d:%02d:%02d", time_s.Hours, time_s.Minutes, time_s.Seconds);
            snprintf(dateText, 20, "%02d/%02d/%02d", date_s.Date, date_s.Month, date_s.Year);
        }
        break;
    case Hours:
        if(blink_flag)
        {
            snprintf(timeText, 20, "  :%02d:%02d", time_s.Minutes, time_s.Seconds);
            snprintf(dateText, 20, "%02d/%02d/%02d", date_s.Date, date_s.Month, date_s.Year);
        }
        else
        {
            snprintf(timeText, 20, "%02d:%02d:%02d", time_s.Hours, time_s.Minutes, time_s.Seconds);
            snprintf(dateText, 20, "%02d/%02d/%02d", date_s.Date, date_s.Month, date_s.Year);
        }
        break;
    case Minutes:
        if(blink_flag)
        {
            snprintf(timeText, 20, "%02d:  :%02d", time_s.Hours, time_s.Seconds);
            snprintf(dateText, 20, "%02d/%02d/%02d", date_s.Date, date_s.Month, date_s.Year);
        }
        else
        {
            snprintf(timeText, 20, "%02d:%02d:%02d", time_s.Hours, time_s.Minutes, time_s.Seconds);
            snprintf(dateText, 20, "%02d/%02d/%02d", date_s.Date, date_s.Month, date_s.Year);
        }
        break;
    case Seconds:
        if(blink_flag)
        {
            snprintf(timeText, 20, "%02d:%02d:  ", time_s.Hours, time_s.Minutes);
            snprintf(dateText, 20, "%02d/%02d/%02d", date_s.Date, date_s.Month, date_s.Year);
        }
        else
        {
            snprintf(timeText, 20, "%02d:%02d:%02d", time_s.Hours, time_s.Minutes, time_s.Seconds);
            snprintf(dateText, 20, "%02d/%02d/%02d", date_s.Date, date_s.Month, date_s.Year);
        }
        break;
    case RTC_page_data_START:
        HAL_RTC_GetTime(&hrtc, &time_s, RTC_FORMAT_BIN);
        HAL_RTC_GetDate(&hrtc, &date_s, RTC_FORMAT_BIN);
        snprintf(timeText, 20, "%02d:%02d:%02d", time_s.Hours, time_s.Minutes, time_s.Seconds);
        snprintf(dateText, 20, "%02d/%02d/%02d", date_s.Date, date_s.Month, date_s.Year);
        break;
    default:
        break;
    }
}

void RTC_page_exit(void)
{
    button_set_callback_hold_function(blank_fun);
    button_set_callback_press_function(blank_fun);
    OLED_deleteObject(dateTextField);
    OLED_deleteObject(timeTextField);
}

static void set_time_date_fun(void)
{
    if(++set_time_date_counter == RTC_page_data_END)
    {
        set_time_date_counter = RTC_page_data_START;
        HAL_RTC_SetDate(&hrtc, &date_s, RTC_FORMAT_BIN);
        HAL_RTC_SetTime(&hrtc, &time_s, RTC_FORMAT_BIN);
    }
}

static void increment_time_date_fun(void)
{
    switch(set_time_date_counter)
        {
        case Date:
            if(date_s.Date++ == DATE_MAX_VAL) date_s.Date = DATE_MIN_VAL;
            break;
        case Month:
            if(date_s.Month++ == MONTH_MAX_VAL) date_s.Month = MONTH_MIN_VAL;
            break;
        case Year:
            if(date_s.Year++ == YEAR_MAX_VAL) date_s.Year = YEAR_MIN_VAL;
            break;
        case Hours:
            if(time_s.Hours++ == HOURS_MAX_VAL) time_s.Hours = HOURS_MIN_VAL;
            break;
        case Minutes:
            if(time_s.Minutes++ == MINUTES_MAX_VAL) time_s.Minutes = MINUTES_MIN_VAL;
            break;
        case Seconds:
            if(time_s.Seconds++ == SECONDS_MAX_VAL) time_s.Seconds = SECONDS_MIN_VAL;
            break;
        default:
            break;
        }
}
