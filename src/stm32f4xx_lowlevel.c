#include <stdint.h>
#include "stm32f4xx_hal.h"

void TASS_BSP_Init(void)
{
    // set the system clock to be HSE
    SystemClock_Config();
    HAL_Init();
    HAL_SYSTICK_Config(SystemCoreClock / 1000); // HAL_Init sets systick wrong, so do it again
}

void SysTick_Hook(void)
{
    HAL_IncTick();
}
