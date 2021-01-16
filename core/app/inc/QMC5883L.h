/*
 * QMC5883L.h
 *
 *  Created on: Jan 10, 2021
 *      Author: Karol Witusik
 */

#pragma once

#define QMC588L_ADDR        0x1A //00011010

#define QMC588L_MODE_REG_1    0x09
#define QMC588L_MODE_REG_2    0x0A
#define QMC588L_XOUT_L      0x00
#define QMC588L_FBR         0x0B

#define PRINT_MAG(mag, axi) mag >= 0  ? "Mag " #axi ": %d.%.3dG" : "Mag " #axi ": -%d.%.3dG"

//----------------------------------------------------------------------

enum QMC5883L_mode
{
    QMC5883L_MODE_STANDBY,
    QMC5883L_MODE_CONTINUOUS
};

enum QMC5883L_output_data_rate
{
    QMC5883L_ODR_10,
    QMC5883L_ODR_50,
    QMC5883L_ODR_100,
    QMC5883L_ODR_200
};

enum QMC5883L_full_scale
{
    QMC5883L_RNG_2G,
    QMC5883L_RNG_8G
};

enum QMC5883L_over_sample_ratio
{
    QMC5883L_OSR_512,
    QMC5883L_OSR_256,
    QMC5883L_OSR_128,
    QMC5883L_OSR_64
};

//----------------------------------------------------------------------

struct QMC5883L_ctx
{
    enum QMC5883L_mode                  mode;
    enum QMC5883L_output_data_rate      output_data_rate;
    enum QMC5883L_full_scale            full_scale;
    enum QMC5883L_over_sample_ratio     over_sample_ratio;
};

//----------------------------------------------------------------------

/*
 * @brief   Get magnetic field strength on the x axis.
 * @param   mag_x - pointer to variable where value will be save, value units: mG
 */
void QMC5883L_get_mag_x(int16_t *mag_x);

//----------------------------------------------------------------------

/*
 * @brief   Get magnetic field strength on the y axis.
 * @param   mag_y - pointer to variable where value will be save, value units: mG
 */
void QMC5883L_get_mag_y(int16_t *mag_y);

//----------------------------------------------------------------------

/*
 * @brief   Get magnetic field strength on the z axis.
 * @param   mag_z - pointer to variable where value will be save, value units: mG
 */
void QMC5883L_get_mag_z(int16_t *mag_z);
