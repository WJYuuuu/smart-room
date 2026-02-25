/**
 * @author: wangjingyu
 * @file: protocol.h
 * @description: 定义通信相关的头文件
 */


#pragma once

#include <cstdint>
#include <cstdio>
#include <time.h>



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


//星期定义
// 星期定义
#define WEEK_MON (1 << 0)  // 1
#define WEEK_TUE (1 << 1)  // 2
#define WEEK_WED (1 << 2)  // 4
#define WEEK_THU (1 << 3)  // 8
#define WEEK_FRI (1 << 4)  // 16
#define WEEK_SAT (1 << 5)  // 32
#define WEEK_SUN (1 << 6)  // 64

// 执行类型
#define REPEAT_DISABLED     0
#define REPEAT_ONCE			1  // 单次（仅执行一次，通常配合具体日期，但你未提日期，可理解为“立即”或“当天”）
#define REPEAT_DAILY		2  // 每天
#define REPEAT_WEEKLY		3  // 每周（根据 week_mask）

/* ==================== 工具函数声明 ==================== */

/**
 * @brief 计算CRC16校验码
 * @param data 待校验数据指针
 * @param length 数据长度
 * @return uint16_t CRC16 校验结果
 */
uint16_t calculate_crc16(const uint8_t* data, size_t length);

/**
 * @brief 获取当前时间戳
 * @return time_t 当前时间戳
 */
time_t get_current_timestamp();

/**
 * @brief 将时间戳转换为字符串
 * @param timestamp 输入的时间戳
 * @param buffer 输出的字符串缓冲区
 * @param buffer_size 缓冲区大小
 * @return int 转换结果 0=成功, !0=失败
 */
int timestamp_to_string(time_t timestamp, char* buffer, size_t buffer_size);

/**
 * @brief 将时间字符串转换为时间戳
 * @param str 输入的时间字符串
 * @return time_t 转换后的时间戳, 转换失败返回-1
 */
time_t string_to_timestamp(const char* str);

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

/**
 * @brief 雷达状态结构体
 */
struct LDSTATUS{
    uint8_t ld_num; //当前雷达设备数量
    int8_t ld_sta[MAX_LD_NUM]; //
    char lid[MAX_LD_NUM][MSG_LENGTH]; //雷达ID字符串数组,每个ID最长24字符+1结尾符
};

/**
 * @brief 设备核心信息：描述一个房间的设备
 */
struct DEVINFO {
    char ip[16]; //设备IP地址字符串
    int8_t floor; //设备所在楼层
    char room[MSG_LENGTH]; //设备所在房间
    double eq;   //电量
    uint8_t jd_num; //继电器数量
    int8_t jd_sta[MAX_JD_NUM]; //继电器状态数组
    LDSTATUS ldinfo;    //雷达状态结构体
    int8_t enable;     //设备启用状态 0=禁用,1=启用
    double s_eq;  

};

/* ==================== 协议操作码（op）语义说明 ==================== */
/*
 * op | 方向        | 含义
 *----|-------------|-----------------------------------
 * 1  | 设备/客户端 → 服务器 | 登录请求
 * 2  | 服务器 → 设备/客户端 | 登录响应
 * 3  | 设备/客户端 → 服务器 | 心跳/状态上报
 * 4  | 服务器 → 设备/客户端 | 心跳/状态响应
 * 5  | 客户端 → 服务器      | 查询请求
 * 6  | 服务器 → 客户端      | 查询响应
 * 7  | 服务器 → 设备        | 获取定时控制指令请求
 * 8  | 服务器 → 设备        | 下发定时控制指令
 */


 /* ==================== 终端设备（td=1）通信结构体 ==================== */

 //设备主动登 op1
 struct DEVLOGIN {
    DEVINFO dev;   //设备完整信息
};

//服务器对op1登录请求的响应:op2
struct DEVLOGINBACK {
    int fd; //连接标识符,服务器分配
    double eq; 
};

//设备主动发送状态（op3）
struct DEVSTA {
    int fd; //连接标识符,服务器分配
    DEVINFO dev; //设备完整信息
};

//服务器对op3状态上报的响应:op4(对DEVSTA的写入响应)
struct DEVSTABACK {
    int ans; //响应结果 1=成功,0=失败
};

//终端发送心跳包 op5
struct DEVHEART {
    DEVINFO dev;   //携带部分简要信息
};

//服务器对心跳包的响应 op6
struct DEVHEARTBACK {
    int fd; //连接标识符,服务器分配
    uint8_t enable; //设备启用状态 0=禁用, !0 启用
    char nowtime[MSG_LENGTH]; //服务器当前时间字符串
};

//获取设置命令 op7
struct DEVTIMECTRL {
    int8_t floor; //目标楼层
    char room[MSG_LENGTH]; //目标房间
};

//定时控制指令 op8
struct DEVTIMECTRLBACK {
   int id;
   char room[MSG_LENGTH];
   int8_t floor;
   uint8_t jd_mask; //继电器掩码,表示要控制哪些继电器(1=控制,0=不控制)
   uint8_t jd_state;    //继电器状态,与掩码配合使用,表示要设置的状态(1=开,0=关)
   int t_type;      //触发类型 
   char start_time[9]; //起始时间
    char end_time[9];   //结束时间
    
    uint8_t week_mask;
    char date[MSG_LENGTH];;
};

//
struct DEVTIMECTRLBACK_ARR {
    int num; //
    DEVTIMECTRLBACK arr[BACK_SIZE];
};


 //销毁定时任务 op9
struct DEVDESTROY {
    int id; //要销毁的定时任务ID
};

//op10 
struct DEVDESTROYBACK {
    int res; //销毁结果 1=成功  2：失败
};

/* ==================== 电脑客户端（td=2）通信结构体 ==================== */



/* ==================== 辅助函数 ==================== */
