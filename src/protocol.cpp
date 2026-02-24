
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
