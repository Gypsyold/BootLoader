#ifndef _boot_h_
#define _boot_h_

//创建一个名为 load_a 的新类型，该类型专门用于表示 “指向『无参数、无返回值函数』的指针
//typedef 把 void (*)(void) 这个 “函数指针的结构” 打包成了一个简洁的类型名 load_a
typedef void (*load_a)(void);


void BootLoader_Brance(void);
void LOAD_A(uint32_t addr);
void BootLoader_Clear(void);
uint8_t BootLoader_Enter(uint8_t timeout);
void BootLoader_Info(void);
void BootLoader_Event(uint8_t *data,uint16_t datalen);
uint16_t Xmodem_CRC16(uint8_t *data,uint16_t datalen);

#endif

