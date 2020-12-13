#include "MPU6050.h"

#define ADDR 0xD0
#define COUNT_OF_CALIB_REG 11

#define I2C_HANDLE hi2c1

/*
 * Register addresses
 */

#define SELF_TEST_X         0x0D
#define SELF_TEST_Y         0x0E
#define SELF_TEST_Z         0x0F
#define SELF_TEST_A         0x10
#define SMPLRT_DIV          0x19
#define CONFIG              0x1A
#define GYRO_CONFIG         0x1B
#define ACCEL_CONFIG        0x1C
#define FIFO_EN             0x23
#define I2C_MST_CTRL        0x24
#define I2C_SLV0_ADDR       0x25
#define I2C_SLV0_REG        0x26
#define I2C_SLV0_CTRL       0x27
#define I2C_SLV1_ADDR       0x28
#define I2C_SLV1_REG        0x29
#define I2C_SLV1_CTRL       0x2A
#define I2C_SLV2_ADDR       0x2B
#define I2C_SLV2_REG        0x2C
#define I2C_SLV2_CTRL       0x2D
#define I2C_SLV3_ADDR       0x2E
#define I2C_SLV3_REG        0x2F
#define I2C_SLV3_CTRL       0x30
#define I2C_SLV4_ADDR       0x31
#define I2C_SLV4_REG        0x32
#define I2C_SLV4_DO         0x33
#define I2C_SLV4_CTRL       0x34
#define I2C_SLV4_DI         0x35
#define I2C_MST_STATUS      0x36
#define INT_PIN_CFG         0x37
#define INT_ENABLE          0x38
#define INT_STATUS          0x3A
#define ACCEL_XOUT_H        0x3B
#define ACCEL_XOUT_L        0x3C
#define ACCEL_YOUT_H        0x3D
#define ACCEL_YOUT_L        0x3E
#define ACCEL_ZOUT_H        0x3F
#define ACCEL_ZOUT_L        0x40
#define TEMP_OUT_H          0x41
#define TEMP_OUT_L          0x42
#define GYRO_XOUT_H         0x43
#define GYRO_XOUT_L         0x44
#define GYRO_YOUT_H         0x45
#define GYRO_YOUT_L         0x46
#define GYRO_ZOUT_H         0x47
#define GYRO_ZOUT_L         0x48
#define EXT_SENS_DATA_00    0x49
#define EXT_SENS_DATA_01    0x4A
#define EXT_SENS_DATA_02    0x4B
#define EXT_SENS_DATA_03    0x4C
#define EXT_SENS_DATA_04    0x4D
#define EXT_SENS_DATA_05    0x4E
#define EXT_SENS_DATA_06    0x4F
#define EXT_SENS_DATA_07    0x50
#define EXT_SENS_DATA_08    0x51
#define EXT_SENS_DATA_09    0x52
#define EXT_SENS_DATA_10    0x53
#define EXT_SENS_DATA_11    0x54
#define EXT_SENS_DATA_12    0x55
#define EXT_SENS_DATA_13    0x56
#define EXT_SENS_DATA_14    0x57
#define EXT_SENS_DATA_15    0x58
#define EXT_SENS_DATA_16    0x59
#define EXT_SENS_DATA_17    0x5A
#define EXT_SENS_DATA_18    0x5B
#define EXT_SENS_DATA_19    0x5C
#define EXT_SENS_DATA_20    0x5D
#define EXT_SENS_DATA_21    0x5E
#define EXT_SENS_DATA_22    0x5F
#define EXT_SENS_DATA_23    0x60
#define I2C_SLV0_DO         0x63
#define I2C_SLV1_DO         0x64
#define I2C_SLV2_DO         0x65
#define I2C_SLV3_DO         0x66
#define I2C_MST_DELAY_CTRL  0x67
#define SIGNAL_PATH_RESET   0x68
#define USER_CTRL           0x6a
#define PWR_MGMT_1          0x6b
#define PWR_MGMT_2          0x6c
#define FIFO_COUNTH         0x72
#define FIFO_COUNTL         0x73
#define FIFO_R_W            0x74
#define WHO_AM_I            0x75


#define AMOUNT_OF_SLAVES    4

