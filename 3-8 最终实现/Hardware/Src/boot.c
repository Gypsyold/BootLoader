#include "stm32f10x.h"                  // Device header
#include "boot.h"
#include "usart.h"
#include "delay.h"
#include "AT24C02.h"
#include "W25Q64.h"
#include "main.h"
#include "fm.h"

// ==============================================================================
// Bootloader内核：分支决策、命令行、XMODEM下载、外部仓库搬运、应用跳转
// 设计要点：
// - 启动阶段尽量少做动作，尽快决定“跳转应用”或“停留命令行”。
// - 跳转前清理外设状态，避免影响应用（串口、GPIO复位）。
// - XMODEM按128字节包接收，聚合至1KB页后落盘（内部或外部Flash）。
// - 命令5/6提供与外部仓库（W25Q64）的交互：下载到仓库、或从仓库搬运到A区。
// - OTA路径：上电读取AT24C02的标志，决定是否自动搬运并复位。
// ==============================================================================

/**
 * @brief 声明一个全局的函数指针变量。
 *
 * 在 LOAD_A 函数中，我们会将从向量表中读取到的应用程序入口地址赋值给它。
 * 之后，通过调用这个指针（如 load_A()），就可以跳转到应用程序执行。
 */
load_a load_A;

/**
 * @brief Bootloader分支选择函数
 * @note 此函数根据OTA标志决定系统启动流程，是进行OTA更新还是正常启动
 * @param  无
 * @retval 无
 */
void BootLoader_Brance(void)
{
    if(BootLoader_Enter(20) == 0)
	{
		/* 检查OTA标志位是否被设置 */
		if(OTA_Info.OTA_flag == OTA_SET_FLAG)
		{
			printf("%x\r\n",OTA_SET_FLAG);
			/* OTA标志已设置，执行OTA更新流程 */
			printf("OTA 更新\r\n");
            BootStaFlag |= UPDATA_A_FLAG;   // 设置标志位，触发外部仓库到A区的搬运流程
			UpDataA.W25Q64_BlockNB = 0;
            /* 具体搬运逻辑在 main.c 的主循环中执行（擦除+分页搬运+复位） */
			
		}else
		{
			/* OTA标志未设置，跳转到A分区正常启动 */
			printf("跳转A分区\r\n");
			LOAD_A(STM32_A_SADDR);
			
			
		}

	}
	
	printf("进入BootLoader命令行\r\n");
	BootLoader_Info();
}

void BootLoader_Info(void)
{

    printf("[1]擦除A分区\r\n");                 // 擦除内部Flash A区（20-63页）
    printf("[2]串口IAP下载A区程序\r\n");         // 通过XMODEM将程序直接写入A区
    printf("[3]设置OTA版本号\r\n");               // 写入AT24C02中的版本字符串
    printf("[4]查询OTA版本号\r\n");               // 读取并打印版本字符串
    printf("[5]向外部Flash下载程序\r\n");         // 通过XMODEM下载到W25Q64指定块
    printf("[6]使用外部Flash内程序\r\n");         // 从W25Q64指定块搬运至A区并复位
    printf("[7]重启系统\r\n");                   // 软复位
}

uint8_t BootLoader_Enter(uint8_t timeout)
{
    printf("2s内,输入小写字母'w',进入BootLoader命令行\r\n");
	while(timeout--)
	{
	    Delay_ms(100);
		if(U0_RxBuff[0] == 'w')
		{
			return 1;			//进入命令行
		}
	}
	return 0;
}


/**
 * @brief Bootloader事件处理函数 - 核心状态机
 * @param data 接收到的数据指针
 * @param datalen 数据长度
 * @details 根据当前状态标志位(BootStaFlag)处理不同类型的事件：
 *          - 空闲状态：处理命令1-7
 *          - XMODEM接收状态：处理数据包和EOT
 *          - 版本设置状态：处理版本号输入
 *          - 命令5状态：处理外部Flash块号选择
 *          - 命令6状态：处理外部Flash块号选择
 * @note 这是一个状态机函数，通过BootStaFlag的不同位组合来区分当前处理状态
 */
