#pragma once

#include <stdlib.h>
#include "i2c.h"
#include "stdbool.h"
#include "main.h"
#include "QMC5883L.h"

//----------------------------------------------------------------------

enum MPU6050_dlpf_acc_bandwidth
{
    MPU6050_BANDWIDTH_260,
    MPU6050_BANDWIDTH_184,
    MPU6050_BANDWIDTH_94,
    MPU6050_BANDWIDTH_44,
    MPU6050_BANDWIDTH_21,
    MPU6050_BANDWIDTH_10,
    MPU6050_BANDWIDTH_5
};

enum MPU6050_gyro_or_acc
{
    MPU6050_GYRO,
    MPU6050_ACC
};

enum MPU6050_selftest
{
    MPU6050_Z_SELFTEST = 32,
    MPU6050_Y_SELFTEST = 64,
    MPU6050_X_SELFTEST = 128
};

enum MPU6050_gyro_full_scale_range
{
    MPU6050_GYRO_FULL_SCALE_250 = 0,
    MPU6050_GYRO_FULL_SCALE_500 = 8,
    MPU6050_GYRO_FULL_SCALE_1000 = 16,
    MPU6050_GYRO_FULL_SCALE_2000 = 24
};

enum MPU6050_acc_full_scale_range
{
    MPU6050_ACC_FULL_SCALE_2 = 0,
    MPU6050_ACC_FULL_SCALE_4 = 8,
    MPU6050_ACC_FULL_SCALE_8 = 16,
    MPU6050_ACC_FULL_SCALE_16 = 24,
};

enum MPU6050_FIFO_data_en
{
    MPU6050_SLV0_FIFO_EN = 1,
    MPU6050_SLV1_FIFO_EN = 2,
    MPU6050_SLV2_FIFO_EN = 4,
    MPU6050_ACCEL_FIFO_EN = 8,
    MPU6050_ZG_FIFO_EN = 16,
    MPU6050_YG_FIFO_EN = 32,
    MPU6050_XG_FIFO_EN = 64,
    MPU6050_TEMP_FIFO_EN = 128,
};

enum MPU6050_I2C_master_clock_speed
{
    MPU6050_I2C_CLOCK_SPEED_348,
    MPU6050_I2C_CLOCK_SPEED_333,
    MPU6050_I2C_CLOCK_SPEED_320,
    MPU6050_I2C_CLOCK_SPEED_308,
    MPU6050_I2C_CLOCK_SPEED_296,
    MPU6050_I2C_CLOCK_SPEED_286,
    MPU6050_I2C_CLOCK_SPEED_276,
    MPU6050_I2C_CLOCK_SPEED_267,
    MPU6050_I2C_CLOCK_SPEED_258,
    MPU6050_I2C_CLOCK_SPEED_500,
    MPU6050_I2C_CLOCK_SPEED_471,
    MPU6050_I2C_CLOCK_SPEED_444,
    MPU6050_I2C_CLOCK_SPEED_421,
    MPU6050_I2C_CLOCK_SPEED_400,
    MPU6050_I2C_CLOCK_SPEED_381,
    MPU6050_I2C_CLOCK_SPEED_364
};

enum MPU6050_clock_select
{
    MPU6050_INTERNAL_OSC,
    MPU6050_PLL_X_GYRO,
    MPU6050_PLL_Y_GYRO,
    MPU6050_PLL_Z_GYRO,
    MPU6050_PLL_EXTERNAL_32K,
    MPU6050_PLL_EXTERNAL_19M,
    MPU6050_PLL_STOP_CLK = 7,
};

enum MPU6050_i2c_delay_select_slave
{
    MPU6050_DELAY_SLV0 = 1,
    MPU6050_DELAY_SLV1 = 2,
    MPU6050_DELAY_SLV2 = 4,
    MPU6050_DELAY_SLV3 = 8,
    MPU6050_DELAY_SLV4 = 16,
};

enum MPUT6050_interrupt_en
{
    MPU6050_INT_DATA_RDY_EN = 1,
    MPU6050_INT_MST_EN = 8,
    MPU6050_INT_FIFO_OFLOW_EN = 16,
};

struct MPUT6050_I2C_master
{
    enum MPU6050_I2C_master_clock_speed master_clock_speed;
    bool                                mult_mst_en;
    bool                                wait_for_es;
    bool                                mst_p_nsr;
    bool                                slave3_fifo_en;
    uint8_t                             slave_delay;    //max val 31, slave sample rate is sample_rate/(1 + I2C_slave_delay)
    uint8_t                             slave_delay_mask;   //choose the slaves that will have reduce samole_rate

};

struct MPU6050_I2C_slave
{
    uint8_t addr;
    uint8_t reg_addr;
    uint8_t len; //15 max
    bool    RW;
    bool    byte_swap;
    bool    reg_dis;
    bool    group;
    bool    en;
};

struct int_pin
{
    bool    level;
    bool    open;
    bool    latch_en;
    bool    rd_clear;
};

struct MPU6050_ctx
{
    enum MPU6050_dlpf_acc_bandwidth     dlpf_acc_bandwidth;
    uint8_t                             sample_rate_div;
    enum MPU6050_gyro_full_scale_range  gyro_full_scale_range;
    enum MPU6050_acc_full_scale_range   acc_full_scale_range;
    uint8_t                             fifo_data_enable_mask;

    struct MPUT6050_I2C_master master;
    struct MPU6050_I2C_slave slave[4];

    struct int_pin                      int_pin;
    bool                                fsync_int_level;
    bool                                fsync_int_en;
    bool                                i2c_bypass_en;

    uint8_t                             interrupt_en_mask;

    bool                                fifo_en;
    bool                                i2c_mst_en;

    enum MPU6050_clock_select           clock_select;

    uint16_t                            acc_sensitivity;
    uint16_t                            gyro_sensitivity;
    uint16_t                            mag_sensitivity;

    struct QMC5883L_ctx                 QMC5883L_ctx;
};

void MPU6050_init(struct MPU6050_ctx *tmp_ctx);
void MPU6050_deinit(void);
void MPU6050_set_sample_rate_divider_reg(uint8_t div);
void MPU6050_get_temp(int16_t *temp);
void MPU6050_get_acc_x(int16_t *acc_x);
void MPU6050_get_acc_y(int16_t *acc_y);
void MPU6050_get_acc_z(int16_t *acc_z);
void MPU6050_get_gyro_x(int16_t *gyro_x);
void MPU6050_get_gyro_y(int16_t *gyro_y);
void MPU6050_get_gyro_z(int16_t *gyro_z);
