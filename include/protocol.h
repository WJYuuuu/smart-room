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
#define MAX_LD_NUM 10 //最大雷达设备数量 设备
#define MAX_JD_NUM 3 //最大继电器设备数量
#define MSG_LENGTH 25  //字符串最大长度
#define BACK_SIZE  5    //批量查询返回数组大小

//集电极索引常量  jd_sta数组
#define LIGHT_INDEX 0 //灯光继电器索引
#define AC_INDEX    1 //空调继电器索引
#define PLUG_INDEX  2 //插座继电器索引


/* ==================== 工具函数声明 ==================== */

/**
 * @brief 计算CRC16校验码
 * @param data 待校验数据指针
 * @param length 数据长度
 * @return uint16_t CRC16 校验结果
 */
uint16_t calculate_crc16(const uint8_t* data, size_t length);



/* ==================== 协议公共头部 ==================== */

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


/* ==================== 子结构体定义 ==================== */



/* ==================== 协议操作码（op）语义说明 ==================== */



 /* ==================== 终端设备（td=1）通信结构体 ==================== */



/* ==================== 电脑客户端（td=2）通信结构体 ==================== */



/* ==================== 辅助函数 ==================== */