void BootLoader_Event(uint8_t *data,uint16_t datalen)
{
	int temp;  // 临时变量，用于版本号格式验证
	
	// ==========================================================================
	// 状态1：空闲状态 (BootStaFlag == 0) - 处理基本命令
	// ==========================================================================
	if(BootStaFlag == 0)
	{
		// 命令1：擦除A分区
		if((datalen == 1)&&(data[0] == '1'))
		{
			printf("擦除A分区\r\n");
			STM32_EraseFlash(STM32_A_START_PAGE,STM32_A_PAGE_NUM);  // 擦除第20-63页(44KB)
		}
		// 命令2：串口XMODEM下载到A区
		else if((datalen == 1)&&(data[0] == '2'))
		{
			printf("通过Xmodem协议,串口IAP下载A区程序,请使用bin格式文件\r\n");
			STM32_EraseFlash(STM32_A_START_PAGE,STM32_A_PAGE_NUM);  // 先擦除A区
			BootStaFlag |= (IAP_XMODEMC_FLAG | IAP_XMODEMD_FLAG);   // 设置XMODEM接收标志
			UpDataA.XmodemTimer = 0;  // 重置心跳定时器
			UpDataA.XmodemNB = 0;     // 重置包计数器
		}
		// 命令3：设置版本号
		else if((datalen == 1)&&(data[0] == '3'))
		{
			printf("设置版本号\r\n");
			BootStaFlag |= SET_CERSION_FLAG;  // 进入版本号输入状态
		}
		// 命令4：查询版本号
		else if((datalen == 1)&&(data[0] == '4'))
		{
			printf("查询版本号\r\n");
			AT24C02_ReadOTAInfo();  // 从EEPROM读取版本信息
			printf("版本号：%s\r\n",OTA_Info.OTA_ver);
			BootLoader_Info();  // 重新显示命令菜单
		}
		// 命令5：向外部Flash下载程序
		else if((datalen == 1)&&(data[0] == '5'))
		{
			printf("向外部Flash下载程序，输入需要使用的块编号（1-9）\r\n");
			BootStaFlag |= CMD_5_FLAG;  // 进入块号选择状态
		}
		// 命令6：使用外部Flash内程序
		else if((datalen == 1)&&(data[0] == '6'))
		{
			printf("使用外部Flash内的程序，输入需要使用的块编号（1-9）\r\n");
			BootStaFlag |= CMD_6_FLAG;  // 进入块号选择状态
		}
		// 命令7：重启系统
		else if((datalen == 1)&&(data[0] == '7'))
		{
			printf("重启系统\r\n");
			Delay_ms(100);  // 等待串口输出完成
			NVIC_SystemReset();  // 软复位
		}
	}
	// ==========================================================================
	// 状态2：XMODEM数据接收状态 (BootStaFlag & IAP_XMODEMD_FLAG) - 处理XMODEM协议
	// ==========================================================================
	else if(BootStaFlag & IAP_XMODEMD_FLAG)
	{
		// 处理XMODEM数据包：SOH(0x01) + 包号 + ~包号 + 128字节数据 + 2字节CRC
		if((datalen == 133)&&(data[0] == 0x01))
		{
			BootStaFlag &=~ IAP_XMODEMC_FLAG;  // 收到数据包，停止发送"C"字符
			
			// 计算接收数据的CRC16校验
			UpDataA.XmodemCRC = Xmodem_CRC16(&data[3],128);
			
			// 验证CRC校验：data[131]是高字节，data[132]是低字节
			if(UpDataA.XmodemCRC == data[131] * 256 + data[132])
			{
				UpDataA.XmodemNB++;  // 包计数器递增
				
				// 将128字节数据复制到缓冲区
				// 计算在缓冲区中的位置：(包号-1) % 8 * 128
				// 因为每8个包(8*128=1024字节)组成一个Flash页
				memcpy(&UpDataA.UpdataBuff[((UpDataA.XmodemNB - 1) % (STM32_PAGE_SIZE / 128)) *128],&data[3],128);
				
				// 检查是否已接收满一页(8个包 = 1024字节)
				if((UpDataA.XmodemNB % (STM32_PAGE_SIZE / 128)) == 0)
				{
					// 根据目标位置决定写入方式
					if(BootStaFlag & CMD_5_XMODEM_FLAG)
					{
						// 命令5：写入外部Flash(W25Q64)
						// W25Q64按256字节页写入，需要分4次写入
						for(uint8_t i = 0;i<4;i++)
						{
							W25Q64_Write_Page(&UpDataA.UpdataBuff[i*256],(UpDataA.XmodemNB / 8 - 1) * 4 + i + UpDataA.W25Q64_BlockNB * 64 *4);
						}
					}
					else 
					{
						// 命令2：写入内部Flash A区
						STM32_WriteFlash(STM32_A_SADDR + ((UpDataA.XmodemNB / (STM32_PAGE_SIZE / 128)) - 1) * STM32_PAGE_SIZE,(uint32_t *)UpDataA.UpdataBuff,STM32_PAGE_SIZE);
					}
				}
				
				printf("\x06\r\n");   // 发送ACK确认包
			}
			else
			{
				printf("\x15\r\n");   // 发送NAK重传请求
			}
		}
		
		// 处理EOT(End of Transmission) - 传输结束标志
		if((datalen == 1)&&(data[0] == 0x04))
		{
			printf("\x06\r\n");  // 发送ACK确认EOT
			
			// 处理最后不满一页的剩余数据
			if((UpDataA.XmodemNB % (STM32_PAGE_SIZE / 128)) != 0)
			{
				if(BootStaFlag & CMD_5_XMODEM_FLAG)
				{
					// 命令5：写入外部Flash剩余数据
					for(uint8_t i = 0;i<4;i++)
					{
						W25Q64_Write_Page(&UpDataA.UpdataBuff[i*256],(UpDataA.XmodemNB / 8 ) * 4 + i + UpDataA.W25Q64_BlockNB * 64 *4);
					}					
				}
				else 
				{
					// 命令2：写入内部Flash剩余数据
					STM32_WriteFlash(STM32_A_SADDR + ((UpDataA.XmodemNB / (STM32_PAGE_SIZE / 128)) ) * STM32_PAGE_SIZE,(uint32_t *)UpDataA.UpdataBuff,(UpDataA.XmodemNB % (STM32_PAGE_SIZE / 128)));
				}
			}
			
			// 清除XMODEM接收标志
			BootStaFlag &=~ IAP_XMODEMD_FLAG;
			
			// 根据传输目标进行后续处理
			if(BootStaFlag & CMD_5_XMODEM_FLAG)
			{
				// 命令5完成：更新固件长度信息并返回命令行
				BootStaFlag &=~ CMD_5_XMODEM_FLAG;
				OTA_Info.Firelen[UpDataA.W25Q64_BlockNB] = UpDataA.XmodemNB * 128;  // 记录固件长度
				AT24C02_WriteOTAInfo();  // 保存到EEPROM
				Delay_ms(100);
				BootLoader_Info();  // 返回命令菜单
			}
			else
			{
				// 命令2完成：重启系统让新固件生效
				Delay_ms(100);
				NVIC_SystemReset();
			}
		}
	}
	
	// ==========================================================================
	// 状态3：版本号设置状态 (BootStaFlag & SET_CERSION_FLAG) - 处理版本号输入
	// ==========================================================================
	else if(BootStaFlag & SET_CERSION_FLAG)
	{
		// 期望接收26字节的版本号字符串
		if(datalen == 26)
		{
			// 使用sscanf验证版本号格式：VER-x.y.z-YYYY/MM/DD-hh:mm
			// 格式示例：VER-1.2.3-2024/01/15-14:30
			if(sscanf((char *)data,"VER-%d.%d.%d-%d/%d/%d-%d:%d\r\n",&temp,&temp,&temp,&temp,&temp,&temp,&temp,&temp) == 8)
			{
				// 格式正确，保存版本号
				memset(OTA_Info.OTA_ver,0,32);  // 清空版本号缓冲区
				memcpy(OTA_Info.OTA_ver,data,26);  // 复制版本号字符串
				AT24C02_WriteOTAInfo();  // 保存到EEPROM
				printf("版本正确\r\n");
				BootStaFlag &=~ SET_CERSION_FLAG;  // 退出版本设置状态
				BootLoader_Info();  // 返回命令菜单
			}
			else	
			{
				printf("版本号格式错误\r\n");
			}
		}
		else
		{
			printf("版本号长度错误\r\n");
		}
	}
	
	// ==========================================================================
	// 状态4：命令5块号选择状态 (BootStaFlag & CMD_5_FLAG) - 选择外部Flash块
	// ==========================================================================
	else if(BootStaFlag & CMD_5_FLAG)
	{
		// 期望接收1字节的块号(1-9)
		if(datalen == 1)
		{
			// 检查块号是否在有效范围内(ASCII '1'到'9')
			if((data[0] > 0x31) &&(data[0] < 0x39) )
			{
				UpDataA.W25Q64_BlockNB = data[0] - 0x30;  // 转换为数字(1-8)
				
				// 设置XMODEM接收标志，目标为外部Flash
				BootStaFlag |= (IAP_XMODEMC_FLAG | IAP_XMODEMD_FLAG | CMD_5_XMODEM_FLAG);
				UpDataA.XmodemTimer = 0;  // 重置心跳定时器
				UpDataA.XmodemNB = 0;     // 重置包计数器
				
				// 清空该块的固件长度记录
				OTA_Info.Firelen[UpDataA.W25Q64_BlockNB] = 0;
				// 擦除W25Q64的指定64KB块
				W25Q64_Erase_Block_64K(UpDataA.W25Q64_BlockNB);
				
				printf("通过Xmodem协议,向外部Flash第%d个块下载程序,请使用bin格式文件\r\n",UpDataA.W25Q64_BlockNB);
				BootStaFlag &=~ CMD_5_FLAG;  // 退出块号选择状态
			}
			else  
			{
				printf("编号错误\r\n");
			}
		}
		else 
		{
			printf("数据长度错误\r\n");
		}
	}
	
	// ==========================================================================
	// 状态5：命令6块号选择状态 (BootStaFlag & CMD_6_FLAG) - 选择外部Flash块进行搬运
	// ==========================================================================
	else if(BootStaFlag & CMD_6_FLAG)
	{
		// 期望接收1字节的块号(1-9)
		if(datalen == 1)
		{
			// 检查块号是否在有效范围内(ASCII '1'到'9')
			if((data[0] > 0x31) &&(data[0] < 0x39) )
			{
				UpDataA.W25Q64_BlockNB = data[0] - 0x30;  // 转换为数字(1-8)
				
				// 设置A区更新标志，触发从外部Flash搬运到内部Flash的流程
				BootStaFlag |= UPDATA_A_FLAG;
				BootStaFlag &=~ CMD_6_FLAG;  // 退出块号选择状态
			}
			else  
			{
				printf("编号错误\r\n");
			}
		}
		else 
		{
			printf("数据长度错误\r\n");
		}
	}
}




