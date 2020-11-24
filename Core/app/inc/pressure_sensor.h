/*
 * pressure_sensor.h
 *
 *  Created on: Nov 24, 2020
 *      Author: Karol Witusik
 */

#pragma once

#include <stdint.h>
#include "i2c.h"
#include "error_codes.h"

enum pressure_sensor_mode
{
    PRESSURE_SENSOR_ULTRA_LOW_POWER,
    PRESSURE_SENSOR_STANDARD,
    PRESSURE_SENSOR_HIGH_RESOLUTION,
    PRESSURE_SENSOR_ULTRA_HIGH_RESOLUTION
};

//----------------------------------------------------------------------

/*
 * @brief   Read calibration registers required to calculate temperature and pressure.
 * @return  Error code.
 */
uint32_t pressure_sensor_read_calib_data(void);

//----------------------------------------------------------------------

/*
 * @brief   Set oversampling parameter.
 * @param   sensor_mode - selected mode
 * @return  Error code.
 */
uint32_t pressure_sensor_set_sensor_mode(enum pressure_sensor_mode sensor_mode);

//----------------------------------------------------------------------

/*
 * @brief   Start reading procedure between sensor and uC.
 * @return  Error code.
 */
uint32_t pressure_sensor_read_temp_and_pres(void);

//----------------------------------------------------------------------

/*
 * @brief   Get temperature value in Celsius degrees divided by 10.
 * @param   temp - pointer to variable where value will be save
 * @return  Error code.
 */
uint32_t pressure_sensor_get_temp(int32_t *temp);

//----------------------------------------------------------------------

/*
 * @brief   Get pressure value in Pa.
 * @param   pres - pointer to variable where value will be save
 * @return  Error code.
 */
uint32_t pressure_sensor_get_pres(int32_t *pres);

/*
 * @brief   Calculate the difference in altitude.
 * @param   initial_pres - pressure against which altitude is calculated
 * @param   actual_pres - actual pressure
 * @return  Error code.
 */
uint32_t pressure_sensor_calc_dif_alt(int32_t initial_pres, int32_t actual_pres);

