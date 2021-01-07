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
#include "i2c.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "MPU6050.h"
#include "pressure_sensor.h"
#include "OLED.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
struct MPU6050_ctx ctx =
{
    .sample_rate_div        = 8,
    .dlpf_acc_bandwidth     = MPU6050_BANDWIDTH_260,
    .gyro_full_scale_range  = MPU6050_GYRO_FULL_SCALE_2000,
    .acc_full_scale_range   = MPU6050_ACC_FULL_SCALE_16,
    .fifo_data_enable_mask  = MPU6050_ACCEL_FIFO_EN | MPU6050_ZG_FIFO_EN | MPU6050_YG_FIFO_EN | MPU6050_XG_FIFO_EN | MPU6050_TEMP_FIFO_EN,
    .master.master_clock_speed = MPU6050_I2C_CLOCK_SPEED_400,
    .clock_select           = MPU6050_PLL_X_GYRO,

    .master.mult_mst_en            = false,
    .master.wait_for_es            = true,
    .master.mst_p_nsr              = false,
    .i2c_bypass_en                 = false,

//    .slave[0].addr            = PRESSUURE_SENSOR_ADDR,
//    .slave[0].reg_addr           = 0, //todo
//    .slave[0].byte_swap       = false,
//    .slave[0].reg_dis     = false,
//    .slave[0].group           = false,

    .int_pin.level = true,
    .int_pin.open = false,
    .int_pin.latch_en = true,
    .int_pin.rd_clear = false, //todo
    .fsync_int_level = false,
    .fsync_int_en = false,

    .interrupt_en_mask = MPU6050_INT_DATA_RDY_EN | MPU6050_INT_FIFO_OFLOW_EN | MPU6050_INT_MST_EN,
    .fifo_en    = true,
    .i2c_mst_en = false,
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
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  uint8_t usun;
  pressure_sensor_set_sensor_mode(PRESSURE_SENSOR_ULTRA_HIGH_RESOLUTION);
  pressure_sensor_read_calib_data();
  HAL_I2C_Mem_Read(&hi2c1, 0xD0, 0x6b, 1, &usun, 1, I2C_TIMEOUT);
  MPU6050_deinit();
  HAL_I2C_Mem_Read(&hi2c1, 0xD0, 0x6b, 1, &usun, 1, I2C_TIMEOUT);
  MPU6050_init(&ctx);
  HAL_I2C_Mem_Read(&hi2c1, 0xD0, 0x6b, 1, &usun, 1, I2C_TIMEOUT);
  OLED_Init();
  OLED_setDisplayOn();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  uint8_t avr = 100;
  int64_t temp, pres, init_pres = 0;
  float alt;
  char tmp[20];

  for(uint8_t i = 0; i < avr; i++)
  {
      static int32_t tmp_init_pres = 0;

      pressure_sensor_read_temp_and_pres();
      pressure_sensor_get_pres(&tmp_init_pres);

      init_pres += tmp_init_pres;
  }
  init_pres /= avr;

  while (1)
  {
    pres = 0;
    temp = 0;

    for(uint8_t i = 0; i < avr; i++)
    {
        static int32_t tmp_pres = 0, tmp_temp = 0;

        pressure_sensor_read_temp_and_pres();
        pressure_sensor_get_temp(&tmp_temp);
        pressure_sensor_get_pres(&tmp_pres);

        pres += tmp_pres;
        temp += tmp_temp;
    }
    pres /= avr;
    temp /= avr;

    pressure_sensor_calc_dif_alt(init_pres, pres, &alt);

    snprintf(tmp, 20, "Temperature: %d.%dC", (int32_t)(temp / 10), (int32_t)(temp % 10));
    OLED_printText(0, 0, tmp);
    snprintf(tmp, 20, "Pressure: %d.%d%dhPa", (int32_t)(pres / 100), (int32_t)((pres % 100) / 10), (int32_t)(pres % 10));
    OLED_printText(1, 0, tmp);
    snprintf(tmp, 20, "Altitude: %fm", alt);
    OLED_printText(2, 0, tmp);
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
