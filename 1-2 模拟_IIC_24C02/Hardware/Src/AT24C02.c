#include "stm32f10x.h"                  // Device header
#include "AT24C02.h"
#include "I2C.h"
#include "delay.h"

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
    for(i=0;i<8;i++)
    {
        MyI2C_SendByte(wdata[i]);
        while(MyI2C_ReceiveAck());

    }
    MyI2C_Stop();
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