// ==============================================================================
// 底层硬件操作函数 (使用内联汇编)
// ==============================================================================

/**
 * @brief 使用内联汇编实现的函数，用于设置处理器的主堆栈指针(MSP)。
 *
 * C语言无法直接访问特殊功能寄存器，因此需要用汇编指令来完成。
 * 这个函数通常在系统启动或任务切换时使用。
 *
 * @param addr 要设置给MSP的新堆栈指针地址。
 */
__asm void MSR_SP(uint32_t addr)
{
	// __asm 关键字告诉编译器，函数体是用汇编语言编写的。

    // ARM Cortex-M 的函数调用约定 (AAPCS) 规定，第一个参数通过 r0 寄存器传递。
    // 因此，函数参数 addr 的值此刻就存放在 r0 寄存器中。

    // MSR (Move to Special Register) 指令：将通用寄存器的值传送到特殊功能寄存器。
    // MSP (Main Stack Pointer) 是主堆栈指针寄存器。
    // 这条指令的作用是：将 r0 寄存器中的值（即 addr）写入到 MSP 寄存器中。
    // 执行后，CPU 的堆栈指针就被更新为新程序指定的地址。
    MSR MSP, r0
    
    // BX (Branch and Exchange) 指令：跳转并可能切换指令集。
    // r14 (Link Register, LR) 寄存器在函数调用时，保存了返回地址。
    // 这条指令的作用是：跳转到 LR 寄存器保存的地址，从而从当前函数返回。
    BX r14
}