//----------------------------------------------------------------------

uint8_t *MPU6050_data;
static uint8_t aux_tab[3];
static uint8_t count_of_data_byte;

//----------------------------------------------------------------------

struct MPU6050_ctx ctx;

//----------------------------------------------------------------------

void MPU6050_init(struct MPU6050_ctx *tmp_ctx)
{
    ctx = *tmp_ctx;

    aux_tab[1] = ctx.clock_select;
    HAL_I2C_Mem_Write(&I2C_HANDLE, ADDR, PWR_MGMT_1, 1, &aux_tab[1], 1, I2C_TIMEOUT); //turn on device, select clk

    aux_tab[1] = ctx.sample_rate_div - 1;
    HAL_I2C_Mem_Write(&I2C_HANDLE, ADDR, SMPLRT_DIV, 1, &aux_tab[1], 1, I2C_TIMEOUT);   //set sample rate divider

    HAL_I2C_Mem_Write(&I2C_HANDLE, ADDR, CONFIG, 1, &ctx.dlpf_acc_bandwidth, 1, I2C_TIMEOUT);   //set DLPF

    HAL_I2C_Mem_Write(&I2C_HANDLE, ADDR, GYRO_CONFIG, 1, &ctx.gyro_full_scale_range, 1, I2C_TIMEOUT);   //set gyro range

    HAL_I2C_Mem_Write(&I2C_HANDLE, ADDR, ACCEL_CONFIG, 1, &ctx.acc_full_scale_range, 1, I2C_TIMEOUT);

    HAL_I2C_Mem_Write(&I2C_HANDLE, ADDR, FIFO_EN, 1, &ctx.fifo_data_enable_mask, 1, I2C_TIMEOUT);

    aux_tab[1] = ctx.master.master_clock_speed | (ctx.master.mst_p_nsr << 4) | (ctx.master.slave3_fifo_en << 5) | (ctx.master.wait_for_es << 6) | (ctx.master.mult_mst_en << 7);
    HAL_I2C_Mem_Write(&I2C_HANDLE, ADDR, I2C_MST_CTRL, 1, &aux_tab[1], 1, I2C_TIMEOUT);

    if(ctx.master.slave_delay) //set sample rate for slaves
    {
        HAL_I2C_Mem_Write(&I2C_HANDLE, ADDR, I2C_SLV4_CTRL, 1, &ctx.master.slave_delay, 1, I2C_TIMEOUT);

        aux_tab[1] = (1 << 7) | ctx.master.slave_delay_mask;
        HAL_I2C_Mem_Write(&I2C_HANDLE, ADDR, I2C_MST_DELAY_CTRL, 1, &aux_tab[1], 1, I2C_TIMEOUT);
    }

    for(uint8_t i = 0; i < AMOUNT_OF_SLAVES; i++)
    {
        aux_tab[1] = ctx.slave[i].addr | (ctx.slave[i].RW << 7);
        HAL_I2C_Mem_Write(&I2C_HANDLE, ADDR, I2C_SLV0_ADDR + 3*i, 1, &aux_tab[1], 1, I2C_TIMEOUT);

        HAL_I2C_Mem_Write(&I2C_HANDLE, ADDR, I2C_SLV0_REG + 3*i, 1, &ctx.slave[i].reg_addr, 1, I2C_TIMEOUT);

        aux_tab[1] = ctx.slave[i].len | (ctx.slave[i].group << 4) | (ctx.slave[i].reg_dis << 5) | (ctx.slave[i].byte_swap << 6) | (ctx.slave[i].en << 7);
        HAL_I2C_Mem_Write(&I2C_HANDLE, ADDR, I2C_SLV0_CTRL + 3*i, 1, &aux_tab[1], 1, I2C_TIMEOUT);
    }

    aux_tab[1] = (ctx.i2c_bypass_en << 1) | (ctx.fsync_int_level << 2) | (ctx.fsync_int_en << 3) | (ctx.int_pin.rd_clear << 4) | (ctx.int_pin.latch_en << 5) | (ctx.int_pin.open << 6) | (ctx.int_pin.level << 7);
    HAL_I2C_Mem_Write(&I2C_HANDLE, ADDR, INT_PIN_CFG, 1, &aux_tab[1], 1, I2C_TIMEOUT);

    HAL_I2C_Mem_Write(&I2C_HANDLE, ADDR, INT_ENABLE, 1, &ctx.interrupt_en_mask, 1, I2C_TIMEOUT);

    aux_tab[1] = (ctx.i2c_mst_en << 5) | (ctx.fifo_en << 6);

    for(uint8_t i = 0; i < 8; i++)
    {
        if(ctx.fifo_data_enable_mask & (1 << i))
        {
            switch(i)
            {
            case MPU6050_SLV0_FIFO_EN:
                count_of_data_byte += ctx.slave[0].len;
            case MPU6050_SLV1_FIFO_EN:
                count_of_data_byte += ctx.slave[1].len;
            case MPU6050_SLV2_FIFO_EN:
                count_of_data_byte += ctx.slave[2].len;
            case MPU6050_ACCEL_FIFO_EN:
                count_of_data_byte += 6;
                break;
            default:
                count_of_data_byte += 2;
            }
        }
    }

    if(ctx.master.slave3_fifo_en == true)
    {
        count_of_data_byte += ctx.slave[3].len;
    }

    MPU6050_data = (uint8_t*)malloc(count_of_data_byte);
}

