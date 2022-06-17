/*
 * battery_managment.c
 *
 *  Created on: Jun 16, 2022
 *      Author: kwit
 */

#include "adc.h"
#include "battery_managment.h"
#include "stdbool.h"

#define ADC_RESOULTION 4096U
#define VOL_DIV_COEF 3.0f
#define SUPPLY_VOL	3.3f
#define OFFSET 0.2f

#define BATTERY_CRITICAL_VOL 3.3f

static float battery_voltage;

uint32_t battery_managment_get_state(void) {
	if (battery_voltage > 3.95f) {
		return BATTERY_LEVEL_ABOVE_75;
	} else if (battery_voltage > 3.7f) {
		return BATTERY_LEVEL_ABOVE_50;
	} else if (battery_voltage > 3.45) {
		return BATTERY_LEVEL_ABOVE_25;
	} else {
		return BATTERY_LEVEL_BELOW_25;
	}
}

float battery_managment_get_voltage(void) {
	return battery_voltage;
}

void battery_managment_voltage_stabilizer_turn_on(void) {
	HAL_GPIO_WritePin(MCU_PWR_EN_GPIO_Port, MCU_PWR_EN_Pin, true);
}

void battery_managment_voltage_stabilizer_turn_off(void) {
	HAL_GPIO_WritePin(MCU_PWR_EN_GPIO_Port, MCU_PWR_EN_Pin, false);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
	battery_voltage = (float)HAL_ADC_GetValue(&hadc1) * VOL_DIV_COEF * SUPPLY_VOL / (float)(ADC_RESOULTION - 1) + OFFSET;
	if (battery_voltage <= BATTERY_CRITICAL_VOL) {
		battery_managment_voltage_stabilizer_turn_off();
	}
}
