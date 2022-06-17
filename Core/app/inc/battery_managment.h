/*
 * battery_managment.c
 *
 *  Created on: Jun 16, 2022
 *      Author: kwit
 */

#pragma once

enum battery_level {
	BATTERY_LEVEL_BELOW_25,
	BATTERY_LEVEL_ABOVE_25,
	BATTERY_LEVEL_ABOVE_50,
	BATTERY_LEVEL_ABOVE_75
};

uint32_t battery_managment_get_state(void);

float battery_managment_get_voltage(void);
