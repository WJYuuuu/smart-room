/**
 * 云茂智慧楼宇
 * @author WangJingYu
 * @date 2024-06-01
 * @description ESP32主程序，负责连接WIFI、与服务器通信等功能
 * @version 1.1
 */
 #include "main.h"
#include "Tools.h"
#include "protocol.h"
#include "Device.h"
#include "MqttConnect.h"
#include <WiFi.h>

 //WIFI账号密码
 char ssid[20] = "BOLU";
 char password[30] = "BL86588330D34";

// 服务器IP地址和端口号:主服务端口和日志服务端口
uint16_t host_port = 20001;
uint16_t log_port = 20011;
char ServerIP[15] = "47.112.116.248"; //服务器IP地址

/**
 * WIFI连接声明
 */
bool connectWiFi();


/**
 * @brief WiFi操作的互斥信号量
 * @author WangJingYu
 * @date 2024-06-01
 * 
 * @note 需要调用xSemaphorCreateMutex()函数初始化之后才能使用xSemaphoreTake()和xSemaphoreGive()函数进行互斥访问控制
 */
SemaphoreHandle_t wifiMutex_ = nullptr;


void setup() {
    Tools::init();    //初始化工具类,创建互斥锁

    Serial.begin(115200); //设置波特率
    Tools::myPrintln("-------------------------------------------   设备启动   -------------------------------------------");

    //初始化wifi互斥锁
    if(wifiMutex_ == nullptr) {
        wifiMutex_ = xSemaphoreCreateMutex();
    }
    
    DeviceSetUp(); //初始化设备配置

    //引脚初始化
    pinMode(LIGHT_PIN, OUTPUT);     digitalWrite(LIGHT_PIN, 0); //初始设为低电平，即有人
    pinMode(AC_PIN, OUTPUT);        digitalWrite(AC_PIN, 0);
    pinMode(PLUGE_PIN, OUTPUT);     digitalWrite(PLUGE_PIN, 0);

    /**
     * @brief 同步继电器状态
     * 状态码：1：有人/开
     */
    Device::getInstance().setJdStaAt(LIGHT_INDEX, 1);
    Device::getInstance().setJdStaAt(AC_INDEX, 1);
    Device::getInstance().setJdStaAt(PLUGE_INDEX, 0);

    //连接WIFI,首次启动，等待稳定
    connectWiFi();      vTaskDelay(pdMS_TO_TICKS(2000));
    
    //mqtt配置，连接
    mqtt_begin();
}

void loop() {

}

/**
 * @author WangJingYu
 */
bool connectWiFi() {
    Tools::myPrint("ESP32正在连接WIFI:");
    Tools::myPrint(ssid);
    WiFi.begin(ssid, password);

    while(WiFi.status() != WL_CONNECTED){
        vTaskDelay(pdMS_TO_TICKS(500));
        Tools::myPrint(".");
    }
    if(WiFi.status() == WL_CONNECTED) {
        Tools::myPrintln("ESP32连接WIFI成功!IP:" + WiFi.localIP().toString());
        Device::getInstance().setIp(WiFi.localIP().toString().c_str());
        isWiFiConnect = true;
        return true;
    }
    isWiFiConnect = false;
    return false;   
}


/**
 * 初始化配置文件
 */
void DeviceSetUp() {
    Tools::myPrint("ssid:");
    Tools::myPrintln(ssid);
    Tools::myPrint("password:");
    Tools::myPrintln(password);
    Tools::myPrint("host_port:");
    Tools::myPrintln(host_port);
    // Tools::myPrint("mqtt_port:");
    // Tools::myPrintln(mqtt_port);
    Tools::myPrint("ServerIP:");
    Tools::myPrintln(ServerIP);
}