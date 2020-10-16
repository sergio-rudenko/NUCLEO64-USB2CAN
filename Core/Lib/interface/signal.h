/*
 * signal.h
 *
 *  Created on: Sep 4, 2020
 *      Author: sa100
 */

#ifndef LIB_SIGNAL_H_
#define LIB_SIGNAL_H_

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include  "gpio.h"

#define SIGNAL_INFINITE         0xFFFF
#define SIGNAL_PATTERN_MEANDR   0b01010101010101010101010101010101
#define SIGNAL_PATTERN_ALARM    0b00101010001110011100111000101010 /* SOS */

typedef enum SignalState {
    SignalState_Disabled = -1,
    SignalState_Reset = 0,
    SignalState_Set,
    SignalState_Sequence,
} SignalState_t;

typedef enum SignalActiveLevel {
    SignalActiveLevel_LOW = 0,
    SignalActiveLevel_HIGH = 1
} SignalActiveLevel_t;

typedef enum SignalInvertedState {
  SIGNAL_NORMAL = 0,
  SIGNAL_INVERTED = 1,
} SignalInvertedState_t;

typedef struct Signal {
    uint32_t timeout;
    uint32_t savedTimeStamp;

    SignalState_t State;

    uint8_t id;

    struct {
        SignalActiveLevel_t Level;
        GPIO_TypeDef *port;
        uint32_t pin;
    } gpio;

    struct {
        uint32_t t0;
        uint32_t t1;
    } timer;

    bool inverted;
    uint32_t pattern;
    uint32_t counter;

} Signal_t;

/* prototypes */
Signal_t signal_init(
        GPIO_TypeDef *port, uint32_t pin,
        SignalActiveLevel_t Level, uint8_t id);

void signal_run(Signal_t *instance);

bool signal_is_enabled(Signal_t *instance);
bool signal_is_in_sequence(Signal_t *instance);

/* LowLevel */
uint32_t signal_get_elapsed_ms(Signal_t *instance);
void signal_set_output(Signal_t *instance, bool level);
bool signal_get_output(Signal_t *instance);

/* API */
void signal_enable(Signal_t *instance);
void signal_disable(Signal_t *instance);

void signal_level(Signal_t *instance, bool level);
void signal_toggle(Signal_t *instance);

void signal_blink(Signal_t *instance,
        uint32_t lenght, uint16_t count, bool inverted);
void signal_impulse(Signal_t *instance,
        uint32_t timeHigh, uint32_t timeLow, uint16_t count, bool inverted);
void signal_pattern(Signal_t *instance,
        uint32_t pattern, uint32_t timeQuant, uint16_t count, bool inverted);

#endif /* LIB_SIGNAL_H_ */
