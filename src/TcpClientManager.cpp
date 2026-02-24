#include "TcpClientManager.h"
#include "Log.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define MIN(a,b) (((a)<(b)) ? (a) : (b))

//constexpr:在编译时求值，用于定义常量和优化性能。
static constexpr uint8_t OP_LOG_UPLOAD = 11; //日志上传操作码

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
            unsigned long timeSinceLastAttempt = millis() - lastConnectAttempt; //距离上次连接尝试的时间
            unsigned long maxDelay = 60000UL; //最大重连间隔1分钟
            //计算当前重连间隔,随着尝试次数增加而增加,但不超过最大值
            /**
             * calculatedDelay:这个参数的意思是根据当前的连接尝试次数来计算下次连接尝试的延迟时间。它使用了指数退避算法，随着连接尝试次数的增加，延迟时间会成倍增加，但不会超过设定的最大值。
             * 具体来说，calculatedDelay的计算方式是将基础重连间隔（RECONNECT_INTERVAL_MS）乘以2的幂次方，其中幂次方的值是当前连接尝试次数（connectAttemptCount_）和5之间的较小值。这意味着：
             * - 当连接尝试次数较少时，calculatedDelay会快速增加（例如：5000ms, 10000ms, 20000ms...）。
             * - 当连接尝试次数达到5次或更多时，calculatedDelay将保持在最大值（例如：60000ms），避免过长的等待时间。
             * 这种机制有助于在网络不稳定时避免频繁的连接尝试，同时也确保在网络恢复后能够尽快重新连接服务器。
             * RECONNECT_INTERVAL_MS是基础重连间隔，connectAttemptCount_是当前的连接尝试次数，MIN函数确保指数增长不会超过5次，从而限制了最大重连间隔。
             * 1UL表示无符号长整型常量1，确保在计算过程中使用无符号类型，避免潜在的整数溢出问题。
             * 1UL << n表示将1左移n位，相当于计算2的n次方，这里用来实现指数增长的重连间隔。
             * connectAttemptCount_是一个计数器，记录当前已经尝试连接的次数，每次连接失败后会增加这个计数器，以便下次计算重连间隔时使用。
             * MIN函数用于确保指数增长不会超过5次，从而限制了最大重连间隔。
             */
            unsigned long calculatedDelay = RECONNECT_INTERVAL_MS * (1UL << MIN(connectAttemptCount_, 5U));
            int backoffDelay = (calculatedDelay > maxDelay) ? maxDelay : calculatedDelay; //最终的重连间隔，确保不超过最大值
            //如果距离上次连接尝试的时间已经超过了计算出的重连间隔,则进行连接尝试
            if(timeSinceLastAttempt >= backoffDelay) {
                connectAttemptCount_++; //增加连接尝试计数
                Tools::myPrintln("🔄 TCP Reconnection attempt #" + String(connectAttemptCount_) + ", delay: " + String(backoffDelay / 1000) + "s");

                if(connectToServer()) {
                    isConnected_ = true;
                    //成功连接,计数器清0
                    connectAttemptCount_ = 0;
                    lastHeartbeat_ = millis();
                    lastOp7SendTime_ = millis(); //首次连接立即请求任务
                    taskCount_ = 0;
                    sendLoginPacket(); //发送登录包(op1)

                    vTaskDelay(pdMS_TO_TICKS(100)); //确保登录相应到达
                    sendHeartbeatPacket();         //op5
                    vTaskDelay(pdMS_TO_TICKS(100));
                    sendOp7Request();             //op7:请求定时任务
                    lastConnectAttempt = millis(); //重置连接尝试时间
                }
                else {
                    lastConnectAttempt = millis(); //更新连接尝试时间
                    vTaskDelay(pdMS_TO_TICKS(100));
                    continue; //连接失败,继续等待下一次尝试
                }
            }
            else {
                //等待一段时间后再检查连接状态
                vTaskDelay(pdMS_TO_TICKS(500));
                continue;
            }
        }

        //发送心跳包
        if (millis() - lastHeartbeat_ > HEARTBEAT_INTERVAL_MS) {
            sendHeartbeatPacket();
            lastHeartbeat_ = millis();
        }

        //定时请求任务列表
        if(millis() - lastOp7SendTime_ > OP7_INTERVAL_MS) {
            sendOp7Request();
            lastOp7SendTime_ = millis();
        }

        // 执行本地缓存的定时任务(开灯,空调等)
        checkAndExecuteScheduledTasks();

        //处理接收到的数据
        handleIncomingData();

        //检查连接是否断开
        if(!client_.connected()) {
            Tools::myPrintln("💔 TCP connection lost. Reconnecting...");
            client_.stop();
            isConnected_ = false;
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }
        vTaskDelay(pdMS_TO_TICKS(10)); //短暂延迟，避免占用过多CPU
    }
}

