#ifndef __TCP_CLIENT_MANAGER_H__
#define __TCP_CLIENT_MANAGER_H__

#include <Arduino.h>
#include <WiFi.h>
#include "protocol.h"
#include "Device.h"
#include "Tools.h"

//定时任务相关
#define MAX_SCHEDULED_TASKS 20
#define OP7_INTERVAL_MS (10000UL) //每10秒请求一次定时任务列表
#define HEARTBEAT_INTERVAL_MS (10000UL) //每10秒发送一次心跳包
#define RECONNECT_INTERVAL_MS (5000UL) //每5秒尝试重连一次

class TcpClientManager
{
public:
    static TcpClientManager &getInstance();
    void begin(const char *serverIp = "47.112.116.248", uint16_t port = 20001);
    void beginLogUpload(const char *serverIp = "47.112.116.248", uint16_t port = 20011);

    //外部可调用的发送接口
    void sendPacket(uint8_t op, const void *payload, size_t payloadLen);

    bool ShouldExecute() {return this->shouldExecute;};

    void sendOp3Request();
    void sendLogLine(const char *logLine);

    bool isConnected() const;

    uint32_t getConnectAttemptCount() const {return this->connectAttemptCount_;}

    void resetCount() { connectAttemptCount_ = 0; }

private:
    bool shouldExecute = false;
    static uint8_t g_currentRelayState; //当前继电器状态
   
    void setRelayOutPut(uint8_t mask, uint8_t state);
    TcpClientManager() = default;
    ~TcpClientManager() = default;
    TcpClientManager(const TcpClientManager &) = delete;
    TcpClientManager &operator = (const TcpClientManager &) = delete;

    //FreeRTOS任务入口
    static void tcpTask(void *param);
    void run(); //主循环

    //连接与重连
    bool connectToServer();
    bool connectToLogServer();
    void handleReconnect(); //重连逻辑

    //协议包发送
    void sendLoginPacket(); //op1
    void sendHeartbeatPacket(); //op5
    void sendOp7Request(); //op7:请求定时任务

    //协议包接收与解析
    void handleIncomingData();  //从TCP读取原始字节
    void processPacket(const uint8_t *packet, size_t len);      //解析HEAD + payload


    //定时任务管理
    void addScheduledTask(const DEVTIMECTRLBACK &task);
    void removeScheduledTaskById(int id);
    void sendOp9Destroy(int id); //op9:销毁任务
    void checkAndExecuteScheduledTasks();

    //成员变量
    char serverIp_[16];
    uint16_t serverPort_ = 0;
    WiFiClient client_;

    char logServerIp_[16];
    uint16_t logServerPort_ = 0;
    WiFiClient logClient_;
    bool logUploadEnabled_ = false; //
    unsigned long lastLogConnectAttempt_ = 0;
    bool lastLogConnectedState_ = false;
    unsigned long lastLogWarnPrint_ = 0;
    
    SemaphoreHandle_t sendMutex_ = nullptr; // 发送互斥锁，保护client_的send操作
    bool shouldExecute = false; // 是否应该执行TCP任务，连接成功后置为true

    //定时任务缓存
    DEVTIMECTRLBACK scheduledTasks_[MAX_SCHEDULED_TASKS];
    uint8_t taskCount_ = 0; //当前定时任务数量

    //时间控制
    unsigned long lastHeartbeat_ = 0; //上次发送心跳的时间
    unsigned long lastOp7SendTime_ = 0; //上次发送op7请求的时间
    bool isConnected_ = false;

    //连接尝试次数
    uint32_t connectAttemptCount_ = 0; 


};

void writeRelayRegister(uint8_t state);

#endif