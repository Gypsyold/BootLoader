#include "stm32f10x.h"                  // Device header
#include "usart.h"
#include "delay.h"

int main(void)
{
	USART1_Init(921600);	// 初始化串口
	Delay_Init();  			// 初始化SysTick延时函数

	
	while (1)
	{
		U0_printf("12345\r\n");
		Delay_ms(1000);
	}
}
