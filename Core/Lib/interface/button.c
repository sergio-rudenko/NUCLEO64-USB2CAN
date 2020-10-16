/*
 * button.c
 *
 *  Created on: Jun 22, 2020
 *      Author: sergi
 */

#include "button.h"
#include <stdio.h>

/**
 * Initialize structure
 */
Button_t button_init(
        GPIO_TypeDef *port, uint32_t pin, ButtonActiveLevel_t Level, uint8_t id) {

    Button_t init = {
        .id = id,

        .gpio = {
            .Level = Level,
            .port = port,
            .pin = pin,
        },

        .time = {
            .debounce = BUTTON_DEFAULT_DEBOUNCE_TIME_MS,
            .longPress = BUTTON_DEFAULT_LONGPRESS_TIME_MS,
            .extraLongPress = BUTTON_DEFAULT_EXTRALONGPRESS_TIME_MS,
        },

        .State = (ButtonState_t) ButtonState_Undefined,

        .timeout = 0UL,
        .savedTimeStamp = 0UL,
    };
    return init;
}

/**
 *
 */
void button_run(Button_t *p) {
    uint32_t ms = button_get_elapsed_ms(p);

    ButtonState_t Current = button_get_input(p) ? ButtonState_Pressed :
                                                  ButtonState_Released;

    if (p->State == ButtonState_Undefined) {
        p->State = button_get_input(p) ? ButtonState_DebouncePress :
                                         ButtonState_DebounceRelease;
        p->PrevState = ButtonState_Disabled;
    }

    if (p->timeout >= ms) {
        p->timeout -= ms;
    } else {
        p->timeout = 0;
    }

    switch (p->State)
    {
        case ButtonState_DebounceRelease:
            if (Current == ButtonState_Pressed) {
                p->State = ButtonState_DebouncePress;
                p->timeout = p->time.debounce;
            } else {
                if (p->timeout == 0) {
                    p->State = ButtonState_Released;

                    if (p->PrevState != ButtonState_Released) {
                        button_released_callback(p);
                    }
                }
            }
            break;

        case ButtonState_DebouncePress:
            if (Current == ButtonState_Pressed) {
                if (p->timeout == 0) {
                    switch (p->PrevState)
                    {
                        case ButtonState_Pressed:
                            p->timeout = p->time.longPress;
                            p->State = ButtonState_Pressed;
                            break;

                        case ButtonState_PressedLong:
                            p->timeout = p->time.extraLongPress;
                            p->State = ButtonState_PressedLong;
                            break;

                        case ButtonState_PressedExtraLong:
                            p->State = ButtonState_PressedExtraLong;
                            break;

                        default:
                            p->timeout = p->time.longPress;
                            p->State = ButtonState_Pressed;
                            button_pressed_callback(p);
                            break;
                    }
                }
            } else {
                p->timeout = p->time.debounce;
                p->State = ButtonState_Released;
            }
            break;

        case ButtonState_Released:
            if (Current == ButtonState_Pressed) {
                p->PrevState = ButtonState_Released;
                p->State = ButtonState_DebouncePress;
                p->timeout = p->time.debounce;
            }
            break;

        case ButtonState_Pressed:
            if (Current == ButtonState_Pressed) {
                if (p->timeout == 0) {
                    p->timeout = p->time.extraLongPress;
                    p->State = ButtonState_PressedLong;
                    button_pressed_long_callback(p);
                }
            } else {
                p->PrevState = ButtonState_Pressed;
                p->State = ButtonState_DebounceRelease;
                p->timeout = p->time.debounce;
            }
            break;

        case ButtonState_PressedLong:
            if (Current == ButtonState_Pressed) {
                if (p->timeout == 0) {
                    p->State = ButtonState_PressedExtraLong;
                    button_pressed_extra_long_callback(p);
                }
            } else {
                p->PrevState = ButtonState_PressedLong;
                p->State = ButtonState_DebounceRelease;
                p->timeout = p->time.debounce;
            }
            break;

        case ButtonState_PressedExtraLong:
            if (Current == ButtonState_Released) {
                p->PrevState = ButtonState_PressedExtraLong;
                p->State = ButtonState_DebounceRelease;
                p->timeout = p->time.debounce;
            }
            break;

        default:
            break;
    }
}

/**
 *
 */
bool button_is_enabled(Button_t *p)
{
    return (p->gpio.pin && p->gpio.port &&
            p->State != ButtonState_Disabled);
}

/**
 *
 */
bool button_is_pressed(Button_t *p)
{
    return (p->State == ButtonState_Pressed ||
            p->State == ButtonState_PressedLong ||
            p->State == ButtonState_PressedExtraLong);
}

/* LowLevel (weak) */

__attribute__((weak))
uint32_t button_get_elapsed_ms(Button_t *p)
{
    uint32_t tick = HAL_GetTick();
    uint32_t ms = tick - p->savedTimeStamp;

    p->savedTimeStamp = HAL_GetTick();
    return ms;
}

__attribute__((weak))
bool button_get_input(Button_t *p)
{
    bool b;

    if (p->gpio.port && p->gpio.pin) {
        b = LL_GPIO_IsInputPinSet(p->gpio.port, p->gpio.pin);
        return (b == p->gpio.Level);
    }
    return false;
}

/* callbacks (weak) */

__attribute__((weak))
void button_released_callback(Button_t *p) {
    printf("BUTTON #%d Released\r\n", p->id);
}

__attribute__((weak))
void button_pressed_callback(Button_t *p) {
    printf("BUTTON #%d Pressed\r\n", p->id);
}

__attribute__((weak))
void button_pressed_long_callback(Button_t *p) {
    printf("BUTTON #%d Pressed Long\r\n", p->id);
}

__attribute__((weak))
void button_pressed_extra_long_callback(Button_t *p) {
    printf("BUTTON #%d Pressed Extra Long\r\n", p->id);
}

