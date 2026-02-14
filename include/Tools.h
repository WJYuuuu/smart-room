#ifndef __TOOLS_H_
#define __TOOLS_H_

#include <Arduino.h>


class Tools { 
public:
    static SemaphoreHandle_t xMutex;    //定义一个互斥锁(是互斥锁还是信号量取决于怎么创建它)
    
    //!!!用来防止初次打印的时候,因为并发打印而导致的重复创建互斥锁,第一次打印的时候才会创建
    static void init();

    // 模板函数：打印字符串
    template <class T>
    static size_t myPrintf(T str);

    // 模板函数：打印字符串并换行
    template <class T> 
    static size_t myPrintln(T str);
};


// 初始化的时候直接创建,防止并发创建xMutex
void Tools::init() {
    if(xMutex == nullptr){
        xMutex = xSemaphoreCreateMutex();
    }
}

template <class T>
size_t Tools::myPrintf(T str) {

}

template <class T>
size_t Tools::myPrintln(T str) {
    //先检查互斥锁是否还没创建!
    //避免重复创建,第一次打印的时候才会创建
    if( xMutex == nullptr) {

    }
}

#endif
