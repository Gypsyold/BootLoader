#include "stm32f10x.h"                  // Device header
#include "usart.h"

UCB_CB U0CB;
uint8_t U0_RxBuff[U0_RX_SIZE];       
uint8_t U0_TxBuff[U0_TX_SIZE];

void USART1_Init(uint32_t baudrate)
{ 
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);
 
    USART_InitTypeDef USART_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = baudrate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);

    USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);					    	// 使能USART1的DMA接收
                                                // 使能USART1


    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);                         // 设置优先级分组为2
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;               // 抢占优先级为0
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;                      // 子优先级为0
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                         // 使能中断
    NVIC_Init(&NVIC_InitStructure);                                         // 初始化NVIC

    USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);                          // 使能USART1的空闲中断

    U0Rx_PtrInit();
    U_DMA_Init();
    USART_Cmd(USART1, ENABLE);  
}


void U_DMA_Init(void)
{
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);


    DMA_InitTypeDef DMA_InitStructure;
    DMA_DeInit(DMA1_Channel5);  											// USART1_RX使用DMA1通道5
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR;		// 外设地址
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)U0_RxBuff;		        // 内存地址
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;					    // 外设为数据源
    DMA_InitStructure.DMA_BufferSize = U0_RX_MAX + 1;				        // 单次数据（DMA的 DMA_BufferSize 参数控制的是单次DMA传输能处理的数据量）+1通过增加1个字节，可以确保写指针永远不会追上读指针
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;		// 外设地址不增
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;				    // 内存地址增
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;	// 数据宽度为8位
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;		    // 数据宽度为8位
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;					    	// 循环模式
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;				    // 中间优先级
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;							// 禁止内存到内存传输
    DMA_Init(DMA1_Channel5, &DMA_InitStructure);

    DMA_Cmd(DMA1_Channel5, ENABLE);                                         // 使能DMA1通道5
}


void U0Rx_PtrInit(void)
{
    U0CB.URxDataIN  = &U0CB.URxDataPtr[0];
    U0CB.URxDataOUT = &U0CB.URxDataPtr[0];
    U0CB.URxDataEND = &U0CB.URxDataPtr[NUM - 1];
    U0CB.URxDataIN -> start = U0_RxBuff;
    U0CB.URxCounter = 0;

}



void USART1_IRQHandler(void)
{

    if (USART_GetITStatus(USART1, USART_IT_IDLE) != RESET)  //产生空闲中断
    {
        // 软件读取SR寄存器，DR寄存器来清除中断标志位
        USART_GetFlagStatus(USART1,USART_FLAG_IDLE);    // 读取SR寄存器
        USART_ReceiveData(USART1);                      // 读取DR寄存器
        
        U0CB.URxCounter += U0_RX_SIZE - DMA_GetCurrDataCounter(DMA1_Channel5);  // 计算接收到的数据长度(DMA总量 - DMA剩余的量)
        U0CB.URxDataIN->end = &U0_RxBuff[U0CB.URxCounter - 1];
        U0CB.URxDataIN ++;                              // 指针写入的时候指向下一个数据包

        if(U0CB.URxDataIN == U0CB.URxDataEND)           // 如果写入指针指向了最后一个数据包，则指向第一个数据包
        {
            U0CB.URxDataIN = &U0CB.URxDataPtr[0];
        }

        if(U0_RX_SIZE - U0CB.URxCounter >= U0_RX_MAX)                // 如果缓冲区剩余的量足够下一个单次传输最大量
        {
            U0CB.URxDataIN -> start = &U0_RxBuff[U0CB.URxCounter];   // 写入的起始地址就等于累积的缓冲区地址

        }
        else{

            U0CB.URxDataIN -> start = U0_RxBuff;                     // 如果不够下一次单次传输最大量，则回到缓冲区起始地址                      
            U0CB.URxCounter = 0;                                     // 收到的数据长度也重新归零       
        }

        DMA_Cmd(DMA1_Channel5, DISABLE);  // 关闭DMA
        DMA_SetCurrDataCounter(DMA1_Channel5, U0_RX_MAX + 1); 

        DMA_Cmd(DMA1_Channel5, ENABLE);  // 重新开启DMA

    }

}



void U0_printf(char * format, ...)
{
    uint16_t i;
    va_list listdata;
    va_start(listdata, format);
    vsprintf((char *)U0_TxBuff, format, listdata);
    va_end(listdata);

    for(i = 0; i < strlen((char *)U0_TxBuff); i++)
    { 
        while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) != SET);
        USART_SendData(USART1, U0_TxBuff[i]);

    }

     while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) != SET);

}

