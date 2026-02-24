/**
 * @author: wangjingyu
 * @file: protocol.h
 * @description: 定义通信相关的头文件
 */


#pragma once

#include <cstdint>
#include <cstdio>
#include <time.h>


// 定时控制指令 (服务器下发) (op = 8)
struct DEVTIMECTRLBACK {

};

/* ==================== 宏定义 ==================== */
#define CRC16_POLYNOMIAL 0x8005 // CRC16-IBM (也称为 CRC-16-ANSI) 的多项式

/* ==================== 工具函数声明 ==================== */

/**
 * @brief 计算CRC16校验码
 * @param data 待校验数据指针
 * @param length 数据长度
 * @return uint16_t CRC16 校验结果
 */
uint16_t calculate_crc16(const uint8_t* data, size_t length);


/**
 * @author wangjingyu
 * @brief 所有通信包的头部结构
 * - op:奇数表示请求,偶数表示相应,op+1为对应响应
 * - td=1 表示终端设备，td=2 表示电脑客户端
 * 
 */
struct HEAD {
    int op;     //操作类型(1-14)
    int td;     //发送方类型:1 = 终端设备, 2=客户端软件
    int len;    //数据体长度(不含HEAD)
    int crc;    //CRC效验码(仅校验数据体)
};
