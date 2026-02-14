#ifndef __DEVICE_H_
#define __DEVICE_H_

#include <Arduino.h>
#include "Tools.h"
#include "protocol.h"

class Device {
public:

    //单例模式创建,保证全局只有一个实例
    static Device &getInstance();//Device & 表示函数返回的是一个Device的引用,不是拷贝

    //析构函数,当对象被销毁时调用!
    ~Device();

    //IP管理
    //常成员函数(方法不会修改对象成员) + 只读返回值
    const char *getIp() const; 
    void setIp(const char *newIp);

    //楼层
    int8_t getFloor() const;
    void setFloor(int8_t newFloor);

    //房间号
    const char *getRoom() const;
    void setRoom(const char *newRoom);

    //继电器数量

    //继电器状态

    //单个继电器状态(按照索引)


    //雷达状态数组(按照索引)


    //单个雷达状态(按照索引)

    //雷达ID数组


private:
    //构造函数,当每次创建新的类时调用!
    Device();
    char ip[16];
    int8_t floor;
    char room[MSG_LENGTH];

   static  SemaphoreHandle_t dMutex;   //防止多个任务同时读写Device成员造成脏数据

};

#endif
