#ifndef HTTPS_STM32_H_
#define HTTPS_STM32_H_

#include "pico_stack.h"
#include "stm32f4xx.h"

volatile uint8_t led1_state;
volatile uint8_t led2_state;
volatile uint8_t led3_state;
volatile uint8_t led4_state;
volatile uint8_t button_state;

void stm32_https_board_init(void);
void stm32_https_poll_state(void);
void stm32_https_toggle(int);

#endif
