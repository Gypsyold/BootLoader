#include "stm32f10x.h"                  // Device header
#include "usart.h"
#include "delay.h"
#include "AT24C02.h"
#include "W25Q64.h"
#include "main.h"
#include "boot.h"
#include "fm.h"


uint32_t wbuff[1024];
uint32_t i;

OTA_InfoCB OTA_Info;
int main(void)
{	


	Delay_Init();
	USART1_Init(921600);
	for(i = 0;i<1024;i++)
	{
		wbuff[i] = 0x12345678;
	
	}
	STM32_EraseFlash(60,4);
	
	STM32_WriteFlash(60 * 1024 + 0x08000000,wbuff,1024 * 4);
	
	for(i = 0;i<1024;i++)
	{
		printf("%x\r\n",*((uint32_t *)((60 * 1024 + 0x08000000) + (i * 4))));
	
	}


	while (1)
	{

	}
}
