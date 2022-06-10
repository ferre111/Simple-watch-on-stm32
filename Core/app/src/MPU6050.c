#include "MPU6050.h"

#define ADDR 0xD0

#define I2C_HANDLE hi2c2

//----------------------------------------------------------------------

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
static uint8_t aux_val;
static uint8_t count_of_data_byte;
static bool read_data_flag;

//----------------------------------------------------------------------

struct MPU6050_ctx ctx;

//----------------------------------------------------------------------

static void set_sensitivity(void);
static void prepare_dynamic_array(void);

//----------------------------------------------------------------------

void MPU6050_init(struct MPU6050_ctx *tmp_ctx)
{
    ctx = *tmp_ctx;

    aux_val = ctx.clock_select;
    HAL_I2C_Mem_Write(&I2C_HANDLE, ADDR, PWR_MGMT_1, 1, &aux_val, 1, HAL_MAX_DELAY);
    // myI2C_writeByte(I2C_HANDLE, ADDR, PWR_MGMT_1, aux_val); //turn on device, select clk

    /* init QMC5883L */
    aux_val = (1 << 1);
    HAL_I2C_Mem_Write(&I2C_HANDLE, ADDR, INT_PIN_CFG, 1, &aux_val, 1, HAL_MAX_DELAY);
//    myI2C_writeByte(I2C_HANDLE, ADDR, INT_PIN_CFG, aux_val); //I2C bypass enable

    aux_val = (1 << 7);
    HAL_I2C_Mem_Write(&I2C_HANDLE, QMC588L_ADDR, QMC588L_MODE_REG_2, 1, &aux_val, 1, HAL_MAX_DELAY);
//    myI2C_writeByte(I2C_HANDLE, QMC588L_ADDR, QMC588L_MODE_REG_2, aux_val); //soft reset
    HAL_Delay(10);

    aux_val = 1;
    HAL_I2C_Mem_Write(&I2C_HANDLE, QMC588L_ADDR, QMC588L_FBR, 1, &aux_val, 1, HAL_MAX_DELAY);
//    myI2C_writeByte(I2C_HANDLE, QMC588L_ADDR, QMC588L_FBR, aux_val); //restart period

    aux_val = ctx.QMC5883L_ctx.mode | (ctx.QMC5883L_ctx.output_data_rate << 2) | (ctx.QMC5883L_ctx.full_scale << 4) | (ctx.QMC5883L_ctx.over_sample_ratio << 6);
    HAL_I2C_Mem_Write(&I2C_HANDLE, QMC588L_ADDR, QMC588L_MODE_REG_1, 1, &aux_val, 1, HAL_MAX_DELAY);
//    myI2C_writeByte(I2C_HANDLE, QMC588L_ADDR, QMC588L_MODE_REG_1, aux_val); //send settings for QMC5883L

    aux_val = ctx.sample_rate_div - 1;
    HAL_I2C_Mem_Write(&I2C_HANDLE, ADDR, SMPLRT_DIV, 1, &aux_val, 1, HAL_MAX_DELAY);
//    myI2C_writeByte(I2C_HANDLE, ADDR, SMPLRT_DIV, aux_val);   //set sample rate divider

    HAL_I2C_Mem_Write(&I2C_HANDLE, ADDR, CONFIG, 1, (uint8_t *)&ctx.dlpf_acc_bandwidth, 1, HAL_MAX_DELAY);
//    myI2C_writeByte(I2C_HANDLE, ADDR, CONFIG, ctx.dlpf_acc_bandwidth);   //set DLPF

    HAL_I2C_Mem_Write(&I2C_HANDLE, ADDR, GYRO_CONFIG, 1, (uint8_t *)&ctx.gyro_full_scale_range, 1, HAL_MAX_DELAY);
//    myI2C_writeByte(I2C_HANDLE, ADDR, GYRO_CONFIG, ctx.gyro_full_scale_range);   //set gyro range

    HAL_I2C_Mem_Write(&I2C_HANDLE, ADDR, ACCEL_CONFIG, 1, (uint8_t *)&ctx.acc_full_scale_range, 1, HAL_MAX_DELAY);
//    myI2C_writeByte(I2C_HANDLE, ADDR, ACCEL_CONFIG, ctx.acc_full_scale_range);   //set acc range

    HAL_I2C_Mem_Write(&I2C_HANDLE, ADDR, FIFO_EN, 1, &ctx.fifo_data_enable_mask, 1, HAL_MAX_DELAY);
//    myI2C_writeByte(I2C_HANDLE, ADDR, FIFO_EN, ctx.fifo_data_enable_mask);       //enable data that will be storage in FIFO

    aux_val = ctx.master.master_clock_speed | (ctx.master.mst_p_nsr << 4) | (ctx.master.slave3_fifo_en << 5) | (ctx.master.wait_for_es << 6) | (ctx.master.mult_mst_en << 7);
    HAL_I2C_Mem_Write(&I2C_HANDLE, ADDR, I2C_MST_CTRL, 1, &aux_val, 1, HAL_MAX_DELAY);
//    myI2C_writeByte(I2C_HANDLE, ADDR, I2C_MST_CTRL, aux_val);   //setup master control register

    if(ctx.master.slave_delay) //set sample rate for slaves
    {
    	HAL_I2C_Mem_Write(&I2C_HANDLE, ADDR, I2C_SLV4_CTRL, 1, &ctx.master.slave_delay, 1, HAL_MAX_DELAY);
//        myI2C_writeByte(I2C_HANDLE, ADDR, I2C_SLV4_CTRL, ctx.master.slave_delay);

        aux_val = (1 << 7) | ctx.master.slave_delay_mask;
        HAL_I2C_Mem_Write(&I2C_HANDLE, ADDR, I2C_MST_DELAY_CTRL, 1, &aux_val, 1, HAL_MAX_DELAY);
//        myI2C_writeByte(I2C_HANDLE, ADDR, I2C_MST_DELAY_CTRL, aux_val);
    }

    for(uint8_t i = 0; i < AMOUNT_OF_SLAVES; i++)   //set all slaves
    {
        aux_val = (ctx.slave[i].addr >> 1) | (ctx.slave[i].RW << 7);
        HAL_I2C_Mem_Write(&I2C_HANDLE, ADDR, I2C_SLV0_ADDR + 3*i, 1, &aux_val, 1, HAL_MAX_DELAY);
//        myI2C_writeByte(I2C_HANDLE, ADDR, I2C_SLV0_ADDR + 3*i, aux_val);    //set slave address

        HAL_I2C_Mem_Write(&I2C_HANDLE, ADDR, I2C_SLV0_REG + 3*i, 1, &ctx.slave[i].reg_addr, 1, HAL_MAX_DELAY);
//        myI2C_writeByte(I2C_HANDLE, ADDR, I2C_SLV0_REG + 3*i, ctx.slave[i].reg_addr);   //set slave register address

        aux_val = ctx.slave[i].len | (ctx.slave[i].group << 4) | (ctx.slave[i].reg_dis << 5) | (ctx.slave[i].byte_swap << 6) | (ctx.slave[i].en << 7);
        HAL_I2C_Mem_Write(&I2C_HANDLE, ADDR, I2C_SLV0_CTRL + 3*i, 1, &aux_val, 1, HAL_MAX_DELAY);
//        myI2C_writeByte(I2C_HANDLE, ADDR, I2C_SLV0_CTRL + 3*i, aux_val);    //set other settings for slave
    }

    aux_val = (ctx.i2c_bypass_en << 1) | (ctx.fsync_int_en << 2) | (ctx.fsync_int_level << 3) | (ctx.int_pin.rd_clear << 4) | (ctx.int_pin.latch_en << 5) | (ctx.int_pin.open << 6) | (ctx.int_pin.level << 7);
    HAL_I2C_Mem_Write(&I2C_HANDLE, ADDR, INT_PIN_CFG, 1, &aux_val, 1, HAL_MAX_DELAY);
//    myI2C_writeByte(I2C_HANDLE, ADDR, INT_PIN_CFG, aux_val);    //setup MPU6050 pin for generate interrupt

    HAL_I2C_Mem_Write(&I2C_HANDLE, ADDR, INT_ENABLE, 1, &ctx.interrupt_en_mask, 1, HAL_MAX_DELAY);
//    myI2C_writeByte(I2C_HANDLE, ADDR, INT_ENABLE, ctx.interrupt_en_mask);   //set when interrupt should happen

    /*prepare dynamic allocated array for data from FIFO*/
    prepare_dynamic_array();

    MPU6050_data = (uint8_t*)malloc(count_of_data_byte);    //allocate needed count of bytes in heap

    /*set correct sensitivity*/
    set_sensitivity();

    aux_val = (ctx.i2c_mst_en << 5) | (ctx.fifo_en << 6);
    HAL_I2C_Mem_Write(&I2C_HANDLE, ADDR, USER_CTRL, 1, &aux_val, 1, HAL_MAX_DELAY);
//    myI2C_writeByte(I2C_HANDLE, ADDR, USER_CTRL, aux_val);  //enable master mode on auxiliary I2C and enable FIFO
}

