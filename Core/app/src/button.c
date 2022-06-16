/*
 * button.c
 *
 *  Created on: Jan 17, 2021
 *      Author: Karol Witusik
 */

#include "button.h"
#include "main.h"

extern void SystemClock_Config(void);

//--------------------------------------------------------------------------------

static void button_do_button_fun(void);

//--------------------------------------------------------------------------------

static struct button ctx = {.port = BUTTON_GPIO_Port, .pin = BUTTON_Pin, .press_button_fun = blank_fun, .hold_button_fun = blank_fun, .double_press_button_fun = blank_fun};

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

void button_set_callback_double_press_function(callback_double_press_button fun)
{
    ctx.double_press_button_fun = fun;
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
        	if ((HAL_GetTick() - ctx.start_press_time) < BUTTON_DOUBLE_PRESS_TIME && ctx.press == true)
        	{
        		ctx.double_press = true;
        	}
        	else
        	{
        		ctx.press = true;
        		ctx.start_press_time = HAL_GetTick();
        	}
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
	/* check if press flag was set in `button_EXTI_handler` function and button is not pressed */
    if(ctx.press == true && ctx.actual_pin_state == true)
    {
    	/* if the button pressing time is greater than BUTTON_KILL_TIME then goto stop mode */
    	if ((HAL_GetTick() - ctx.start_press_time) > BUTTON_KILL_TIME)
    	{
			main_goto_stop_mode();
			main_goto_normal_mode();
			ctx.press = false;
    	}
    	/* if the button pressing time is greater than BUTTON_HOLD_TIME then call hold button function */
        else if(((HAL_GetTick() - ctx.start_press_time) > BUTTON_HOLD_TIME))
        {
            ctx.hold_button_fun();
			ctx.press = false;
        }
    	/* if double press was detected then call double press function */
        else if(ctx.double_press)
        {
        	ctx.double_press = false;
        	ctx.double_press_button_fun();
			ctx.press = false;
        }
    	/*  if the button pressing time is greater than BUTTON_DOUBLE_PRESS_TIME then call press button function */
        else if ((HAL_GetTick() - ctx.start_press_time) > BUTTON_DOUBLE_PRESS_TIME) {
        	ctx.press_button_fun();
        	ctx.press = false;
        }
    }
}

