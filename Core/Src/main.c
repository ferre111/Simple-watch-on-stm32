/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "MPU6050.h"
#include "pressure_sensor.h"
#include "OLED.h"
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define PRINT_TEMP(temp) temp >= 0 ? "Temperature: %d.%dC" : "Temperature: -%d.%dC"
#define PRINT_ACC(acc, axi) acc >= 0  ? "Acc " #axi ": %d.%.3dg" : "Acc " #axi ": -%d.%.3dg"
#define PRINT_GYRO(gyro, axi) gyro >= 0 ? "Gyro " #axi ": %d.%.3ddeg/s" : "Gyro " #axi ": -%d.%.3ddeg/s"
#define PRINT_MAG(mag, axi) mag >= 0  ? "Mag " #axi ": %d.%.3dG" : "Mag " #axi ": -%d.%.3dG"
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
struct MPU6050_ctx ctx =
{
    .sample_rate_div        = 255,
    .dlpf_acc_bandwidth     = MPU6050_BANDWIDTH_260,
    .gyro_full_scale_range  = MPU6050_GYRO_FULL_SCALE_2000,
    .acc_full_scale_range   = MPU6050_ACC_FULL_SCALE_16,
    .fifo_data_enable_mask  = MPU6050_ACCEL_FIFO_EN | MPU6050_ZG_FIFO_EN | MPU6050_YG_FIFO_EN | MPU6050_XG_FIFO_EN | MPU6050_TEMP_FIFO_EN | MPU6050_SLV0_FIFO_EN,
    .clock_select           = MPU6050_PLL_X_GYRO,

    .master.master_clock_speed = MPU6050_I2C_CLOCK_SPEED_400,
    .master.mult_mst_en        = false,
    .master.wait_for_es        = true,
    .master.mst_p_nsr          = true,
    .master.slave_delay        = 0,
    .i2c_bypass_en             = false,

    .slave[0].addr      = QMC588L_ADDR,
    .slave[0].RW        = true,
    .slave[0].reg_addr  = QMC588L_XOUT_L,
    .slave[0].byte_swap = false,
    .slave[0].reg_dis   = false,
    .slave[0].group     = false,
    .slave[0].len       = 6,
    .slave[0].en        = true,

    .int_pin.level = true,
    .int_pin.open = false,
    .int_pin.latch_en = false,
    .int_pin.rd_clear = false,
    .fsync_int_level = false,
    .fsync_int_en = false,
    .i2c_bypass_en = false,

    .interrupt_en_mask = MPU6050_INT_DATA_RDY_EN | MPU6050_INT_FIFO_OFLOW_EN | MPU6050_INT_MST_EN,
    .fifo_en    = true,
    .i2c_mst_en = true,

    .QMC5883L_ctx.mode = QMC5883L_MODE_CONTINUOUS,
    .QMC5883L_ctx.output_data_rate = QMC5883L_ODR_10,
    .QMC5883L_ctx.full_scale = QMC5883L_RNG_8G,
    .QMC5883L_ctx.over_sample_ratio = QMC5883L_OSR_512
};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  /* USER CODE BEGIN 2 */
  myI2C_Init();
  HAL_Delay(50);
  MPU6050_deinit();
  HAL_Delay(50);
  pressure_sensor_set_sensor_mode(PRESSURE_SENSOR_ULTRA_HIGH_RESOLUTION);
  pressure_sensor_read_calib_data();


  MPU6050_init(&ctx);

  OLED_Init();
  OLED_setDisplayOn();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  uint8_t avr = 100;
  int64_t temp1, pres, init_pres = 0;
  int16_t temp2, acc_x, acc_y, acc_z, gyro_x, gyro_y, gyro_z, mag_x, mag_y, mag_z;
  float alt;
  char tmp[20];

//  for(uint8_t i = 0; i < avr; i++)
//  {
//      static int32_t tmp_init_pres = 0;
//
//      pressure_sensor_read_temp_and_pres();
//      pressure_sensor_get_pres(&tmp_init_pres);
//
//      init_pres += tmp_init_pres;
//  }
//  init_pres /= avr;
  uint8_t first_line;
  OLED_createTextField(&first_line, 10, 10, tmp, 2);
