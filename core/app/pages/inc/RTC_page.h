/*
 * RTC_page.h
 *
 *  Created on: Jan 13, 2021
 *      Author: Karol Witusik
 */

#pragma once

#include "OLED.h"
#include "rtc.h"
#include <stdio.h>

void RTC_page_init(void);
void RTC_page_draw(void);
void RTC_page_exit(void);
