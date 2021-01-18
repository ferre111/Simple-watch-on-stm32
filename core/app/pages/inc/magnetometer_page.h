/*
 * magnetometer_page.h
 *
 *  Created on: Jan 14, 2021
 *      Author: Karol Witusik
 */

#pragma once

#include "OLED.h"
#include "QMC5883L.h"
#include <stdio.h>
#include <stdlib.h>

void magnetometer_page_init(void);
void magnetometer_page_draw(void);
void magnetometer_page_exit(void);

