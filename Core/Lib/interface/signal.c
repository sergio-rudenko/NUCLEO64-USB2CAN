/*
 * signal.c
 *
 *  Created on: Sep 4, 2020
 *      Author: sa100
 */

#include "signal.h"
#include <stdio.h>

/**
 *
 */
static void
priv_sequence(Signal_t *p, uint32_t t1, uint32_t t0,
        uint32_t pattern, uint16_t cnt, uint16_t mul)
{
    if (!signal_is_enabled(p)) {
        signal_enable(p);
    }

    if (!signal_is_in_sequence(p)) {
        p->pattern = pattern;
        p->counter = (cnt == SIGNAL_INFINITE) ? cnt : cnt * mul;

        p->timer.t0 = t0;
        p->timer.t1 = t1;

        if (p->pattern & 0x01) {
            if (signal_get_output(p)) {
                /* additional pause */
                p->counter += (p->counter == SIGNAL_INFINITE) ? 0 : 1;
                p->timeout = p->timer.t0;
                signal_set_output(p, RESET);
            }
            else {
                p->timeout = p->timer.t1;
                signal_set_output(p, SET);
            }
        }
        else {
            p->timeout = p->timer.t0;
            signal_set_output(p, RESET);
        }

        p->State = SignalState_Sequence;
    }
}

/**
 * Initialize structure
 */
Signal_t signal_init(
        GPIO_TypeDef *port, uint32_t pin,
        SignalActiveLevel_t Level, uint8_t id)
{
    Signal_t init = {
        .id = id,

        .gpio = {
            .Level = Level,
            .port = port,
            .pin = pin,
        },

        .timer = {
            .t0 = 0UL,
            .t1 = 0UL,
        },

        .State = (SignalState_t) SignalState_Reset,

        .inverted = false,

        .timeout = 0UL,
        .savedTimeStamp = 0UL,
    };
    return init;
}

/**
 *
 */
void signal_run(Signal_t *p)
{
    uint32_t ms = signal_get_elapsed_ms(p);

    if (p->timeout >= ms) {
        p->timeout -= ms;
    }
    else {
        p->timeout = 0;

        switch (p->State)
        {
            case SignalState_Set:
                signal_set_output(p, SET);
                break;

            case SignalState_Sequence:
                if (p->counter != SIGNAL_INFINITE) {
                    p->counter--;
                }

                if (p->counter) {
                    p->pattern = (p->pattern << 31) | (p->pattern >> 1);
                    if (p->pattern & 0x01) {
                        p->timeout = p->timer.t1;
                        signal_set_output(p, SET);
                    }
                    else {
                        p->timeout = p->timer.t0;
                        signal_set_output(p, RESET);
                    }
                }
                else {
                    p->State = SignalState_Reset;
                    signal_set_output(p, RESET);
                }
                break;

            default:
                signal_set_output(p, RESET);
                break;
        }
    }
}

/**
 *
 */
bool signal_is_enabled(Signal_t *p)
{
    return (p->gpio.pin && p->gpio.port &&
            p->State != SignalState_Disabled);
}

/**
 *
 */
bool signal_is_in_sequence(Signal_t *p)
{
    return (p->gpio.pin && p->gpio.port &&
            p->State == SignalState_Sequence);
}

/* API */

void signal_enable(Signal_t *p)
{
    p->State = SignalState_Reset;
    signal_set_output(p, RESET);
}

void signal_disable(Signal_t *p)
{
    p->State = SignalState_Disabled;
    signal_set_output(p, RESET);
}

void signal_level(Signal_t *p, bool level)
{
    p->timeout = 0UL;
    p->inverted = false;
    p->State = (level) ? SignalState_Set : SignalState_Reset;
    signal_set_output(p, level);
}

void signal_toggle(Signal_t *p)
{
    bool level = signal_get_output(p);

    if (p->timeout) {
        p->timeout = 0UL;
    }
    p->State = (level) ? SignalState_Reset : SignalState_Set;
    signal_set_output(p, !level);
}

void signal_blink(Signal_t *p, uint32_t lenght, uint16_t count, bool inverted)
{
    p->inverted = inverted;
    priv_sequence(p, lenght, (count > 1) ? lenght : 1,
    SIGNAL_PATTERN_MEANDR, count, 2);
}

void signal_impulse(Signal_t *p, uint32_t timeHigh, uint32_t timeLow,
        uint16_t count, bool inverted)
{
    p->inverted = inverted;
    priv_sequence(p, timeHigh, timeLow,
    SIGNAL_PATTERN_MEANDR, count, 2);
}

void signal_pattern(Signal_t *p, uint32_t pattern, uint32_t timeQuant,
        uint16_t count, bool inverted)
{
    p->inverted = inverted;
    priv_sequence(p, timeQuant, timeQuant, pattern, count, 32);
}

/* LowLevel (weak) */

__attribute__((weak))
  uint32_t signal_get_elapsed_ms(Signal_t *p)
{
    uint32_t tick = HAL_GetTick();
    uint32_t ms = tick - p->savedTimeStamp;

    p->savedTimeStamp = HAL_GetTick();
    return ms;
}

__attribute__((weak))
bool signal_get_output(Signal_t *p)
{
    bool b;

    if (p->gpio.port && p->gpio.pin) {
        b = LL_GPIO_IsOutputPinSet(p->gpio.port, p->gpio.pin);
        return (b == p->gpio.Level);
    }
    return false;
}

__attribute__((weak))
void signal_set_output(Signal_t *p, bool level)
{
    bool b;

    if (p->gpio.port && p->gpio.pin) {
        b = (level == p->gpio.Level);
        b = (p->inverted) ? !b : b;

        if (b) {
            LL_GPIO_SetOutputPin(p->gpio.port, p->gpio.pin);
        }
        else {
            LL_GPIO_ResetOutputPin(p->gpio.port, p->gpio.pin);
        }
    }
}
