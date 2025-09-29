#include "stm32f10x.h"                  // Device header
#include "usart.h"
#include "delay.h"
#include "AT24C02.h"
#include "W25Q64.h"
#include "main.h"
#include "boot.h"



OTA_InfoCB OTA_Info;
int main(void)
{	


	Delay_Init();
	USART1_Init(921600);
	AT24C02_Init();
	AT24C02_ReadOTAInfo();
	BootLoader_Brance();
	while (1)
	{

	}
}
