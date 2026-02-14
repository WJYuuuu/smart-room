/**
 * 2026/2/14 WangJingYu   云茂智慧楼宇
 */
 #include "main.h"

 //WIFI账号密码
 char ssid[20] = "BOLU";
 char password[30] = "BL86588330D34";

// 服务器IP地址和端口号:主服务端口和日志服务端口
uint16_t host_port = 20001;
uint16_t log_port = 20011;
char ServerIP[15] = "47.112.116.248";

/**
 * WIFI连接
 */
bool connectWIFI();


void setup() {

}

void loop() {

}

bool connectWIFI(){
    bool tag = false;

    return tag;
}