// ==============================================================================
// 启动应用程序的核心函数
// ==============================================================================

/**
 * @brief 从指定地址加载并启动一个应用程序。
 *
 * 这是 Bootloader 的核心功能函数。它会读取应用程序的向量表，
 * 检查其有效性，然后设置新的堆栈指针并跳转到应用程序的复位处理函数。
 *
 * @param addr 应用程序在内存中的起始地址（即其向量表的基地址）。
 */
void LOAD_A(uint32_t addr)
{
	// ==========================================================================
	// 1. 安全检查：验证堆栈指针是否有效
	// ==========================================================================
	
	// *(uint32_t *) addr:
	//   1. (uint32_t *) addr: 将传入的地址 addr 强制转换为一个指向 32位整数的指针。
	//   2. *(...): 解引用该指针，获取 addr 地址上存放的第一个 32位数值。
	//
	// 在 ARM Cortex-M 的向量表中，第一个元素永远是“初始堆栈指针(Initial Stack Pointer)”。
	//
	// if ((... >= 0x20000000) && (... <= 0x20004FFF)):
	//   检查获取到的堆栈指针值是否落在 SRAM（内存）的有效地址范围内。
	//   这是一种简单的有效性校验，确保 addr 指向的是一个可能有效的应用程序。
	if((*(uint32_t *) addr >= 0x20000000) && (*(uint32_t *) addr <= 0x20004FFF))
	{
		// ======================================================================
		// 2. 设置新程序的堆栈指针
		// ======================================================================
		
		// 调用我们上面定义的汇编函数，将应用程序的初始堆栈指针值写入 CPU 的 MSP 寄存器。
		// 这一步是在为切换到新程序运行做准备。
		MSR_SP(*(uint32_t *) addr);
		
		// ======================================================================
		// 3. 获取并跳转到新程序的入口点（复位向量）
		// ======================================================================
		
		// *(uint32_t *) (addr + 4):
		//   1. addr + 4: 移动到向量表的第二个元素,即复位向量的地址
		//   2. *(uint32_t *) (...): 获取该地址上的 32位数值。
		//
		// 在向量表中，第二个元素是“复位向量(Reset Vector)”，它存放的是应用程序
		// 复位处理函数 (Reset_Handler) 的地址。
		//
		// (load_a): 将获取到的函数地址（一个整数）强制转换为我们定义的函数指针类型。
		//
		// load_A = ...: 将这个函数指针赋值给全局变量 load_A。
		// 现在，load_A 就指向了应用程序的起点。
		load_A = (load_a)*(uint32_t *) (addr + 4);
	
		// ======================================================================
		// 4. 复位操作
		// ======================================================================	


		BootLoader_Clear();

		// ======================================================================
		// 5. 执行跳转
		// ======================================================================
		
		// 像调用一个普通函数一样调用函数指针。
		// 这行代码会导致 CPU 跳转到 load_A 所指向的地址去执行。
		// 即跳转到应用程序的 Reset_Handler 函数。
		// 从这一刻起，Bootloader 的使命完成，应用程序开始运行。
		load_A();
	}else{
	    printf("跳转A分区失败\r\n");

	}
}



