#ifndef __TOOLS_H_
#define __TOOLS_H_

#include <Arduino.h>
#include <time.h>
#include "main.h"

class Tools { 
public:
    static SemaphoreHandle_t xMutex;    //定义一个互斥锁(是互斥锁还是信号量取决于怎么创建它)
    
    //!!!用来防止初次打印的时候,因为并发打印而导致的重复创建互斥锁,第一次打印的时候才会创建
    static void init();

    // 模板函数：打印字符串
    template <class T>
    static size_t myPrint(T str);

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
size_t Tools::myPrint(T str) {
    //互斥访问临界区
    size_t ret = -1;
    if(xSemaphoreTake(xMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        //获取到了互斥锁xMutex
        ret = Serial.print(str);
        
        //注意后期加入TCP LOG

        xSemaphoreGive(xMutex);
    }
    return ret;
}

template <class T>
size_t Tools::myPrintln(T str) {
    //互斥访问临界区
    size_t ret  = -1;
    if(xSemaphoreTake(xMutex, pdMS_TO_TICKS(100)) == pdTRUE){
        ret = Serial.println(str);

        //注意后期加入TCP LOG

        xSemaphoreGive(xMutex);    
    }
    return ret;
}

#endif
