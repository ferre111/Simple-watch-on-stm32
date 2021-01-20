/*
 * gyroscope_page.h
 *
 *  Created on: Jan 13, 2021
 *      Author: Karol Witusik
 */

#pragma once

#include "OLED.h"
#include "MPU6050.h"

void gyroscope_page_init(void);
void gyroscope_page_draw(void);
void gyroscope_page_exit(void);