void MPU6050_deinit(void)
{
    aux_tab[1] = 1 << 7;
    HAL_I2C_Mem_Write(&I2C_HANDLE, ADDR, PWR_MGMT_1, 1, &aux_tab[1], 1, I2C_TIMEOUT);
    free(MPU6050_data);
}

static void read_data(void)
{
    uint16_t size;
    uint32_t tick = HAL_GetTick();

    HAL_I2C_Mem_Read(&I2C_HANDLE, ADDR, FIFO_COUNTH, 1, (uint8_t*)&size + 1, 1, I2C_TIMEOUT);   //todo mozna sie o to zapytac
    HAL_I2C_Mem_Read(&I2C_HANDLE, ADDR, FIFO_COUNTL, 1, (uint8_t*)&size, 1, I2C_TIMEOUT);

    HAL_I2C_Mem_Read(&I2C_HANDLE, ADDR, FIFO_COUNTH, 1, &aux_tab[2], 2, I2C_TIMEOUT); //todo

    HAL_I2C_Mem_Read(&I2C_HANDLE, ADDR, FIFO_COUNTH, 1, (uint8_t*)&size + 1, 1, I2C_TIMEOUT);   //todo mozna sie o to zapytac
        HAL_I2C_Mem_Read(&I2C_HANDLE, ADDR, FIFO_COUNTL, 1, (uint8_t*)&size, 1, I2C_TIMEOUT);

    if((size % 14))
    {
        __NOP();
    }

    for(uint16_t i = 0; i < size; i++)
    {
        static uint8_t tmp;
        tmp++;
        if(tmp == count_of_data_byte)
        {
            tmp -= count_of_data_byte;
        }

        HAL_I2C_Mem_Read(&I2C_HANDLE, ADDR, FIFO_R_W, 1, &MPU6050_data[tmp], 1, I2C_TIMEOUT);
    }
    uint8_t tmp = size;
    HAL_I2C_Mem_Read(&I2C_HANDLE, ADDR, FIFO_COUNTH, 1, (uint8_t*)&size + 1, 1, I2C_TIMEOUT);   //todo mozna sie o to zapytac
    HAL_I2C_Mem_Read(&I2C_HANDLE, ADDR, FIFO_COUNTL, 1, (uint8_t*)&size, 1, I2C_TIMEOUT);
    uint32_t time = HAL_GetTick() - tick;
    __NOP();
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if(GPIO_Pin == INTA_Pin)
    {
        uint8_t tmp;

        HAL_I2C_Mem_Read(&I2C_HANDLE, ADDR, INT_STATUS, 1, &tmp, 1, I2C_TIMEOUT);

        if(tmp & MPU6050_INT_DATA_RDY_EN)
        {
            read_data();
        }

        if(tmp & MPU6050_INT_MST_EN)
        {
            __NOP();
        }

        if(tmp & MPU6050_INT_FIFO_OFLOW_EN)
        {
            __NOP();
        }
    }
}
