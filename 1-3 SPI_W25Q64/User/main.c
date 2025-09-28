#include "stm32f10x.h"                  // Device header
#include "usart.h"
#include "delay.h"
#include "AT24C02.h"
#include "W25Q64.h"


uint8_t wdata[256];
uint8_t rdata[256];

int main(void)
{	


	Delay_Init();
	USART1_Init(921600);
	W25Q64_Init();

	W25Q64_Erase_Block_64K(0);	//擦除第0号block
	//64 * 1024 = 65536     65536 / 256(每页字节) = 256(页)


	uint16_t i ,j;
	for (i = 0; i < 256; i++)
	{
		for (j = 0; j < 256; j++)
		{
			wdata[j] = i;

		}

		W25Q64_Write_Page(wdata, i);
	}
	
	Delay_ms(50);

	for (i = 0; i < 256; i++)
	{
	    W25Q64_Read(rdata, i*256, 256);
		for(j = 0; j < 256; j++)
		{
			printf("地址：%d  数据：%d\r\n",i*256+j, rdata[j]);
		}
	}
	

	while (1)
	{

	}
}