/**
 * 连接到服务器
 */
bool TcpClientManager::connectToServer() {
    Tools::myPrintln("⚠️ LOG TCP connect failed: " + String(logServerIp_) + ":" + String(logServerPort_));
    if( xSemaphoreTake(wifiMutex_, pdMS_TO_TICKS(100)) != pdTRUE ) {
        Tools::myPrintln("❌ Failed to acquire WiFi mutex");
        return false;
    }

    //连接超时时间
    client_.setTimeout(15);

    bool connected = client_.connect(serverIp_, serverPort_);

    if(!connected) {
        Tools::myPrintln("❌ Failed to connect to TCP server.");
        Tools::myPrintln("💡 Server IP: " + String(serverIp_) + ", Port: " + String(serverPort_));
        xSemaphoreGive(wifiMutex_); // 用完必须释放！
        return false;
    }
    //设置TCP选项以提高稳定性
    client_.setNoDelay(true);
    client_.setTimeout(30); //设置接收超时为30s

    Tools::myPrintln("✅ Connected to TCP server.");
    xSemaphoreGive(wifiMutex_);
    return true;
}

bool TcpClientManager::connectToLogServer() {
    if(!logUploadEnabled_ || logServerPort_ == 0) {
        return false;
    }
    if(xSemaphoreTake(wifiMutex_, pdMS_TO_TICKS(100)) != pdTRUE) {
        return false;
    }
    logClient_.setTimeout(5);
    bool connected = logClient_.connect(logServerIp_, logServerPort_);
    if(connected) {
        logClient_.setNoDelay(true);
        logClient_.setTimeout(10);
    }
    else {
        unsigned long now = millis();
        if(now - lastLogWarnPrint_ > 10000UL) {
            lastLogWarnPrint_ = now;
            Tools::myPrintln("⚠️ LOG TCP connect failed: " + String(logServerIp_) + ":" + String(logServerPort_));
        }
    }
    xSemaphoreGive(wifiMutex_);
    return connected;
}

/**
 * 发送通用协议包(带HEAD头)
 */
void TcpClientManager::sendPacket(uint8_t op, const void *payload, size_t payloadLen) {
    if(!payload || payloadLen == 0)
        return;
    if(xSemaphoreTake(sendMutex_, pdMS_TO_TICKS(100)) != pdTRUE)
        return;
    if(client_.connected()) {
        //构造完整包:[HEAD][payload]
        char buffer[1024] = {0};
        HEAD *h = (HEAD*)buffer;
        h->len = payloadLen;
        h->td = 1; // 设备端发起
        h->op = op;
        memcpy(buffer+sizeof(HEAD), payload, payloadLen);
        h->crc = calculate_crc16((uint8_t *)(buffer + sizeof(HEAD)), payloadLen);

        //发送数据
        client_.write((uint8_t *)buffer, sizeof(HEAD) + payloadLen);
        if(op != OP_LOG_UPLOAD) {
            Tools::myPrintln("📤 Sent op" + String(op));
        }
    }
    xSemaphoreGive(sendMutex_);
}

/**
 * 发送日志行到日志服务器
 */
void TcpClientManager::sendLogLine(const char *logLine) {
    if(!logUploadEnabled_)
        return;
    if(!logLine)
        return;
    
    String line = String(logLine);
    if(line.length() == 0)
        return;
    
        int floor = Device::getInstance().getFloor();
        
}

/**
 * 发送登录包(op1)
 */
void TcpClientManager::sendLoginPacket() {

}

/**
 * 发送状态包(op3)
 */
void TcpClientManager::sendOp3Request() {

}

/**
 * 发送心跳包(op5)
 */
void TcpClientManager::sendHeartbeatPacket() {

}

/**
 * 发送请求定时任务包(op7)
 */
void TcpClientManager::sendOp7Request() {

}

/**
 * 从TCP流读取原始字节,按照包边界解析并处理协议包
 */
void TcpClientManager::handleIncomingData() {

}

/**
 * 根据op分发处理逻辑
 */
void TcpClientManager::processPacket(const uint8_t *fullPacket, size_t totalLen) {

}

/**
 * 添加定时任务
 */
void TcpClientManager::addScheduledTask(const DEVTIMECTRLBACK &task) {

}

/**
 * 移除指定ID任务
 */
void TcpClientManager::removeScheduledTaskById(int id) {

}

/**
 * 发送op9销毁任务
 */
void TcpClientManager::sendOp9Destroy(int id) {

}

/**
 * 检查并执行所有定时任务
 */
void TcpClientManager::checkAndExecuteScheduledTasks() {

}

/**
 * 重连后处理
 */
void TcpClientManager::handleReconnect() {

}

