/*
 * pressure_sensor.c
 *
 *  Created on: Nov 24, 2020
 *      Author: Karol Witusik
 */

#include "pressure_sensor.h"

#define ADDR PRESSUURE_SENSOR_ADDR
#define COUNT_OF_CALIB_REG 11

#define I2C_HANDLE hi2c2

#define OUT_XLSB    0xF8
#define OUT_LSB     0xF7
#define OUT_MSB     0xF6
#define CTRL_MEAS   0xF4
#define SOFT_RESET  0xE0
#define ID          0xD0
#define CALIB0      0xAA

#define PRESS_I2C_TIMEOUT 100

//----------------------------------------------------------------------

/** @brief      Structure for calibration data.*/
struct calib_data
{
    int16_t AC1;
    int16_t AC2;
    int16_t AC3;
    uint16_t AC4;
    uint16_t AC5;
    uint16_t AC6;
    int16_t B1;
    int16_t B2;
    int16_t MB;
    int16_t MC;
    int16_t MD;
};

//----------------------------------------------------------------------

/** @brief      Structure for calculation.*/
struct calc_data
{
    int32_t     X1;
    int32_t     X2;
    int32_t     X3;
    int32_t     B3;
    uint32_t    B4;
    int32_t     B5;
    int32_t     B6;
    uint32_t    B7;
};

//----------------------------------------------------------------------

/** @brief      Structure for temperature after reading from sensor and after calculation.*/
struct temp
{
    int32_t uncomp_temp;
    int32_t true_temp;
};

//----------------------------------------------------------------------

/** @brief      Main structure.*/
struct pressure_sensor
{
    struct calib_data                   calib_data;
    struct calc_data                    calc_data;
    struct temp                         temp;
    int32_t                             uncomp_pres;
    int32_t                             true_pres;

    enum pressure_sensor_mode    mode;
};

//----------------------------------------------------------------------

static struct pressure_sensor ctx;

static uint8_t aux_tab[3];

//----------------------------------------------------------------------

static void read_uncomp_temp(void);
static void read_uncomp_pres(void);
static void calc_true_temp(void);
static void calc_true_pres(void);

//----------------------------------------------------------------------

void pressure_sensor_read_calib_data(void)
{
    /*Reading of calibration data*/
    for(uint8_t i = 0; i < COUNT_OF_CALIB_REG; i++)
    {
    	HAL_I2C_Mem_Read(&I2C_HANDLE, ADDR, CALIB0 + i * 2, 1, aux_tab, 2, PRESS_I2C_TIMEOUT);
//        myI2C_readByteStream(I2C_HANDLE, ADDR, CALIB0 + i * 2, aux_tab, 2);
        *(&ctx.calib_data.AC1 + i) = (aux_tab[0] << 8) + aux_tab[1];
    }
}

//----------------------------------------------------------------------

void pressure_sensor_set_sensor_mode(enum pressure_sensor_mode sensor_mode)
{
    ctx.mode = sensor_mode;
}

//----------------------------------------------------------------------

void pressure_sensor_read_temp_and_pres(void)
{
    read_uncomp_temp();
    read_uncomp_pres();
    calc_true_temp();
    calc_true_pres();
}

//----------------------------------------------------------------------

void pressure_sensor_get_temp(int32_t *temp)
{
    *temp = ctx.temp.true_temp;
}

//----------------------------------------------------------------------

void pressure_sensor_get_pres(int32_t *pres)
{
    *pres = ctx.true_pres;
}

//----------------------------------------------------------------------

void pressure_sensor_calc_dif_alt(int32_t initial_pres, int32_t actual_pres, float *alt)
{
    /*Formula from reference manual of BMP180 sensor.*/
    *alt = 44330 * (1 - pow((float)actual_pres/(float)initial_pres, 0.190294f));
}

//----------------------------------------------------------------------
// Static functions
//----------------------------------------------------------------------

static void read_uncomp_temp(void)
{
    /*Write proper value to measurement control register*/
    aux_tab[0] = 0x2E;
    HAL_I2C_Mem_Write(&I2C_HANDLE, ADDR, CTRL_MEAS, 1, aux_tab, 1, PRESS_I2C_TIMEOUT);
//    myI2C_writeByteStream(I2C_HANDLE, ADDR, CTRL_MEAS, aux_tab, 1);
    HAL_Delay(5);

    HAL_I2C_Mem_Read(&I2C_HANDLE, ADDR, OUT_MSB, 1, aux_tab, 2, PRESS_I2C_TIMEOUT);
//    myI2C_readByteStream(I2C_HANDLE, ADDR, OUT_MSB, aux_tab, 2);

    ctx.temp.uncomp_temp = (aux_tab[0] << 8) + aux_tab[1];
}

