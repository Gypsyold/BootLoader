#include "stm32f10x.h"                  // Device header
#include "AT24C02.h"
#include "I2C.h"
#include "delay.h"
#include "main.h"
#include "string.h"
#include <stdio.h>
void AT24C02_Init(void)
{
    My_I2C_Init();
}



uint8_t AT24C02_WriteByte(uint8_t addr,uint8_t wdata)
{
    MyI2C_Start();
    MyI2C_SendByte(AT24C02_WADDR);
    while(MyI2C_ReceiveAck());
    MyI2C_SendByte(addr);
    while(MyI2C_ReceiveAck());
    MyI2C_SendByte(wdata);
    while(MyI2C_ReceiveAck());
    MyI2C_Stop();
    return 0;

}

uint8_t AT24C02_WritePage(uint8_t addr,uint8_t *wdata)
{
    uint8_t i;
    MyI2C_Start();
    MyI2C_SendByte(AT24C02_WADDR);
    while(MyI2C_ReceiveAck());
    MyI2C_SendByte(addr);
    while(MyI2C_ReceiveAck());
    for(i=0;i<16;i++)  // AT24C02页大小为16字节
    {
        MyI2C_SendByte(wdata[i]);
        while(MyI2C_ReceiveAck());

    }
    MyI2C_Stop();
    Delay_ms(5);  // 等待写入完成，AT24C02需要5ms写入周期时间
    return 0;

}


uint8_t AT24C02_ReadData(uint8_t addr,uint8_t *rdata,uint16_t len)
{
    uint16_t i;
    MyI2C_Start();
    MyI2C_SendByte(AT24C02_WADDR);
    while(MyI2C_ReceiveAck());
    MyI2C_SendByte(addr);
    while(MyI2C_ReceiveAck());
    MyI2C_Start();
    MyI2C_SendByte(AT24C02_RADDR);
    while(MyI2C_ReceiveAck());
    for ( i = 0; i < len - 1; i++)
    {
       rdata[i] = MyI2C_ReceiveByte();
       MyI2C_SendAck(0);
    }

    rdata[len - 1] = MyI2C_ReceiveByte();
    MyI2C_SendAck(1);

    MyI2C_Stop();
    return 0;

}


/**
 * @brief 从AT24C02 EEPROM中读取OTA信息
 * 
 * 该函数用于从AT24C02 EEPROM存储器中读取OTA(Over-The-Air)升级相关信息。
 * 首先将OTA_Info结构体清零，然后从EEPROM的地址0开始读取数据到OTA_Info结构体中。
 * 
 * @note 读取的数据大小由OTA_INFOCB_SIZE宏定义决定
 * @note AT24C02_ReadData是底层的EEPROM读取函数
 */
void AT24C02_ReadOTAInfo(void)
{
    // 将OTA_Info结构体内存清零，确保数据初始化
    memset(&OTA_Info, 0, OTA_INFOCB_SIZE);
    
    // 从EEPROM的0地址开始读取OTA信息数据
    // 参数说明：
    // - 0: EEPROM的起始地址
    // - (uint8_t *)&OTA_Info: 存储读取数据的目标缓冲区
    // - OTA_INFOCB_SIZE: 要读取的数据字节数
    AT24C02_ReadData(0, (uint8_t *)&OTA_Info, OTA_INFOCB_SIZE);
	printf("%x\r\n",OTA_Info.OTA_flag);
}


/**
 * @brief 将OTA信息写入AT24C02 EEPROM
 * 
 * 该函数用于将OTA_Info结构体中的信息分页写入AT24C02 EEPROM存储器中。
 * 由于AT24C02的页写入特性，数据需要按页（16字节）进行写入。
 * 每次页写入后需要延时5ms以确保数据正确写入。
 * 
 * @note AT24C02每页大小为16字节
 * @note 每次页写入操作后需要等待5ms的写入时间
 * @note 写入的总数据大小由OTA_INFOCB_SIZE宏定义决定
 */
void AT24C02_WriteOTAInfo(void)
{
    uint8_t i;              // 循环计数器，用于控制写入的页数
    uint8_t *wptr;          // 指向OTA_Info结构体的指针，用于数据写入
    
    // 将OTA_Info的地址赋值给写指针
    wptr = (uint8_t *)&OTA_Info;
    
    // 循环写入每一页数据
    // 每次写入16字节（一页），直到写入所有数据
    for(i=0; i<OTA_INFOCB_SIZE/16; i++)
    {
        // 写入一页数据
        // 参数说明：
        // - i*16: 当前页的起始地址
        // - wptr+i*16: 要写入的数据的起始地址
        AT24C02_WritePage(i*16, wptr+i*16);
        
        // 延时5ms，等待EEPROM完成写入操作
        Delay_ms(5);
    }
}

