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

struct QMC5883L_mag_data
{
    int16_t x;
    int16_t y;
    int16_t z;
};

//----------------------------------------------------------------------

/*
 * @brief   Get magnetic field strength on the x axis.
 * @param   mag_x - pointer to structure where values will be save, value units: mG
 */
void QMC5883L_get_mag_data(struct QMC5883L_mag_data *mag_data);
