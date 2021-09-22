/*
 * button.h
 *
 *  Created on: Jun 21, 2020
 *      Author: sergi
 */

#ifndef LIB_BUTTON_H_
#define LIB_BUTTON_H_

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

//#include  "gpio.h"
#include "stm32f0xx_ll_gpio.h"


#define BUTTON_DEFAULT_DEBOUNCE_TIME_MS			50
#define BUTTON_DEFAULT_LONGPRESS_TIME_MS		3000
#define BUTTON_DEFAULT_EXTRALONGPRESS_TIME_MS	10000

typedef enum ButtonState {
    ButtonState_Disabled = -1,
    ButtonState_Released = 0,
    ButtonState_Pressed,
    ButtonState_PressedLong,
    ButtonState_PressedExtraLong,

    ButtonState_DebounceRelease,
    ButtonState_DebouncePress,

    ButtonState_Undefined,
} ButtonState_t;

typedef enum ButtonActiveLevel {
    ButtonActiveLevel_LOW = 0,
    ButtonActiveLevel_HIGH = 1
} ButtonActiveLevel_t;

typedef struct Button {
    uint32_t timeout;
    uint32_t savedTimeStamp;

    ButtonState_t State;
    ButtonState_t PrevState;

    uint8_t id;

    struct {
        ButtonActiveLevel_t Level;
        GPIO_TypeDef *port;
        uint32_t pin;
    } gpio;

    struct {
        uint32_t debounce;
        uint32_t longPress;
        uint32_t extraLongPress;
    } time;
} Button_t;

/* prototypes */
Button_t button_init(
        GPIO_TypeDef *port, uint32_t pin,
        ButtonActiveLevel_t Level, uint8_t id);

void button_run(Button_t *instance);

/* LowLevel */
uint32_t button_get_elapsed_ms(Button_t *instance);
bool button_get_input(Button_t *instance);

/* API */
bool button_is_enabled(Button_t *instance);
bool button_is_pressed(Button_t *instance);

/* callbacks */
void button_pressed_callback(Button_t *instance);
void button_released_callback(Button_t *instance);
void button_pressed_long_callback(Button_t *instance);
void button_pressed_extra_long_callback(Button_t *instance);

#endif /* LIB_BUTTON_H_ */
