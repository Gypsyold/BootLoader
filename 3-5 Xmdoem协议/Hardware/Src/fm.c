#include "stm32f10x.h"                  // Device header
#include "fm.h"
#include "delay.h"

/**
 * @brief  擦除STM32的Flash指定页
 * @param  start: 起始页号(从0开始)
 * @param  num:   要擦除的页数
 * @retval None
 * @note   Flash页大小为1KB(1024字节)
 *        STM32F1xx的Flash起始地址为0x08000000
 */
void STM32_EraseFlash(uint16_t start, uint16_t num)
{
	uint16_t i;
	
	// 解锁Flash，允许进行擦除和写操作
	FLASH_Unlock();
	
	// 循环擦除指定数量的页
	for(i = 0; i < num; i++)
	{
		// 计算要擦除的页地址：
		// 0x8000000是Flash起始地址
		// start * 1024是起始页的偏移地址
		// i * 1024是当前页相对于起始页的偏移
		FLASH_ErasePage((0x8000000 + start * 1024) + (i * 1024));
	}
	
	// 重新锁定Flash，防止意外操作
	FLASH_Lock();
}





/**
 * @brief  向STM32的Flash写入数据
 * @param  saddr:  要写入的Flash起始地址
 * @param  wdata:  指向要写入的数据的指针
 * @param  wnum:   要写入的数据总字节数
 * @retval None
 * @note   1. STM32的Flash写入必须以32位(4字节)为单位进行
 *         2. 写入前目标区域必须已经擦除
 *         3. 写入地址必须4字节对齐
 */
void STM32_WriteFlash(uint32_t saddr, uint32_t *wdata, uint32_t wnum)
{
    // 解锁Flash，允许进行写操作
    FLASH_Unlock();
    
    // 循环写入数据，直到所有数据写入完成
    while(wnum)
    {
        // 以32位(4字节)为单位写入一个字到Flash
        FLASH_ProgramWord(saddr, *wdata);
        
        // 更新剩余字节数(每次写入4字节)
        wnum -= 4;
        
        // 更新目标地址(每次增加4字节)
        saddr += 4;
        
        // 移动到下一个要写入的数据
        wdata++;
    }
    
    // 重新锁定Flash，防止意外操作
    FLASH_Lock();
}
