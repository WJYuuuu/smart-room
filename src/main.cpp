/**
 * 云茂智慧楼宇
 * @author WangJingYu
 * @date 2024-06-01
 * @description ESP32主程序，负责连接WIFI、与服务器通信等功能
 * @version 1.1
 */
 #include "main.h"
#include "Tools.h"
#include "Device.h"
#include <WiFi.h>

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
    Tools::init();    //初始化工具类,创建互斥锁
}

void loop() {

}

/**
 * @author WangJingYu
 */
bool connectWIFI() {
    Tools::myPrint("ESP32正在连接WIFI:");
    Tools::myPrint(ssid);
    WiFi.begin(ssid, password);

    while(WiFi.status() != WL_CONNECTED){
        Tools::myPrintln("ESP32连接WIFI成功!IP:" + WiFi.localIP().toString());
        
    }
    return true;
    
}
