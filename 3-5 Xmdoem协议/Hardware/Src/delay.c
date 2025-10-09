#include "stm32f10x.h"                  // Device header
#include "delay.h"


void Delay_Init(void)
{
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);    //72Mhz,计数72个数代表过去了1us
}

void Delay_us(uint32_t us)
{
    SysTick->LOAD = us * 72 - 1;                                // 72Mhz,计数72个数代表过去了1us
    SysTick->VAL = 0;
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;                   // 使能计数器
    while(!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk));       // 等待计数完成
     SysTick->CTRL &=~ SysTick_CTRL_ENABLE_Msk;                 // 关闭计数器

}

void Delay_ms(uint32_t ms)
{
    while(ms--)
    {
        Delay_us(1000);
    }

}

