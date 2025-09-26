#include "stm32f10x.h"                  // Device header
#include "usart.h"


uint16_t i;


int main(void)
{
	USART1_Init(921600);
	U0_printf("%d,%c,%x\r\n",0x30,0x30,0x30);
	
	while (1)
	{
		if(U0CB.URxDataOUT != U0CB.URxDataIN)
		{
			U0_printf("本次接收了%d字节数据\r\n",U0CB.URxDataOUT->end - U0CB.URxDataOUT->start + 1);	//数据长度 = 结束索引 - 开始索引 + 1
			for(i = 0;i<U0CB.URxDataOUT->end - U0CB.URxDataOUT->start + 1;i++)
			{
				U0_printf("%c ",U0CB.URxDataOUT->start[i]);
			}

			U0_printf("\r\n\r\n");
			
			U0CB.URxDataOUT ++;
			if(U0CB.URxDataOUT >= U0CB.URxDataEND)
			{
				U0CB.URxDataOUT = &U0CB.URxDataPtr[0];
			}


		}
	}
}
