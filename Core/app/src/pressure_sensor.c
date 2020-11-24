/*
 * pressure_sensor.c
 *
 *  Created on: Nov 24, 2020
 *      Author: Karol Witusik
 */

#include "pressure_sensor.h"

#define ADDR 0xEE
#define COUNT_OF_CALIB_REG 11

#define I2C_HANDLE hi2c1

//----------------------------------------------------------------------

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

struct temp
{
    int32_t uncomp_temp;
    int32_t true_temp;
};

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

static uint32_t read_uncomp_temp(void);
static uint32_t read_uncomp_pres(void);
static void calc_true_temp(void);
static void calc_true_pres(void);

//----------------------------------------------------------------------

uint32_t pressure_sensor_read_calib_data(void)
{
    uint32_t err = ERR_PRESSURE_SENSOR_MASK;

    for(uint8_t i = 0; i < COUNT_OF_CALIB_REG; i++)
    {
        err = HAL_I2C_Mem_Read(&I2C_HANDLE, ADDR, 0xAA + i * 2, 1, aux_tab, 2, I2C_TIMEOUT);
        if(ERROR_OCCURRED(err)) return err;
        *(&ctx.calib_data.AC1 + i) = (aux_tab[0] << 8) + aux_tab[1];
    }
    return err;
}

//----------------------------------------------------------------------

uint32_t pressure_sensor_set_sensor_mode(enum pressure_sensor_mode sensor_mode)
{
    uint32_t err = ERR_PRESSURE_SENSOR_MASK;
    if(sensor_mode > 4)
    {
        err |= ERR_WRONG_MODE;
        return err;
    }
    ctx.mode = sensor_mode;

    return err;
}

//----------------------------------------------------------------------

uint32_t pressure_sensor_read_temp_and_pres(void)
{
    uint32_t err = ERR_PRESSURE_SENSOR_MASK;

    err = read_uncomp_temp();
    if(ERROR_OCCURRED(err)) return err;
    err = read_uncomp_pres();
    if(ERROR_OCCURRED(err)) return err;
    calc_true_temp();
    calc_true_pres();

    return err;
}

//----------------------------------------------------------------------

uint32_t pressure_sensor_get_temp(int32_t *temp)
{
    uint32_t err = ERR_PRESSURE_SENSOR_MASK;

    if(temp == NULL)
    {
        err |= ERR_NULL_POINTER;
        return err;
    }

    *temp = ctx.temp.true_temp;

    return err;
}

uint32_t pressure_sensor_get_pres(int32_t *pres)
{
    uint32_t err = ERR_PRESSURE_SENSOR_MASK;

    if(pres == NULL)
    {
        err |= ERR_NULL_POINTER;
        return err;
    }

    *pres = ctx.true_pres;

    return err;
}

//----------------------------------------------------------------------

uint32_t pressure_sensor_calc_dif_alt(int32_t initial_pres, int32_t actual_pres)
{
    //todo
    return 0;
}

//----------------------------------------------------------------------
// Static functions
//----------------------------------------------------------------------

static uint32_t read_uncomp_temp(void)
{
    uint32_t err = ERR_PRESSURE_SENSOR_MASK;

    aux_tab[0] = 0x2E;
    err = HAL_I2C_Mem_Write(&I2C_HANDLE, ADDR, 0xF4, 1, aux_tab, 1, I2C_TIMEOUT);
    if(ERROR_OCCURRED(err)) return err;
    HAL_Delay(5);

    err = HAL_I2C_Mem_Read(&I2C_HANDLE, ADDR, 0xF6, 1, aux_tab, 2, I2C_TIMEOUT);
    if(ERROR_OCCURRED(err)) return err;

    ctx.temp.uncomp_temp = (aux_tab[0] << 8) + aux_tab[1];

    return err;
}

//----------------------------------------------------------------------

static uint32_t read_uncomp_pres(void)
{
    uint32_t err = ERR_PRESSURE_SENSOR_MASK;

    aux_tab[0] = 0x34 + (ctx.mode << 6);
    err = HAL_I2C_Mem_Write(&I2C_HANDLE, ADDR, 0xF4, 1, aux_tab, 1, I2C_TIMEOUT);
    if(ERROR_OCCURRED(err)) return err;

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
        err |= ERR_WRONG_MODE;
        return err;
    }

    err = HAL_I2C_Mem_Read(&I2C_HANDLE, ADDR, 0xF6, 1, aux_tab, 3, I2C_TIMEOUT);
    if(ERROR_OCCURRED(err)) return err;

    ctx.uncomp_pres = ((aux_tab[0] << 16) + (aux_tab[1] << 8) + aux_tab[2]) >> (8 - ctx.mode);

    return err;
}

//----------------------------------------------------------------------

static void calc_true_temp(void)
{
    ctx.calc_data.X1 = (ctx.temp.uncomp_temp - ctx.calib_data.AC6) * ctx.calib_data.AC5 / 32768;
    ctx.calc_data.X2 = (ctx.calib_data.MC * 2048) / (ctx.calc_data.X1 + ctx.calib_data.MD);
    ctx.calc_data.B5 = ctx.calc_data.X1 + ctx.calc_data.X2;

    ctx.temp.true_temp = (ctx.calc_data.B5 + 8 ) / 16;
}

//----------------------------------------------------------------------

static void calc_true_pres(void)
{
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
