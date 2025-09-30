#ifndef _boot_h_
#define _boot_h_

//创建一个名为 load_a 的新类型，该类型专门用于表示 “指向『无参数、无返回值函数』的指针
//typedef 把 void (*)(void) 这个 “函数指针的结构” 打包成了一个简洁的类型名 load_a
typedef void (*load_a)(void);


void BootLoader_Brance(void);
void LOAD_A(uint32_t addr);
void BootLoader_Clear(void);

#endif

