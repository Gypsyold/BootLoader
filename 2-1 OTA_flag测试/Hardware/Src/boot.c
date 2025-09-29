#include "stm32f10x.h"                  // Device header
#include "boot.h"
#include "usart.h"
#include "delay.h"
#include "AT24C02.h"
#include "W25Q64.h"
#include "main.h"


void BootLoader_Brance(void)
{
	if(OTA_Info.OTA_flag == OTA_SET_FLAG)
	{
	    printf("OTA 更新\r\n");
	}else
	{
	    printf("跳转A分区\r\n");
	}

}