//----------------------------------------------------------------------

void MPU6050_deinit(void)
{
    //MPU6050 reset
    aux_val = 1 << 7;
    HAL_I2C_Mem_Write(&I2C_HANDLE, ADDR, PWR_MGMT_1, 1, &aux_val, 1, HAL_MAX_DELAY);
//    myI2C_writeByte(I2C_HANDLE, ADDR, PWR_MGMT_1, aux_val);

    free(MPU6050_data); //deallocate array for FIFO

    //auxiliary I2C reset
    HAL_Delay(10);
    aux_val = 1 << 1;
    HAL_I2C_Mem_Write(&I2C_HANDLE, ADDR, USER_CTRL, 1, &aux_val, 1, HAL_MAX_DELAY);
//    myI2C_writeByte(I2C_HANDLE, ADDR, USER_CTRL, aux_val);
}

//----------------------------------------------------------------------

static void read_data(void)
{
    uint16_t size;
    /*Check how many data is in FIFO*/
    HAL_I2C_Mem_Read(&I2C_HANDLE, ADDR, FIFO_COUNTH, 1, (uint8_t*)&size + 1, 1, HAL_MAX_DELAY);
//    myI2C_readByteStream(I2C_HANDLE, ADDR, FIFO_COUNTH, (uint8_t*)&size + 1, 1);
    HAL_I2C_Mem_Read(&I2C_HANDLE, ADDR, FIFO_COUNTL, 1, (uint8_t*)&size, 1, HAL_MAX_DELAY);
//    myI2C_readByteStream(I2C_HANDLE, ADDR, FIFO_COUNTL, (uint8_t*)&size, 1);

    for(uint16_t i = 0; i < size; i++)  //read all data in FIFO
    {
        static uint8_t tmp;

        if(tmp == count_of_data_byte)   //if there is more bytes in FIFO than count_of_data_byte then overwrite data
        {
            tmp -= count_of_data_byte;
        }

        HAL_I2C_Mem_Read(&I2C_HANDLE, ADDR, FIFO_R_W, 1, &MPU6050_data[tmp], 1, HAL_MAX_DELAY);
//        myI2C_readByteStream(I2C_HANDLE, ADDR, FIFO_R_W, &MPU6050_data[tmp], 1);
        tmp++;
    }

    /*Check gyro data. If there is enough deg/s then change displayed page.*/
    if((int16_t)((MPU6050_data[12] << 8) | MPU6050_data[13]) > GYRO_REG_VAL_TO_CHANGE_PAGE) menu_set_next_page_flag();
    if((int16_t)((MPU6050_data[12] << 8) | MPU6050_data[13]) < -GYRO_REG_VAL_TO_CHANGE_PAGE) menu_set_prev_page_flag();
}

