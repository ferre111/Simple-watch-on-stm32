/*
 * button.c
 *
 *  Created on: Jan 17, 2021
 *      Author: Karol Witusik
 */

#include "button.h"

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
    ctx.actual_pin_state =  HAL_GPIO_ReadPin(ctx.port, ctx.pin);

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
//    static bool flag;
//    HAL_GPIO_WritePin(TEST_LED_GPIO_Port, TEST_LED_Pin, flag);
//    flag ^= 1;
}

//--------------------------------------------------------------------------------

static void button_do_button_fun(void)
{
    if(ctx.press == true)
    {
        if(ctx.actual_pin_state == true)
        {
            ctx.press = false;
            ctx.press_button_fun();
        }
        else if((HAL_GetTick() - ctx.start_press_time) > BUTTON_HOLD_TIME)
        {
            ctx.press = false;
            ctx.hold_button_fun();
        }
    }
}

