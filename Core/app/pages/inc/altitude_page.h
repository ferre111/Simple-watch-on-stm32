/*
 * altitude_page.h
 *
 *  Created on: Jan 15, 2021
 *      Author: Karol Witusik
 */

#pragma once

#include "OLED.h"
#include "pressure_sensor.h"
#include <stdio.h>

void altitude_page_init(void);
void altitude_page_draw(void);
void altitude_page_exit(void);

