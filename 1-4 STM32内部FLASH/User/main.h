#ifndef _main_h_
#define _main_h_




#define STM32_FLASH_SADDR    0x08000000                                                     //Flash起始地址
#define STM32_PAGE_SIZE      1024                                                           //每个Flash页大小
#define STM32_PAGE_NUM       64                                                             //Flash总页数
#define STM32_B_PAGE_NUM     20                                                             //Flash中B区总页数
#define STM32_A_PAGE_NUM     STM32_PAGE_NUM - STM32_B_PAGE_NUM                              //Flash中A区总页数
#define STM32_A_START_PAGE   STM32_B_PAGE_NUM                                               //Flash中A区起始页
#define STM32_A_SADDR        STM32_FLASH_SADDR + STM32_A_START_PAGE * STM32_PAGE_SIZE       //Flash中A区起始地址


#define OTA_SET_FLAG      0x03020101

#define OTA_INFOCB_SIZE  sizeof(OTA_InfoCB)

typedef struct 
{
    uint32_t  OTA_flag;
    uint32_t  Firelen[11];      //0号成员固定对应OAT大小


}OTA_InfoCB;


typedef struct 
{
    uint8_t  UpdataBuff[STM32_PAGE_SIZE];
    uint32_t W25Q64_BlockNB;
    

}UpDataA_CB;



extern OTA_InfoCB OTA_Info;
extern UpDataA_CB UpDataA;


#endif

