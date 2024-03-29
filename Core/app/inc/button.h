/*
 * button.h
 *
 *  Created on: Jan 17, 2021
 *      Author: Karol Witusik
 */

#pragma once

#include <stdbool.h>
#include "main.h"

//--------------------------------------------------------------------------------

#define BUTTON_DEBOUNCE_TIME 		20
#define BUTTON_DOUBLE_PRESS_TIME 	200 /* this macro define how long algorithm should wait for second button press */
#define BUTTON_HOLD_TIME     		750
#define BUTTON_KILL_TIME     		2500

//--------------------------------------------------------------------------------

/** @brief      Pointers to function for button. */
typedef void (*callback_press_button)(void);
typedef void (*callback_double_press_button)(void);
typedef void (*callback_hold_button)(void);

//--------------------------------------------------------------------------------

/** @brief      Button structure describing all parameters. */
struct button
{
    GPIO_TypeDef                	*port;
    uint16_t                    	pin;
    volatile uint32_t           	start_press_time;
    uint32_t 						double_press_time;
    volatile uint32_t           	start_unpress_time;
    volatile bool               	actual_pin_state;
    volatile bool               	press;
    volatile bool               	double_press;
    callback_press_button       	press_button_fun;
    callback_double_press_button    double_press_button_fun;
    callback_hold_button        	hold_button_fun;
};

//--------------------------------------------------------------------------------

/*
 * @brief   This function should be insert in main loop.
 */
void button_process(void);

//--------------------------------------------------------------------------------

/** @brief   Set callback function for pressed button.
 *  @param   fun - function
 */
void button_set_callback_press_function(callback_press_button fun);

//--------------------------------------------------------------------------------

/** @brief   Set callback function for holded button.
 *  @param   fun - function
 */
void button_set_callback_hold_function(callback_hold_button fun);

//--------------------------------------------------------------------------------

/** @brief   Set callback function for double pressed button.
 *  @param   fun - function
 */
void button_set_callback_double_press_function(callback_double_press_button fun);

//--------------------------------------------------------------------------------

/*
 * @brief   This function should be insert in HAL_GPIO_EXTI_Callback for BUTTON interrupt.
 */
void button_EXTI_handler(void);

//--------------------------------------------------------------------------------

/*
 * @brief   Blank function;
 */
void blank_fun(void);
