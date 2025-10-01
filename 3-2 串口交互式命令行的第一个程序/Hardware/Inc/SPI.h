#ifndef _SPI_h_
#define _SPI_h_

void My_SPI_Init(void);
uint8_t SPI1_ReadWriteByte(uint8_t txd);
void SPI1_Read(uint8_t *rdata,uint16_t datalen);
void SPI1_Write(uint8_t *wdata,uint16_t datalen);
void MySPI_W_SS(uint8_t BitValue);



#endif

