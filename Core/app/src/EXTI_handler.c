/*
 * EXTI_handler.c
 *
 *  Created on: Jan 17, 2021
 *      Author: Karol Witusik
 */
#include "MPU6050.h"
#include "button.h"
#include "main.h"

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if(GPIO_Pin == INTA_Pin)
    {
        QMC5883L_EXTI_handler();
    }
    else if(GPIO_Pin == BUTTON_Pin)
    {
        button_EXTI_handler();
    }
}
