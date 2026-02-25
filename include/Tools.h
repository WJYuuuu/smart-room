#ifndef __TOOLS_H__
#define __TOOLS_H__

#include <Arduino.h>
#include <time.h>
#include "protocol.h"
#include "Device.h"
#include <esp_timer.h>
#include "main.h"

#define LIGHT_PIN 15
#define AC_PIN 19
#define PLUG_PIN 18

extern bool isWiFiConnect; //WiFi连接状态.

class Tools {
public:
    static SemaphoreHandle_t xMutex;
    
    template <class T>
    static size_t myPrint(T str);

    template <class T>
    static size_t myPrintln(T str);
};

template <class T>
size_t Tools::myPrint(T str) {
    if(xMutex == nullptr) {
        xMutex = xSemaphoreCreateMutex();
    }
    size_t ret = -1;
    if (xSemaphoreTask(xMutex, pdMS_TO_TICKS(100) == pdTRUE)){
        //成功获取锁，操作临界区
        ret = Serial.print(str);
        TcpLogUpLoad_send(String(str));
        xSemaphoreGive(xMutex); // 释放锁;
    }
    return ret;
}

template <class T>
size_t Tools::myPrintln(T str) {
    if(xMutex == nullptr) {
        xMutex = xSemaphoreCreateMutex();
    }
    size_t ret = -1;
    if(xSemaphoreTake(xMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        ret = Serial.println(str);
        TcpLogUpLoad_send(String(str)+ "\n");
        xSemaphoreGive(xMutex); // 释放锁;
    }
    return ret;
}

int parseTimeToSeconds(const char *timeStr);
int getCurrentSecondsOfDay();
bool isInTimeWindow(int nowSec, int startSec, int endSec);



#endif