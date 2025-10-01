#include "stm32f10x.h"                  // Device header
#include "SPI.h"
#include "Delay.h"


/**
  * 函    数：SPI写SS引脚电平，SS仍由软件模拟
  * 参    数：BitValue 协议层传入的当前需要写入SS的电平，范围0~1
  * 返 回 值：无
  * 注意事项：此函数需要用户实现内容，当BitValue为0时，需要置SS为低电平，当BitValue为1时，需要置SS为高电平
  */
void MySPI_W_SS(uint8_t BitValue)
{
	GPIO_WriteBit(GPIOA, GPIO_Pin_4, (BitAction)BitValue);		//根据BitValue，设置SS引脚的电平
}


void My_SPI_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
    
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);					//将PA4引脚初始化为推挽输出
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);					//将PA5和PA7引脚初始化为复用推挽输出
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);					//将PA6引脚初始化为上拉输入
    


    SPI_I2S_DeInit(SPI1);
    SPI_InitTypeDef SPI_InitStructure;
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;      //全双工
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;                           //主机模式
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;                       //数据大小8位
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;                              //时钟极性
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;                            //时钟相位
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;                               //NSS信号由软件控制
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;      //波特率预分频2
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;                      //高位在前
    SPI_InitStructure.SPI_CRCPolynomial = 7;                                //CRC多项式
    SPI_Init(SPI1, &SPI_InitStructure);

    SPI_Cmd(SPI1, ENABLE);

	/*设置默认电平*/
	MySPI_W_SS(1);											//SS默认高电平

}


uint8_t SPI1_ReadWriteByte(uint8_t txd)
{
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) != SET);    //等待发送区空
    SPI_I2S_SendData(SPI1, txd);                                    //发送一个字节
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) != SET);   //等待接收区非空
    return SPI_I2S_ReceiveData(SPI1);                               //接收一个字节


}


void SPI1_Write(uint8_t *wdata,uint16_t datalen)
{
    uint16_t i;
    for(i=0;i<datalen;i++)
    {
        SPI1_ReadWriteByte(wdata[i]);
    }
}


void SPI1_Read(uint8_t *rdata,uint16_t datalen)
{
    uint16_t i;
    for(i=0;i<datalen;i++)
    {
        rdata[i] = SPI1_ReadWriteByte(0xff);
    }
}


