/*
 * RTC_page.c
 *
 *  Created on: Jan 13, 2021
 *      Author: Karol Witusik
 */

#include "RTC_page.h"

#define BLINK_PERIOD 500

//#define DATE_MACRO(timestamp) (timestamp - HAL_GetTick()) > BLINK_PERIOD

enum RTC_page_data
{
    RTC_page_data_START,
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
static char timeText[8];

static uint8_t dateTextField = 0;
static char dateText[8];

static enum RTC_page_data set_time_data_counter;

static void set_time_data_fun(void);


void RTC_page_init(void)
{
    OLED_createTextField(&dateTextField, 38, 8, dateText, 1);
    OLED_createTextField(&timeTextField, 15, 24, timeText, 2);
    button_set_callback_hold_function(set_time_data_fun);
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

    switch(set_time_data_counter)
    {
    case Date:
        if(blink_flag)
        {
            snprintf(timeText, 20, "%02d:%02d:%02d", time_s.Hours, time_s.Minutes, time_s.Seconds);
            snprintf(dateText, 20, "/%02d/%02d", date_s.Month, date_s.Year);
        }
        else
        {
            snprintf(timeText, 20, "%02d:%02d:%02d", time_s.Hours, time_s.Minutes, time_s.Seconds);
            snprintf(dateText, 20, "%02d/%02d/%02d", date_s.Date, date_s.Month, date_s.Year);
        }
        break;
    case Month:
        break;
    case Year:
        break;
    case Hours:
        break;
    case Minutes:
        break;
    case Seconds:
        break;
    case RTC_page_data_START:
        HAL_RTC_GetTime(&hrtc, &time_s, RTC_FORMAT_BIN);
        HAL_RTC_GetDate(&hrtc, &date_s, RTC_FORMAT_BIN);
        snprintf(timeText, 20, "%02d:%02d:%02d", time_s.Hours, time_s.Minutes, time_s.Seconds);
        snprintf(dateText, 20, "%02d/%02d/%02d", date_s.Date, date_s.Month, date_s.Year);
        break;
    }


//    if(set_time_data_counter)
//    {
//        static uint32_t timestamp;
//        switch(set_time_data_counter)
//        case Date:
//        case
//        sprintf(timeText, "%02d:%02d:%02d", time_s.Hours, time_s.Minutes, time_s.Seconds);
//        sprintf(dateText, "%02d/%02d/%02d", date_s.Date, date_s.Month, date_s.Year);
//    }
//    else
//    {
//
//    }


}

void RTC_page_exit(void)
{
    button_set_callback_hold_function(blank_fun);
    OLED_deleteObject(dateTextField);
    OLED_deleteObject(timeTextField);
}

static void set_time_data_fun(void)
{
    if(++set_time_data_counter == RTC_page_data_END)
        set_time_data_counter = RTC_page_data_START;
}
