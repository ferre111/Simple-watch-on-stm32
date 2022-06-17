/*
 * spirit_level_page.h
 *
 *  Created on: Jan 15, 2021
 *      Author: Karol Witusik
 */

#pragma once

#include "OLED.h"
#include "MPU6050.h"

void spirit_level_page_init(void);
void spirit_level_page_draw(void);
void spirit_level_page_exit(void);
