/*
 * RTC_page.c
 *
 *  Created on: Jan 13, 2021
 *      Author: Karol Witusik
 */

#include "RTC_page.h"

static RTC_TimeTypeDef time_s;
static RTC_DateTypeDef date_s;

static uint8_t timeTextField = 0;
static char timeText[8];

static uint8_t dateTextField = 0;
static char dateText[8];


void RTC_page_init(void)
{
    OLED_createTextField(&dateTextField, 38, 8, dateText, 1);
    OLED_createTextField(&timeTextField, 15, 24, timeText, 2);
}

void RTC_page_draw(void)
{
    HAL_RTC_GetTime(&hrtc, &time_s, RTC_FORMAT_BIN);
    sprintf(timeText, "%02d:%02d:%02d", time_s.Hours, time_s.Minutes, time_s.Seconds);

    HAL_RTC_GetDate(&hrtc, &date_s, RTC_FORMAT_BIN);
    sprintf(dateText, "%02d/%02d/%02d", date_s.Date, date_s.Month, date_s.Year);
}

void RTC_page_exit(void)
{
    OLED_deleteObject(dateTextField);
    OLED_deleteObject(timeTextField);
}