//----------------------------------------------------------------------

void MPU6050_get_acc_data(struct MPU6050_acc_data *acc_data)
{
    acc_data->x = (int32_t)((int16_t)((MPU6050_data[0] << 8) | MPU6050_data[1])) * 1000 / ctx.acc_sensitivity;
    acc_data->y = (int32_t)((int16_t)((MPU6050_data[2] << 8) | MPU6050_data[3])) * 1000 / ctx.acc_sensitivity;
    acc_data->z = (int32_t)((int16_t)((MPU6050_data[4] << 8) | MPU6050_data[5])) * 1000 / ctx.acc_sensitivity;
}

//----------------------------------------------------------------------

void MPU6050_get_temp(int16_t *temp)
{
    *temp = (int16_t)((MPU6050_data[6] << 8) | MPU6050_data[7]) / 34 + 365;
}

//----------------------------------------------------------------------

void MPU6050_get_gyro_data(struct MPU6050_gyro_data *gyro_data)
{
    gyro_data->x = (int32_t)((int16_t)((MPU6050_data[8] << 8) | MPU6050_data[9])) * 10000 / ctx.gyro_sensitivity;
    gyro_data->y = (int32_t)((int16_t)((MPU6050_data[10] << 8) | MPU6050_data[11])) * 10000 / ctx.gyro_sensitivity;
    gyro_data->z = (int32_t)((int16_t)((MPU6050_data[12] << 8) | MPU6050_data[13])) * 10000 / ctx.gyro_sensitivity;
}