//----------------------------------------------------------------------

static void read_uncomp_pres(void)
{
    /*Write proper value to measurement control register*/
    aux_tab[0] = 0x34 + (ctx.mode << 6);
    HAL_I2C_Mem_Write(&I2C_HANDLE, ADDR, CTRL_MEAS, 1, aux_tab, 1, PRESS_I2C_TIMEOUT);
//    myI2C_writeByteStream(I2C_HANDLE, ADDR, CTRL_MEAS, aux_tab, 1);

    /*Choose correct delay.*/
    switch(ctx.mode)
    {
    case PRESSURE_SENSOR_ULTRA_LOW_POWER:
        HAL_Delay(5);
        break;
    case PRESSURE_SENSOR_STANDARD:
        HAL_Delay(8);
        break;
    case PRESSURE_SENSOR_HIGH_RESOLUTION:
        HAL_Delay(14);
        break;
    case PRESSURE_SENSOR_ULTRA_HIGH_RESOLUTION:
        HAL_Delay(26);
        break;
    default:
        break;
    }

    HAL_I2C_Mem_Read(&I2C_HANDLE, ADDR, OUT_MSB, 1, aux_tab, 3, PRESS_I2C_TIMEOUT);
//    myI2C_readByteStream(I2C_HANDLE, ADDR, OUT_MSB, aux_tab, 3);

    ctx.uncomp_pres = ((aux_tab[0] << 16) + (aux_tab[1] << 8) + aux_tab[2]) >> (8 - ctx.mode);
}

//----------------------------------------------------------------------

static void calc_true_temp(void)
{
    /*Formula from reference manual of BMP180 sensor.*/
    ctx.calc_data.X1 = (ctx.temp.uncomp_temp - ctx.calib_data.AC6) * ctx.calib_data.AC5 / 32768;
    ctx.calc_data.X2 = (ctx.calib_data.MC * 2048) / (ctx.calc_data.X1 + ctx.calib_data.MD);
    ctx.calc_data.B5 = ctx.calc_data.X1 + ctx.calc_data.X2;

    ctx.temp.true_temp = (ctx.calc_data.B5 + 8 ) / 16;
}

//----------------------------------------------------------------------

static void calc_true_pres(void)
{
    /*Formula from reference manual of BMP180 sensor.*/
    ctx.calc_data.B6 = ctx.calc_data.B5 - 4000;
    ctx.calc_data.X1 = (ctx.calib_data.B2 * (ctx.calc_data.B6 * ctx.calc_data.B6 / 4096)) / 2048;
    ctx.calc_data.X2 = ctx.calib_data.AC2 * ctx.calc_data.B6 / 2048;
    ctx.calc_data.X3 = ctx.calc_data.X1 + ctx.calc_data.X2;
    ctx.calc_data.B3 = (((ctx.calib_data.AC1 * 4 + ctx.calc_data.X3) << ctx.mode) + 2) / 4;
    ctx.calc_data.X1 = ctx.calib_data.AC3 * ctx.calc_data.B6 / 8192;
    ctx.calc_data.X2 = (ctx.calib_data.B1 * (ctx.calc_data.B6 * ctx.calc_data.B6 / 4096)) / 65536;
    ctx.calc_data.X3 = ((ctx.calc_data.X1 + ctx.calc_data.X2) + 2) / 4;
    ctx.calc_data.B4 = ctx.calib_data.AC4 * (uint32_t)(ctx.calc_data.X3 + 32768) / 32768;
    ctx.calc_data.B7 = ((uint32_t)ctx.uncomp_pres - ctx.calc_data.B3) * (50000 >> ctx.mode);

    if(ctx.calc_data.B7 < 0x80000000)
    {
        ctx.true_pres = (ctx.calc_data.B7 * 2) / ctx.calc_data.B4;
    }
    else
    {
        ctx.true_pres = (ctx.calc_data.B7 / ctx.calc_data.B4) * 2;
    }

    ctx.calc_data.X1 = (ctx.true_pres / 256) * (ctx.true_pres / 256);
    ctx.calc_data.X1 = (ctx.calc_data.X1 * 3038) / 65536;
    ctx.calc_data.X2 = (-7357 * ctx.true_pres) / 65536;

    ctx.true_pres = ctx.true_pres + (ctx.calc_data.X1 + ctx.calc_data.X2 + 3791) / 16;
}
