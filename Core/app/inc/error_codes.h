/*
 * error_codes.h
 *
 *  Created on: Nov 24, 2020
 *      Author: Karol Witusik
 */

#pragma once

/*
 * @brief macro for error checking
 */
#define ERROR_OCCURRED(_err_code) (((_err_code) & 0x000000FF) != 0ul)

/*
 * @brief error masks
 */
#define ERR_PRESSURE_SENSOR_MASK (0x01 << 8)

/*
 * @brief pressure sensor errors
 */
#define ERR_WRONG_MODE 0x01

/*
 * @brief common errors
 */

#define ERR_NULL_POINTER 0xF1
