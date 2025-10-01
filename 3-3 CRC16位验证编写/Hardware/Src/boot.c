#include "stm32f10x.h"                  // Device header
#include "boot.h"
#include "usart.h"
#include "delay.h"
#include "AT24C02.h"
#include "W25Q64.h"
#include "main.h"

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
			BootStaFlag |= UPDATA_A_FLAG;
			UpDataA.W25Q64_BlockNB = 0;
			/* TODO: 此处应添加OTA更新的具体实现代码 */
			
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

	printf("[1]擦除A分区\r\n");
	printf("[2]串口IAP下载A区程序\r\n");
	printf("[3]设置OTA版本号\r\n");
	printf("[4]查询OTA版本号\r\n");
	printf("[5]向外部Flash下载程序\r\n");
	printf("[6]使用外部Flash内程序\r\n");
	printf("[7]重启系统\r\n");
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


void BootLoader_Event(uint8_t *data,uint16_t datalen)
{
    if((datalen == 1)&&(data[0] == '1'))
    {
		printf("擦除A分区\r\n");
		STM32_EraseFlash(STM32_A_START_PAGE,STM32_A_PAGE_NUM);
    }else if((datalen == 1)&&(data[0] == '7'))
    {
		printf("重启系统\r\n");
		Delay_ms(100);
		NVIC_SystemReset();
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


