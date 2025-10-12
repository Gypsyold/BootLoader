#ifndef _boot_h_
#define _boot_h_

// 创建一个名为 load_a 的新类型：指向“无参数、无返回值函数”的指针。
// 用于保存应用程序的重置入口（向量表第2项），以便由Bootloader跳转执行。
typedef void (*load_a)(void);


/**
 * @brief Bootloader分支决策
 * @details 在有限时间窗口内检测用户是否希望进入命令行；
 *          若未进入且AT24C02中的OTA标志为置位，则设置搬运标志并留在Boot；
 *          否则直接跳转至A区应用。
 */
void BootLoader_Brance(void);

/**
 * @brief 加载并跳转到应用程序
 * @param addr 应用程序向量表基址（如 `STM32_A_SADDR`）
 * @note 将校验向量表中的初始MSP是否位于有效SRAM区间，再设置MSP并跳转。
 */
void LOAD_A(uint32_t addr);

/**
 * @brief 清理Bootloader初始化的外设
 * @note 跳转前调用，防止串口和GPIO配置“泄漏”到应用环境。
 */
void BootLoader_Clear(void);

/**
 * @brief 检测是否进入Boot命令行
 * @param timeout 以100ms为步进的计数（例如20代表约2秒）
 * @return 1=进入命令行；0=超时未进入
 */
uint8_t BootLoader_Enter(uint8_t timeout);

/**
 * @brief 打印Boot命令列表
 */
void BootLoader_Info(void);

/**
 * @brief 处理串口接收事件
 * @param data 数据指针
 * @param datalen 数据长度
 * @details 同时承担命令解析与XMODEM帧处理（根据状态位区分）。
 */
void BootLoader_Event(uint8_t *data,uint16_t datalen);

/**
 * @brief 计算XMODEM-CRC16校验
 * @param data 数据缓冲区
 * @param datalen 数据长度
 * @return 16位CRC
 */
uint16_t Xmodem_CRC16(uint8_t *data,uint16_t datalen);

#endif

