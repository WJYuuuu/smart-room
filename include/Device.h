#ifndef __DEVICE_H__
#define __DEVICE_H__

#include <Arduino.h>
#include "protocol.h"
#include "Tools.h"
#include "HeartBag.h"
#include "string.h"

class Device {
public:
    static Device& getInstance();
    ~Device();

    //ip地址
    const char *getIp() const;
    void setIp(const char *newIp);
    //楼层
    int8_t getFloor() const;
    void setFloor(int8_t newFloor);
    //房间号
    const char *getRoom() const;
    void setRoom(const char *newRoom);
    //电量
    double getEq() const;
    void setEq(double newEq);
    void accEq(double v);

    //继电器数量
    uint8_t getJdNum() const;
    void setJdNum(uint8_t num);

    //继电器状态(整体数组)
    const int8_t* getJdSta() const;
    void setJdSta(const int8_t* sta, size_t len);//安全写入

    //单个继电器状态,按照索引
    int8_t getJdStaAt(size_t index) const;
    void setJdStaAt(size_t index, int8_t value);

    //雷达数量
    uint8_t getLdNum() const;
    void setLdNum(uint8_t num);
    void plusLdNum();

    //雷达状态(整体数组)
    const int8_t* getLdSta() const;
    void setLdSta(const int8_t* sta, size_t len);

    //单个雷达状态,按照索引
    int8_t getLdStaAt(size_t index) const;
    void setLdStaAt(size_t index, int8_t value);

    //
    int8_t getEnable() const;
    void setEnable(int8_t value);

    //雷达ID数组  整体
    const char* getLidAt(size_t index) const;
    void setLidAt(size_t index, const char* lid);


    //
    int getFd() const;
    void setFd(int fd);

    double getSeq() const;
    void setSeq(double v);

    void getDev(DEVINFO *p);

    void incrementLightOnCount();
    uint32_t getLightOnCount() const { return lightOnCount; }
    void getCurrentDateString(char *buf, size_t len) const;

private:
    Device();
    char ip[16]; //存储IP地址的字符串，最大长度为15字符 + 1个终止符
    int8_t floor; //楼层
    char room[MSG_LENGTH]; //房间号
    double eq;      //电量
    uint8_t jd_num; //继电器数量
    int8_t jd_sta[MAX_JD_NUM]; //继电器状态数组
    uint8_t ld_num; //雷达数量
    int8_t ld_sta[MAX_LD_NUM]; //雷达状态数组
    char lid[MAX_LD_NUM][MSG_LENGTH]; //雷达ID数组

    int8_t enable;
    double s_eq;
    int fd;
    SemaphoreHandle_t dMutex; 

    uint32_t lightOnCount = 0;   //累计开启次数
};

#endif