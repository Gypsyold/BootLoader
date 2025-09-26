#ifndef _uart_h_
#define _uart_h_

#include "stdarg.h"
#include "stdio.h"
#include "string.h"


#define U0_RX_SIZE 2048                  // 缓冲区最大容量
#define U0_TX_SIZE 2048                  // 每个数据最大容量
#define U0_RX_MAX 256                    // 一次数据最大容量
#define NUM 10                           // 管理数据变量的数量

typedef struct
{                         
    uint8_t *start;
    uint8_t *end;
}UCB_URxBuffptr;                        // 管理单个数据的结构体


typedef struct
{
    uint16_t URxCounter;                // 统计接收的数据量
    UCB_URxBuffptr URxDataPtr[NUM];     // 管理数据起始和终止指针的数组
    UCB_URxBuffptr *URxDataIN;          // 写入位置的指针
    UCB_URxBuffptr *URxDataOUT;         // 读取位置的指针
    UCB_URxBuffptr *URxDataEND;         // 缓冲区最后位置指针（不可以越界）


}UCB_CB;

extern UCB_CB U0CB;             
extern uint8_t U0_RxBuff[U0_RX_SIZE];   



void USART1_Init(uint32_t baudrate);
void U_DMA_Init(void);
void U0Rx_PtrInit(void);
void U0_printf(char * format, ...);

#endif

