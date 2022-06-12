/*
 * button.c
 *
 *  Created on: Jan 17, 2021
 *      Author: Karol Witusik
 */

#include "button.h"
#include "OLED.h"
#include "MPU6050.h"
#include "main.h"

extern void SystemClock_Config(void);

//--------------------------------------------------------------------------------

static void button_do_button_fun(void);

//--------------------------------------------------------------------------------

static struct button ctx = {.port = BUTTON_GPIO_Port, .pin = BUTTON_Pin, .press_button_fun = blank_fun, .hold_button_fun = blank_fun};

//--------------------------------------------------------------------------------

void button_set_callback_press_function(callback_press_button fun)
{
    ctx.press_button_fun = fun;
}

//--------------------------------------------------------------------------------

void button_set_callback_hold_function(callback_hold_button fun)
{
    ctx.hold_button_fun = fun;
}

//--------------------------------------------------------------------------------

void button_process(void)
{
    button_do_button_fun();
}

//--------------------------------------------------------------------------------

void button_EXTI_handler(void)
{
    ctx.actual_pin_state = !HAL_GPIO_ReadPin(ctx.port, ctx.pin);

    if(ctx.actual_pin_state == false)
    {
        if((HAL_GetTick() - ctx.start_unpress_time) > BUTTON_DEBOUNCE_TIME)
        {
            ctx.start_press_time = HAL_GetTick();
            ctx.press = true;
        }
    }
    else
    {
        ctx.start_unpress_time = HAL_GetTick();
    }
}


//--------------------------------------------------------------------------------

void blank_fun(void)
{
    __NOP();
}

//--------------------------------------------------------------------------------

static void button_do_button_fun(void)
{
    if(ctx.press == true)
    {
    	if ((HAL_GetTick() - ctx.start_press_time) > BUTTON_KILL_TIME && ctx.actual_pin_state == true)
    	{
    		OLED_setDisplayOff();
    		MPU6050_sleep();
			ctx.press = false;
			HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
    		HAL_Delay(100);
    		HAL_PWR_EnterSTOPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
    		SystemClock_Config();
    		OLED_setDisplayOn();
			MPU6050_init(&MPU6050);
			HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
    	}
        else if(((HAL_GetTick() - ctx.start_press_time) > BUTTON_HOLD_TIME) && ctx.actual_pin_state == true)
        {
            ctx.hold_button_fun();
			ctx.press = false;
        }
        else if(ctx.actual_pin_state == true)
        {
            ctx.press_button_fun();
			ctx.press = false;
        }
    }
}