//----------------------------------------------------------------------

void QMC5883L_get_mag_data(struct QMC5883L_mag_data *mag_data)
{
    mag_data->x = (int16_t)((MPU6050_data[15] << 8) | MPU6050_data[14]) / ctx.mag_sensitivity;
    mag_data->y = (int16_t)((MPU6050_data[17] << 8) | MPU6050_data[16]) / ctx.mag_sensitivity;
    mag_data->z = (int16_t)((MPU6050_data[19] << 8) | MPU6050_data[18]) / ctx.mag_sensitivity;
}

//----------------------------------------------------------------------

void QMC5883L_process(void)
{
    if(read_data_flag)
    {
        read_data();
        read_data_flag = false;

    }
}

//----------------------------------------------------------------------

void QMC5883L_EXTI_handler(void)
{
    uint8_t tmp = 0;
    HAL_I2C_Mem_Read(&I2C_HANDLE, ADDR, INT_STATUS, 1, &tmp, 1, HAL_MAX_DELAY);
//    myI2C_readByteStream(I2C_HANDLE, ADDR, INT_STATUS, &tmp, 1);    //read register with data what event bring interrupt

    if(tmp & MPU6050_INT_DATA_RDY_EN)
    {
        read_data_flag = true;
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


//----------------------------------------------------------------------

static void set_sensitivity(void)
{
    /*Choose correct sensitivity depending on what user set*/
    switch(ctx.acc_full_scale_range)
    {
        case MPU6050_ACC_FULL_SCALE_2:
            ctx.acc_sensitivity = 16384;
            break;
        case MPU6050_ACC_FULL_SCALE_4:
            ctx.acc_sensitivity = 8192;
            break;
        case MPU6050_ACC_FULL_SCALE_8:
            ctx.acc_sensitivity = 4096;
            break;
        case MPU6050_ACC_FULL_SCALE_16:
            ctx.acc_sensitivity = 2048;
            break;
    }

    switch(ctx.gyro_full_scale_range)
    {
        case MPU6050_GYRO_FULL_SCALE_250:
            ctx.gyro_sensitivity = 1310;
            break;
        case MPU6050_GYRO_FULL_SCALE_500:
            ctx.gyro_sensitivity = 655;
            break;
        case MPU6050_GYRO_FULL_SCALE_1000:
            ctx.gyro_sensitivity = 328;
            break;
        case MPU6050_GYRO_FULL_SCALE_2000:
            ctx.gyro_sensitivity = 164;
            break;
    }

    switch(ctx.QMC5883L_ctx.full_scale)
    {
    case QMC5883L_RNG_2G:
        ctx.mag_sensitivity = 12;
        break;
    case QMC5883L_RNG_8G:
        ctx.mag_sensitivity = 3;
        break;
    }
}

//----------------------------------------------------------------------

static void prepare_dynamic_array(void)
{
    /*This function check how much data will be in FIFO*/
    for(uint8_t i = 0; i < 8; i++)
    {
        if(ctx.fifo_data_enable_mask & (1 << i))
        {
            switch(1 << i)
            {
            case MPU6050_SLV0_FIFO_EN:
                count_of_data_byte += ctx.slave[0].len;
                break;
            case MPU6050_SLV1_FIFO_EN:
                count_of_data_byte += ctx.slave[1].len;
                break;
            case MPU6050_SLV2_FIFO_EN:
                count_of_data_byte += ctx.slave[2].len;
                break;
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
}
