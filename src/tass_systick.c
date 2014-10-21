volatile unsigned int tassTick = 0u; /* Architecture dependant clock! */

void __attribute__((weak)) SysTick_Hook(void)
{
}

void __attribute__((weak)) SysTick_RTOS_Hook(void)
{
}


void SysTick_Handler(void)
{
    tassTick++;
    SysTick_Hook();
    SysTick_RTOS_Hook();
}

