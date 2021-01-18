/*
 * temperature_page.h
 *
 *  Created on: Jan 13, 2021
 *      Author: Karol Witusik
 */

#pragma once

#include "OLED.h"
#include "pressure_sensor.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define CHANGING_DISP_DATA_TIME 5000    //in ms

void temperature_page_init(void);
void temperature_page_draw(void);
void temperature_page_exit(void);
