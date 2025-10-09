/**
 * @file main.h
 * @brief STM32 Bootloader主程序头文件 - 定义系统常量和数据结构
 * @details 本文件定义了Bootloader系统中使用的所有常量、宏定义和数据结构，
 *          包括Flash分区布局、状态标志、OTA信息结构等
 * @author 无敌雪碧
 * @date 2025.10.1
 */

#ifndef _main_h_
#define _main_h_

// ==============================================================================
// STM32 Flash分区布局定义
// ==============================================================================
/**
 * @brief STM32内部Flash存储器布局配置
 * @details STM32F103C8T6具有64KB Flash，分为两个区域：
 *          - B区(Bootloader区)：存储Bootloader程序
 *          - A区(Application区)：存储用户应用程序
 */

#define STM32_FLASH_SADDR    0x08000000                                                     // STM32 Flash起始地址
#define STM32_PAGE_SIZE      1024                                                           // 每个Flash页大小(1KB)
#define STM32_PAGE_NUM       64                                                             // Flash总页数(64KB/1KB=64页)

// ==============================================================================
// 双分区布局定义
// ==============================================================================
/**
 * @brief 双分区Bootloader布局
 * @details 将64KB Flash分为两个区域：
 *          - B区：0x08000000-0x08004FFF (20KB，存储Bootloader)
 *          - A区：0x08005000-0x0800FFFF (44KB，存储应用程序)
 */

#define STM32_B_PAGE_NUM     20                                                             // Flash中B区总页数(20KB)
#define STM32_A_PAGE_NUM     STM32_PAGE_NUM - STM32_B_PAGE_NUM                              // Flash中A区总页数(44KB)
#define STM32_A_START_PAGE   STM32_B_PAGE_NUM                                               // Flash中A区起始页(第20页)
#define STM32_A_SADDR        STM32_FLASH_SADDR + STM32_A_START_PAGE * STM32_PAGE_SIZE       // Flash中A区起始地址(0x08005000)




#define IAP_XMODEMC_FLAG    0x00000002
#define IAP_XMODEMD_FLAG 	0x00000004
#define SET_CERSION_FLAG	0x00000008
#define CMD_5_FLAG			0x00000010
#define CMD_5_XMODEM_FLAG	0x00000020
#define CMD_6_FLAG			0x00000040

// ==============================================================================
// 系统状态标志定义
// ==============================================================================
/**
 * @brief 系统启动状态标志位定义
 * @details 使用位运算方式管理多个状态标志，提高效率
 */

#define UPDATA_A_FLAG      0x00000001                                                       // 更新A区标志位(位0)

// ==============================================================================
// OTA更新标志定义
// ==============================================================================
/**
 * @brief OTA更新状态标志
 * @details 用于标识是否需要执行OTA固件更新
 */

#define OTA_SET_FLAG      0xaabbccdd                                                       // OTA更新标志值(特殊值，避免误触发)

// ==============================================================================
// 数据结构大小定义
// ==============================================================================
/**
 * @brief 数据结构大小宏定义
 * @details 用于计算结构体大小，便于内存管理和数据校验
 */

#define OTA_INFOCB_SIZE  sizeof(OTA_InfoCB)                                                // OTA信息控制块大小

// ==============================================================================
// 数据结构定义
// ==============================================================================

/**
 * @brief OTA信息控制块结构体
 * @details 存储OTA更新相关的配置信息，包括更新标志和固件长度
 * @note 该结构体数据存储在AT24C02 EEPROM中，断电保持
 */
typedef struct 
{
    uint32_t  OTA_flag;                                                                     // OTA更新标志：0=正常启动，OTA_SET_FLAG=需要更新
    uint32_t  Firelen[11];      // 固件长度数组，存储每个W25Q64块中固件的字节数
                                // 索引0-10对应W25Q64的Block 0-10，每个块最大64KB
	uint8_t OTA_ver[32];		
}OTA_InfoCB;


/**
 * @brief A区更新控制块结构体
 * @details 用于控制A区固件更新过程，包含数据缓冲区和块号信息
 * @note 该结构体数据存储在RAM中，用于临时处理
 */
typedef struct 
{
    uint8_t  UpdataBuff[STM32_PAGE_SIZE];                                                  // 数据缓冲区，用于存储从W25Q64读取的固件数据
    uint32_t W25Q64_BlockNB;                                                               // 当前处理的W25Q64块号(0-10)
    uint32_t XmodemTimer;
    uint32_t XmodemNB;
    uint32_t XmodemCRC;	


}UpDataA_CB;



// ==============================================================================
// 全局变量声明
// ==============================================================================
/**
 * @brief 全局变量外部声明
 * @details 这些变量在main.c中定义，在其他文件中使用
 */

extern OTA_InfoCB OTA_Info;                                                                // OTA信息控制块全局变量
extern UpDataA_CB UpDataA;                                                                 // A区更新控制块全局变量
extern uint32_t  BootStaFlag;                                                              // 系统启动状态标志全局变量

#endif

