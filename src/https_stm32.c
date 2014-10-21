#ifdef STM32F407xx

#include "https_stm32.h"
#include "pico_stack.h"
#include "stm32f4xx.h"

// private functions
static void stm32_https_int_timer(pico_time now, void* arg);
static void actuateleds(void);

void stm32_https_board_init(void){

	////////////////
	//  LED INIT  //
	////////////////
	led1_state=0;
	led2_state=1;
	led3_state=0;
	led4_state=0;
	
	// Clock the GPIO module
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN ; 
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN ; 
    
    // Configure pins 
    GPIOD->MODER = (1<<30) | (1<<28) | (1<<26) | (1<<24);       // Output
    GPIOD->PUPDR = 0;       // No pull

	// Button stuff
	GPIOD->MODER &= ~(1<<0); // Input on pin 0
	GPIOD->PUPDR |= (2<<0);  // Pull down on pin 0
	actuateleds();

	pico_timer_add(1000,stm32_https_int_timer, NULL);
}

static void stm32_https_int_timer(pico_time now, void* arg){
	static int state = 0;
	const static int delays[] = {500,50,100,50};
	const static int ledOn[]  = {0,1,0,1};
	state = (state+1)%4;
	led2_state = ledOn[state];
	actuateleds();
	pico_timer_add(delays[state],stm32_https_int_timer, NULL);
}

void stm32_https_poll_state(void){
	button_state = GPIOA->IDR & 1; // Bit 0 is user button 
}

void stm32_https_toggle(int num){
	static uint8_t *asArray[] = { &led1_state, &led2_state, &led3_state, &led4_state };
	*(asArray[num]) ^= 1;
	actuateleds();
}

static void actuateleds(void){
	GPIOD->ODR &= 0x0FFF; // Clear led bits
	GPIOD->ODR |= (led1_state << 12) | (led2_state << 15) | (led3_state << 13) | (led4_state << 14); // Set led bits
}

#endif