/**
 * @brief 清除Bootloader相关外设初始化
 * @note 此函数用于在Bootloader执行完毕后，关闭已初始化的外设，释放资源
 *       主要包括USART1和GPIOA、GPIOB的配置清除，为后续应用程序运行做准备
 * @param  无
 * @retval 无
 */
void BootLoader_Clear(void)
{
    /* 关闭USART1外设，清除其配置 */
	USART_DeInit(USART1);
	
	/* 复位GPIOA所有引脚到默认状态 */
	GPIO_DeInit(GPIOA);
	
	/* 复位GPIOB所有引脚到默认状态 */
	GPIO_DeInit(GPIOB);
}





uint16_t Xmodem_CRC16(uint8_t *data,uint16_t datalen)
{
    uint16_t Crcinit = 0x0000;		// 初始值
	uint16_t Crcipoly = 0x1021;		// 多项式

	uint8_t i;

	while(datalen--)
	{
		Crcinit = (*data << 8) ^ Crcinit;		// 将数据左移8位与初始值异或
		for(i = 0; i < 8; i++)
		{
			if(Crcinit & 0x8000)		// 检查最高位是否为1
			{
				Crcinit = (Crcinit << 1) ^ Crcipoly;		// 如果为1，则左移1位并与多项式异或
			}else
			{
			    Crcinit <<= 1;		// 如果为0，则左移1位
			}


		}

		data++;		// 移动到下一个数据字节


	}

	return Crcinit;		// 返回计算得到的CRC值
}


