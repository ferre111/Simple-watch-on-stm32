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
#include "rtc.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "OLED.h"
#include "images.h"
#include "myI2C.h"
#include "pressure_sensor.h"
#include "MPU6050.h"
#include <stdio.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define PRES_TAB_SIZE 10
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
    uint16_t fps = 0;
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
  MX_RTC_Init();
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


  char temp1Text[20];
  uint8_t first_line;
  OLED_createTextField(&first_line, 0, 0, temp1Text, 1);

  char temp2Text[20];
  uint8_t second_line;
  OLED_createTextField(&second_line, 0, 8, temp2Text, 1);

  char accXText[20];
  uint8_t third_line;
  OLED_createTextField(&third_line, 0, 16, accXText, 1);

  char accYText[20];
  uint8_t fourth_line;
  OLED_createTextField(&fourth_line, 0, 24, accYText, 1);

  char accZText[20];
  uint8_t fifth_line;
  OLED_createTextField(&fifth_line, 0, 55, accZText, 1);


  int32_t temp1, pres, init_pres = 0;
  int16_t temp2, acc_x, acc_y, acc_z, gyro_x, gyro_y, gyro_z, mag_x, mag_y, mag_z;
  float alt;

//  uint8_t timeTextField = 0;
//  RTC_TimeTypeDef time_s;
//  char timeText[8];
//
//  uint8_t dateTextField = 0;
//  RTC_DateTypeDef date_s;
//  char dateText[8];
//
//  uint8_t fpsTextField = 0;
//  char fpsText[8];
//
//  uint8_t tempTextField = 0;
//  char tempText[20];
//
//  uint8_t tempPresField = 0;
//  char tempPresText[20];
//
//  uint8_t oldTime = 0;
//
//
//  OLED_createTextField(&timeTextField, 15, 24, timeText, 2);
//  OLED_createTextField(&dateTextField, 38, 8, dateText, 1);
//  OLED_createTextField(&fpsTextField, 100, 0, fpsText, 1);
//  OLED_createTextField(&tempTextField, 1, 60, tempText, 1);
//  OLED_createTextField(&tempPresField, 1, 0, tempPresText, 1);

  uint8_t x, y = 0;
  while (1)
  {

      HAL_Delay(20);
      HAL_GPIO_TogglePin(RED_LED_GPIO_Port, RED_LED_Pin);

//      HAL_RTC_GetTime(&hrtc, &time_s, RTC_FORMAT_BIN);
//      sprintf(timeText, "%02d:%02d:%02d", time_s.Hours, time_s.Minutes, time_s.Seconds);
//
//      HAL_RTC_GetDate(&hrtc, &date_s, RTC_FORMAT_BIN);
//      sprintf(dateText, "%02d/%02d/%02d", date_s.Date, date_s.Month, date_s.Year);

      pressure_sensor_read_temp_and_pres();
      pressure_sensor_get_temp(&temp1);
      snprintf(temp1Text, 20, PRINT_TEMP(temp1), abs(temp1) / 10, abs(temp1) % 10);

      MPU6050_get_temp(&temp2);
      snprintf(temp2Text, 20, PRINT_TEMP(temp2), abs(temp2) / 10, abs(temp2) % 10);


      MPU6050_get_acc_x(&acc_x);
      MPU6050_get_acc_y(&acc_y);
      MPU6050_get_acc_z(&acc_z);
      snprintf(accXText, 20, PRINT_ACC(acc_x, x) , abs(acc_x) / 1000, abs(acc_x) % 1000);
      snprintf(accYText, 20, PRINT_ACC(acc_y, y) , abs(acc_y) / 1000, abs(acc_y) % 1000);
      snprintf(accZText, 20, PRINT_ACC(acc_z, z) , abs(acc_z) / 1000, abs(acc_z) % 1000);

      OLED_update();

//      fps++;
//      if(time_s.Seconds != oldTime)
//      {
//          oldTime = time_s.Seconds;
//          sprintf(fpsText, "%2d", fps);
//          fps = 0;
//      }


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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
//void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c)
//{
//    HAL_GPIO_TogglePin(RED_LED_GPIO_Port, RED_LED_Pin);
//    fps++;
//}


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
