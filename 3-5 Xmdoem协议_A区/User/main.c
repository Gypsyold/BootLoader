#include "stm32f10x.h"                  // Device header
#include "usart.h"


//生成bin文件：
//"E:\Keil\Keil_v5\Keil5_MDK\ARM\ARMCC\bin\fromelf.exe" --bin -o "E:\Github_warehouse\Boot_Loader\3-5 Xmdoem协议_A区\Project.bin" "E:\Github_warehouse\Boot_Loader\3-5 Xmdoem协议_A区\Objects\Project.axf"
int main(void)
{
	USART1_Init(921600);
	printf("%d,%c,%x\r\n",0x30,0x30,0x30);
	
	while (1)
	{
		if(U0CB.URxDataOUT != U0CB.URxDataIN)
		{
			printf("本次接收了%d字节数据\r\n",U0CB.URxDataOUT->end - U0CB.URxDataOUT->start + 1);	//数据长度 = 结束索引 - 开始索引 + 1
			for(uint16_t i = 0;i<U0CB.URxDataOUT->end - U0CB.URxDataOUT->start + 1;i++)
			{
				printf("---%c %d---",U0CB.URxDataOUT->start[i],i);

			}



			printf("\r\n\r\n");
			
			U0CB.URxDataOUT ++;
			if(U0CB.URxDataOUT >= U0CB.URxDataEND)
			{
				U0CB.URxDataOUT = &U0CB.URxDataPtr[0];
			}


		}
	}
}
