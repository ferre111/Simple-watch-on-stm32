/*
 * temperature_page.h
 *
 *  Created on: Jan 13, 2021
 *      Author: Karol Witusik
 */

#pragma once

#include "OLED.h"
#include "MPU6050.h"

void temperature_page_init(void);
void temperature_page_draw(void);
void temperature_page_exit(void);
