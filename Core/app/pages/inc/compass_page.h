/*
 * compass_page.h
 *
 *  Created on: Jan 13, 2021
 *      Author: Karol Witusik
 */

#pragma once

#include "OLED.h"
#include "MPU6050.h"
#include <stdio.h>

void compass_page_init(void);
void compass_page_draw(void);
void compass_page_exit(void);
