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

#define BUTTON_DEBOUNCE_TIME 10
#define BUTTON_HOLD_TIME     1000

//--------------------------------------------------------------------------------

/** @brief      Pointers to function for button. */
typedef void (*callback_press_button)(void);
typedef void (*callback_hold_button)(void);

//--------------------------------------------------------------------------------

/** @brief      Button state. */
enum button_state
{
    IDLE,
    WATCH,
};

//--------------------------------------------------------------------------------

/** @brief      Button structure describing all parameters. */
struct button
{
    GPIO_TypeDef                *port;
    uint16_t                    pin;
    volatile enum button_state           button_state;
    volatile uint32_t           start_press_time;
    volatile uint32_t           start_unpress_time;
    volatile bool                        actual_pin_state;
    volatile bool               press;
    callback_press_button       press_button_fun;
    callback_hold_button        hold_button_fun;
};

void button_process(void);
void button_set_callback_press_function(callback_press_button fun);
void button_set_callback_hold_function(callback_hold_button fun);
void button_EXTI_handler(void);
