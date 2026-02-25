/**
 * @author wangjingyu
 * @file protocol.cpp
 * @brief 协议相关工具函数与辅助功能实现
 *
 * ┌──────────────┬─────────────────────────────────────────────────────┐
 * │ 项目名称     │ 智慧终端通信协议栈                                    │
 * │ 版本         │ v0.0                                                 │
 * │ 作者         │ wangjingyu                                           │
 * │ 创建日期     │ 2026-2-25                                            │
 * │ 功能概述     │ 实现 protocol.h 中声明的工具函数。                     │
 * │ 依赖         │ C++11, iostream, cstdio, ctime                       │
 * └──────────────┴─────────────────────────────────────────────────────┘
 */

#include "protocol.h"
#include <iostream>
using namespace std;

/**
 * @brief 计算CRC16校验码
 * @param data 待校验数据指针
 * @param length 数据长度
 * @return uint16_t CRC16 校验结果
 */
uint16_t calculate_crc16(const uint8_t* data, size_t length) {
    uint16_t crc = 0xFFFF; // 初始值
    for(size_t i = 0; i < length; ++i) {
        crc ^= (uint16_t)data[i]; // 将当前字节与CRC进行异或
        for (int j=0; j<8; j++) {
            if(crc & 1) {
                crc = (crc >> 1) ^ CRC16_POLYNOMIAL;
            }
            else {
                crc >>= 1;
            }
        }
        
    }
    return crc;
}


/**
 * @brief 字符串转换为时间戳
 * @param str 输入的时间字符串，格式应为 "YYYY-MM-DD HH:MM:SS"
 * @return time_t 转换后的时间戳, 转换失败返回-1
 */
time_t string_to_timestamp(const char* str) {
    struct tm tm_info = { 0 };
    //sscanf会读取整数，年月日需手动调整
    if(sscanf(str, "%d-%d-%d %d-%d-%d",
        &tm_info.tm_year,
        &tm_info.tm_mon,
        &tm_info.tm_mday,
        &tm_info.tm_hour,
        &tm_info.tm_min,
        &tm_info.tm_sec) != 6) 
    {
         return -1; //解析失败
    }
    tm_info.tm_year -= 1900; // tm结构体中年份从1900开始
    tm_info.tm_mon -= 1;     // tm结构体中月份从0开始
    tm_info.tm_isdst = -1;   // 自动判断
    time_t result = mktime(&tm_info);
    return (result == (time_t)-1) ? -1 : result;
}
