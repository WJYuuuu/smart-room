#include "TcpClientManager.h"
#include "Log.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define MIN(a,b) (((a)<(b)) ? (a) : (b))

extern bool isWiFiConnect; //来自Tools, 表示WiFi是否已连接

/*
静态函数:
    与类绑定而不是与对象绑定
    只能操作静态变量
    没有this指针的相关操作,因为它不依赖于任何对象实例
    可以通过类名直接调用,不需要创建对象实例
例如:学生类
    静态变量可以是:学生总数
    静态函数可以是:统计学生总数
*/

/**
 * 单例实现
 * @note 由于FreeRTOS任务函数必须是静态的，因此TcpClientManager设计为单例模式，所有实例方法通过getInstance()访问。
 */
TcpClientManager &TcpClientManager::getInstance() {
    static TcpClientManager instance;
    return instance;
}

/**
 * TCP客户端主任务入口
 * 连接服务器后进入主循环，处理发送和接收逻辑
 */
void TcpClientManager::begin(const char *serverIp, uint16_t port) {
    if (!serverIp)
        return;
    strncpy(serverIp_, serverIp, sizeof(serverIp_));
    serverIp_[sizeof(serverIp_)-1] = '\0';
    serverPort_ = port;

    //创建互斥锁
    if(sendMutex_ == nullptr) {
        sendMutex_ = xSemaphoreCreateMutex();
    }

    //启动独立任务处理TCP逻辑
    xTaskCreate(tcpTask, "TcpClientTask", 8192, this, 4, nullptr);
}

/**
 * @brief 启动日志上传功能，连接到日志服务器
 */
void TcpClientManager::beginLogUpload(const char *serverIp, uint16_t port) {
    if(!serverIp)
        return;

    strncpy(logServerIp_, serverIp, sizeof(logServerIp_)-1);
    logServerIp_[sizeof(logServerIp_)-1] = '\0';
    logServerPort_ = port;
    logUploadEnabled_ = true;
    lastLogConnectAttempt_ = 0; //重置日志服务器连接尝试计数
    lastLogConnectedState_ = false; //重置日志服务器连接状态
    lastLogWarnPrint_ = 0; //重置日志服务器警告打印时间
}

/**
 * @brief FreeRTOS任务入口
 */
void TcpClientManager::tcpTask(void* param) {
    TcpClientManager *self = static_cast<TcpClientManager *>(param);
    self->run();
    vTaskDelete(nullptr); //任务结束，删除自己
}
/**
 * 主循环，处理连接、发送和接收逻辑 任务调度
 */
void TcpClientManager::run() {
    // 初始化重连计数器
    unsigned long lastConnectAttempt = millis();
    while(true) {
        //如果 WiFi 未连接,等待
        if(!isWiFiConnect)
        {
            vTaskDelay(pdMS_TO_TICKS(1000));
            isConnected_ = false;
            if(client_.connected()) {
                client_.stop();
            }
            if(logClient_.connected()) {
                logClient_.stop();
            }
            continue;
        }
        if(logUploadEnabled_ && !logClient_.connected()) {
            // 尝试连接日志服务器
            unsigned long now = millis();
            if(now - lastLogConnectAttempt_ >= 5000UL) {
                lastLogConnectAttempt_ = now;
                connectToLogServer();
            }
        }
        //如果未连接服务器,尝试连接
        if(!isConnected_){
            //实现指数退避重连机制
        }


    }
}


//外部可调用的发送接口
void sendPacket(uint8_t op, const void *payload, size_t payloadLen);

bool ShouldExecute() {return this->shouldExecute;};

void sendOp3Request();
void sendLogLine(const char *logLine);

bool isConnected() const;

uint32_t getConnectAttemptCount() const {return this->connectAttemptCount_;}

void resetCount() { connectAttemptCount_ = 0; }