//  OLED_createTextField(&dateTextField, 38, 8, dateText, 1);
//  OLED_createTextField(&fpsTextField, 100, 0, fpsText, 1);
//  OLED_createTextField(&tempTextField, 10, 0, tempText, 1);
  while (1)
  {
//    pres = 0;
//    temp1 = 0;
//    temp2 = 0;
//
//    for(uint8_t i = 0; i < avr; i++)
//    {
//        static int32_t tmp_pres = 0, tmp_temp1 = 0, tmp_temp2 = 0;
//
//        pressure_sensor_read_temp_and_pres();
//        pressure_sensor_get_temp(&tmp_temp1);
//        MPU6050_get_temp((int16_t*)&tmp_temp2);
//        pressure_sensor_get_pres(&tmp_pres);
//
//        pres += tmp_pres;
//        temp1 += tmp_temp1;
//        temp2 += tmp_temp2;
//    }
//    pres /= avr;
//    temp1 /= avr;
//    temp2 /= avr;

//    pressure_sensor_calc_dif_alt(init_pres, pres, &alt);
    MPU6050_get_temp(&temp2);
    snprintf(tmp, 20, PRINT_TEMP(temp2), abs(temp2) / 10, abs(temp2) % 10);

//    snprintf(tmp, 20, "Temperature: %d.%dC", (int32_t)(temp2 / 10), (int32_t)(temp2 % 10));
//    OLED_printText(1, 0, tmp);
//    snprintf(tmp, 20, "Pressure: %d.%d%dhPa", (int32_t)(pres / 100), (int32_t)((pres % 100) / 10), (int32_t)(pres % 10));
//    OLED_printText(2, 0, tmp);
//    snprintf(tmp, 20, "Altitude: %fm", alt);
//    OLED_printText(3, 0, tmp);
//    MPU6050_get_acc_x(&acc_x);
//    MPU6050_get_acc_y(&acc_y);
//    MPU6050_get_acc_z(&acc_z);
//    snprintf(tmp, 20, PRINT_ACC(acc_x, x) , abs(acc_x) / 1000, abs(acc_x) % 1000);
//    OLED_printText(1, 0, tmp);
//    snprintf(tmp, 20, PRINT_ACC(acc_y, y) , abs(acc_y) / 1000, abs(acc_y) % 1000);
//    OLED_printText(2, 0, tmp);
//    snprintf(tmp, 20, PRINT_ACC(acc_z, z) , abs(acc_z) / 1000, abs(acc_z) % 1000);
//    OLED_printText(3, 0, tmp);

//    MPU6050_get_gyro_x(&gyro_x);
//    MPU6050_get_gyro_y(&gyro_y);
//    MPU6050_get_gyro_z(&gyro_z);
//    snprintf(tmp, 20, PRINT_GYRO(gyro_x, x), abs(gyro_x) / 1000, abs(gyro_x) % 1000);
//    OLED_printText(4, 0, tmp);
//    snprintf(tmp, 20, PRINT_GYRO(gyro_y, y), abs(gyro_y) / 1000, abs(gyro_y) % 1000);
//    OLED_printText(5, 0, tmp);
//    snprintf(tmp, 20, PRINT_GYRO(gyro_z, z), abs(gyro_z) / 1000, abs(gyro_z) % 1000);
//    OLED_printText(6, 0, tmp);
//
//    QMC5883L_get_mag_x(&mag_x);
//    QMC5883L_get_mag_y(&mag_y);
//    QMC5883L_get_mag_z(&mag_z);
//    snprintf(tmp, 20, PRINT_MAG(mag_x, x), abs(mag_x) / 1000, abs(mag_x) % 1000);
//    OLED_printText(4, 0, tmp);
//    snprintf(tmp, 20, PRINT_MAG(mag_y, y), abs(mag_y) / 1000, abs(mag_y) % 1000);
//    OLED_printText(5, 0, tmp);
//    snprintf(tmp, 20, PRINT_MAG(mag_z, z), abs(mag_z) / 1000, abs(mag_z) % 1000);
//    OLED_printText(6, 0, tmp);
    OLED_update();

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
