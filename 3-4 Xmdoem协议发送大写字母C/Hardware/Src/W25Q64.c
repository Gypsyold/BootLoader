#include "stm32f10x.h"                  // Device header
#include "delay.h"
#include "W25Q64.h"
#include "SPI.h"

void W25Q64_Init(void)
{

    My_SPI_Init();
    
}


void W25Q64_Wait_Busy(void)
{
    uint8_t res = 0;
    do
    {
        MySPI_W_SS(0);
        SPI1_ReadWriteByte(W25Q64_READ_STATUS_REGISTER_1);           // 读取第一个状态寄存器
        res = SPI1_ReadWriteByte(W25Q64_DUMMY_BYTE);
        MySPI_W_SS(1);
    } while (res & 0x01);  // S0 位为 BUSY 位，为1表示正在忙,需要等待busy置零
    
}


void W25Q64_Write_Enable(void)
{
    W25Q64_Wait_Busy(); // 等待W25Q64空闲
    MySPI_W_SS(0);
    SPI1_ReadWriteByte(W25Q64_WRITE_ENABLE);    // W25Q64写使能
    MySPI_W_SS(1);
}


//W25Q64代表64M的二进制位，换算成字节就是8M
//8*1024/64 = 128个block
void W25Q64_Erase_Block_64K(uint8_t BlockNum)
{
    uint8_t wdata[4];
    wdata[0] = W25Q64_BLOCK_ERASE_64KB;
    wdata[1] = (BlockNum * 64 *1024) >> 16;
    wdata[2] = (BlockNum * 64 *1024) >> 8;
    wdata[3] = (BlockNum * 64 *1024) >> 0;

    W25Q64_Wait_Busy();
    W25Q64_Write_Enable();
    MySPI_W_SS(0);
    SPI1_Write(wdata, 4);
    MySPI_W_SS(1);
    W25Q64_Wait_Busy();

}


void W25Q64_Write_Page(uint8_t *wbuff, uint16_t pageNum)
{
    uint8_t wdata[4];

    wdata[0] = W25Q64_PAGE_PROGRAM;
    wdata[1] = (pageNum * 256) >> 16;
    wdata[2] = (pageNum * 256) >> 8;
    wdata[3] = (pageNum * 256) >> 0;

    W25Q64_Wait_Busy();
    W25Q64_Write_Enable();
    MySPI_W_SS(0);

    SPI1_Write(wdata, 4);
    SPI1_Write(wbuff, 256);
    MySPI_W_SS(1);

}

void W25Q64_Read(uint8_t *rbuff,uint32_t addr,uint32_t datalen)
{
    uint8_t wdata[4];

    wdata[0] = W25Q64_READ_DATA;
    wdata[1] = (addr) >> 16;
    wdata[2] = (addr) >> 8;
    wdata[3] = (addr) >> 0;

    W25Q64_Wait_Busy();
    MySPI_W_SS(0);
    SPI1_Write(wdata, 4);
    SPI1_Read(rbuff, datalen);
    MySPI_W_SS(1);


}

