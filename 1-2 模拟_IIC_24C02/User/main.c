#include "stm32f10x.h"                  // Device header
#include "usart.h"
#include "delay.h"
#include "AT24C02.h"


uint8_t rdata_buff[256];
uint8_t wdata_buff1[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
uint8_t wdata_buff2[8] = {0,1,2,3,4,5,6,7};

int main(void)
{	
	Delay_Init();  			// 初始化SysTick延时函数
	USART1_Init(921600);	// 初始化串口
	AT24C02_Init();
	uint16_t i;
	
	 for(i = 0; i < 256; i++)
	 {
	 	AT24C02_WriteByte(i, i);
	 	Delay_ms(5);
	 }
	printf("demo_1\r\n");

//	for(i = 0;i<8;i++)
//	{
//	    AT24C02_WritePage(i*8,wdata_buff2);
//	}

	AT24C02_ReadData(0,rdata_buff,256);
	
	for(i = 0; i < 256; i++)
	{
	    printf("地址%d = %x \r\n",i,rdata_buff[i]);
	}



	while (1)
	{

	}
}
