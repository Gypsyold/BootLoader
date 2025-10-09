#ifndef _AT24C02_H_
#define _AT24C02_H_

#define AT24C02_WADDR 0xA0
#define AT24C02_RADDR 0xA1



void AT24C02_Init(void);
uint8_t AT24C02_WriteByte(uint8_t addr,uint8_t wdata);
uint8_t AT24C02_WritePage(uint8_t addr,uint8_t *wdata);
uint8_t AT24C02_ReadData(uint8_t addr,uint8_t *rdata,uint16_t len);
void AT24C02_ReadOTAInfo(void);
void AT24C02_WriteOTAInfo(void);
#endif

