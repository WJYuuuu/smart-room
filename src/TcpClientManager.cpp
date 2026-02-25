#include "TcpClientManager.h"
#include "Log.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define MIN(a,b) (((a)<(b)) ? (a) : (b))

uint8_t TcpClientManager::g_currentRelayState = 0x00;
const uint8_t RELAY_PINS[3] = {LIGHT_PIN, AC_PIN, PLUG_PIN};
static constexpr uint8_t SCHEDULE_ALLOWED_MASK = 0x00; // 继电器由雷达逻辑接管，禁用定时任务写继电器


//constexpr:在编译时求值，用于定义常量和优化性能。
static constexpr uint8_t OP_LOG_UPLOAD = 11; //日志上传操作码

extern bool isWiFiConnect; //来自Tools, 表示WiFi是否已连接

char nowTime[MSG_LENGTH];

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
    const char *room = Device::getInstance().getRoom();
    String roomStr = (room && strlen(room) > 0) ? String(room) : String("Unknown");
    
    String taggedLine = "floor=" + String(floor) + ";room=" + roomStr +";msg=" + line;

    const size_t maxPayload = 900; //留出HEAD和CRC的空间
    if(taggedLine.length() > maxPayload) {
        taggedLine = taggedLine.substring(0, maxPayload);
    }
    if(!isWiFiConnect || !logClient_.connected()) 
        return;
    if(xSemaphoreTake(sendMutex_, 0) != pdTRUE)
        return;

    char buffer[1024] = {0};
    HEAD *h = (HEAD *)buffer;
    h->len = taggedLine.length();
    h->td = 1; //设备端发起
    h->op = OP_LOG_UPLOAD;
    memcpy(buffer + sizeof(HEAD), taggedLine.c_str(), taggedLine.length());
    h->crc = calculate_crc16((uint8_t *)(buffer + sizeof(HEAD)), taggedLine.length());
    logClient_.write((uint8_t *)buffer, sizeof(HEAD)+taggedLine.length());
    xSemaphoreGive(sendMutex_);
}

/**
 * 发送登录包(op1)
 */
void TcpClientManager::sendLoginPacket() {
    DEVLOGIN devLogin = {};
    Device::getInstance().getDev(&devLogin.dev);
    sendPacket(1, &devLogin, sizeof(DEVLOGIN));
}

/**
 * 发送状态包(op3)
 */
void TcpClientManager::sendOp3Request() {
    DEVSTA d = {};
    Device::getInstance().getDev(&d.dev);
    sendPacket(3, &d, sizeof(DEVSTA));
}

/**
 * 发送心跳包(op5)
 */
void TcpClientManager::sendHeartbeatPacket() {
    DEVHEART devHeart = {};
    Device::getInstance().getDev(&devHeart.dev);
    sendPacket(5, &devHeart, sizeof(DEVHEART));
}

/**
 * 发送请求定时任务包(op7)
 */
void TcpClientManager::sendOp7Request() {
    DEVTIMECTRL req = {};
    strcpy(req.room, Device::getInstance().getRoom());
    req.floor = Device::getInstance().getFloor();
    sendPacket(7, &req, sizeof(DEVTIMECTRL));
};

/**
 * 从TCP流读取原始字节,按照包边界解析并处理协议包
 */
void TcpClientManager::handleIncomingData() {
    while(client_.available() >= sizeof(HEAD)) {
        uint8_t data[2048] = {0};
        HEAD *head = (HEAD*)data;
        uint8_t *payload = data + sizeof(HEAD);

        //读取HEAD
        client_.readBytes(data, sizeof(HEAD));
        //检查长度是否合理
        if(head->len > 1000 || head->len == 0){
            Tools::myPrintln("📥 Invalid packet length");
            continue;
        }

        //读消息体 payload
        size_t readLen = client_.readBytes(payload, head->len);
        if(readLen != head->len) {
            continue; //读取不完整,丢弃
        }

        //校验CRC
        uint16_t calcCrc = calculate_crc16(payload, head->len);
        if(calcCrc != head->crc) {
            Tools::myPrintln("📥 CRC error on op" + String(head->op));
            continue;
        }

        //处理完整协议包
        processPacket(data, sizeof(HEAD) + head->len);
    }
}

/**
 * 根据op分发处理完整协议包
 */
void TcpClientManager::processPacket(const uint8_t *fullPacket, size_t totalLen) {
    uint8_t data[2048] = {0};
    memcpy(data, fullPacket, totalLen);
    HEAD *h = (HEAD *)data;
    uint8_t *payload = data + sizeof(HEAD);

    Tools::myPrintln("📥 收到 op" + String(h->op) + " 的响应");

    switch(h->op) {
        case 2:
        {
            //op2:登录响应
            const DEVLOGINBACK *back = (const DEVLOGINBACK *)payload;
            Device::getInstance().setFd(back->fd);
            Device::getInstance().setEq(back->eq);
            Tools::myPrintln("✅ 对 op1 的响应 op");
            break;
        }
        case 4:
        {
            //op4:对写入的响应
            Tools::myPrintln("✅ 对 op3 的响应 op4");
            break;
        }
        case 6:
        {
            //op6:心跳响应
            lastHeartbeat_ = millis(); //更新心跳响应时间
            Tools::myPrintln("✅ 对 op5 的响应 op6");

            const DEVHEARTBACK *p = (const DEVHEARTBACK *)payload;
            strcpy(nowTime, p->nowtime);
            Device::getInstance().setEnable(p->enable);
            Tools::myPrintln("⏰ 获取服务器时间: " + String(p->nowtime));
            break;
        }
        case 8:
        {
            //op8: 服务器下发定时任务
            const DEVTIMECTRLBACK_ARR *arr = (const DEVTIMECTRLBACK_ARR *)payload;
            Tools::myPrintln("📥 收到 op8: 定时控制指令 (" + String(arr->num) + " 条)");
            for(int i=0; i<arr->num&&i<BACK_SIZE;i++) {
                addScheduledTask(arr->arr[i]);
            }
            break;
        }
        case 10:
        {
            //op10: 服务器下发销毁任务的响应
            const DEVDESTROYBACK *res = (const DEVDESTROYBACK *)payload;
            if(res->res == 1) {
                Tools::myPrintln("✅ op9 销毁指令服务器确认成功");
            }
            else{
                Tools::myPrintln("❌ op9 销毁失败");
            }
            break;
        }
        default:
        {
            Tools::myPrintln("❓ 未知 op: " + String(h->op));
            break;
        }

    }

}

/**
 * 添加定时任务
 */
void TcpClientManager::addScheduledTask(const DEVTIMECTRLBACK &task) {
    for(int i=0; i<taskCount_;i++){
        if(scheduledTasks_[i].id == task.id) {
            return; //已存在相同ID的任务,不添加
        }
    }
    if(taskCount_ < MAX_SCHEDULED_TASKS) {
        scheduledTasks_[taskCount_++] = task;
        char log[128];
        sprintf(log, "✅ 添加新定时任务 ID=%d, mask=0x%02X, state=0x%02X", task.id, task.jd_mask, task.jd_state);
        Tools::myPrintln(log);
    }
}

/**
 * 移除指定ID任务
 */
void TcpClientManager::removeScheduledTaskById(int id) {
    for (int i = 0; i< taskCount_;i++) {
        if(scheduledTasks_[i].id == id) {
            //移除任务
            for (int j = i; j < taskCount_ -1; j++) {
                scheduledTasks_[j] = scheduledTasks_[j+1];
            }
            taskCount_--;
            Tools::myPrintln("🗑️ 本地移除任务 ID=" + String(id));
            return;
        }
    }
}

/**
 * 发送op9销毁任务
 */
void TcpClientManager::sendOp9Destroy(int id) {
    DEVDESTROY req = {};
    req.id = id;
    sendPacket(9, &req, sizeof(DEVDESTROY));
    Tools::myPrintln("📤 发送 op9: 销毁任务 ID=" + String(id));
}

/**
 * 检查并执行所有定时任务
 */
void TcpClientManager::checkAndExecuteScheduledTasks()
{
    time_t now = string_to_timestamp(nowTime);
    if (now < 100000000)
        return;

    struct tm *t = localtime(&now);
    int currentSec = t->tm_hour * 3600 + t->tm_min * 60 + t->tm_sec;
    int currentWeekDay = t->tm_wday;
    uint8_t todayBit = (currentWeekDay == 0) ? WEEK_SUN : (1 << (currentWeekDay - 1));
    char dateStr[MSG_LENGTH];
    strftime(dateStr, sizeof(dateStr), "%Y-%m-%d", t);
    // Tools::myPrintln(dateStr);

    uint8_t effectiveMask = 0;
    uint8_t effectiveState = 0;

    // 第一步：找出所有当前生效的任务，并合并控制状态
    for (int i = 0; i < taskCount_; i++)
    {
        DEVTIMECTRLBACK &task = scheduledTasks_[i];
        int startSec = parseTimeToSeconds(task.start_time);
        int endSec = parseTimeToSeconds(task.end_time);
        if (startSec == -1 || endSec == -1)
            continue;

        bool inWindow = isInTimeWindow(currentSec, startSec, endSec);
        bool matchesDateOrRepeat = false;

        if (task.t_type == REPEAT_ONCE && strcmp(task.date, dateStr) == 0)
        {
            matchesDateOrRepeat = true;
        }
        else if (task.t_type == REPEAT_DAILY)
        {
            matchesDateOrRepeat = true;
        }
        else if (task.t_type == REPEAT_WEEKLY && (task.week_mask & todayBit))
        {
            matchesDateOrRepeat = true;
        }

        if (inWindow && matchesDateOrRepeat)
        {
            effectiveMask |= task.jd_mask;
            effectiveState |= (task.jd_state & task.jd_mask);
        }
    }

    // 第二步：应用最终状态
    effectiveMask &= SCHEDULE_ALLOWED_MASK;
    effectiveState &= SCHEDULE_ALLOWED_MASK;

    if (effectiveMask)
    {
        //
        // printf("生效任务: mask=0x%02X, state=0x%02X\n", effectiveMask, effectiveState);
        // 控制继电器...
        // setRelayOutput(effectiveMask, effectiveState);
        // shouldExecute = true;
       
        shouldExecute = false; // 恢复雷达控制
    }
    else
    {
        shouldExecute = false; // 恢复雷达控制
    }

    // 第三步：清理过期的 REPEAT_ONCE 任务
    for (int i = 0; i < taskCount_;)
    {
        DEVTIMECTRLBACK &task = scheduledTasks_[i];
        if (task.t_type != REPEAT_ONCE)
        {
            i++;
            continue;
        }
        if (strcmp(task.date, dateStr) != 0)
        {
            i++;
            continue;
        }
        int startSec = parseTimeToSeconds(task.start_time);
        int endSec = parseTimeToSeconds(task.end_time);
        if (startSec == -1 || endSec == -1)
        {
            i++;
            continue;
        }
        bool inWindow = isInTimeWindow(currentSec, startSec, endSec);
        if (currentSec > endSec) // !inWindow
        {
            sendOp9Destroy(task.id);
            removeScheduledTaskById(task.id);
            // 不 ++i，因为数组前移了
        }
        else
        {
            i++;
        }
    }
}

// ========================
// 重连后处理（目前仅重置状态）
// ========================
void TcpClientManager::handleReconnect()
{
    // 实际逻辑已在 run() 中处理，此处可留空或做额外初始化
    taskCount_ = 0; // 可选：清空任务（通常不清，因服务器会重新下发）
}

// 在 TcpClientManager.cpp 中新增以下函数
bool TcpClientManager::isConnected() const
{
    return isConnected_;
}


void TcpClientManager::setRelayOutput(uint8_t mask, uint8_t state)
{
    if (mask == 0) return;
    uint8_t newState = (g_currentRelayState & ~mask) | (state & mask);
    if (newState != g_currentRelayState) {
        g_currentRelayState = newState;
        writeRelayRegister(g_currentRelayState);
    }
}

void writeRelayRegister(uint8_t state)
{
    uint8_t output = (~state) & 0x07; // 只取低3位，并取反

    const int controllableIndexes[2] = {LIGHT_INDEX, PLUG_INDEX};
    for (int idx = 0; idx < 2; idx++)
    {
        int i = controllableIndexes[idx];
        digitalWrite(RELAY_PINS[i], (output & (1 << i)) ? HIGH : LOW);
        Device::getInstance().setJdStaAt(i, ((state & (1 << i)) != 0) ? 1 : 2);
        // 解释：
        // 如果 output 的第 i 位是 1 → 写 HIGH（继电器 OFF）
        // 如果是 0 → 写 LOW（继电器 ON）
    }

    digitalWrite(AC_PIN, HIGH);
    Device::getInstance().setJdStaAt(AC_INDEX, 2);

    char relayLog[96];
    sprintf(relayLog, "写入继电器: state=0x%02X -> output=0x%02X", state, output);
    Tools::myPrintln(relayLog);
    Log_relayRadarSnapshot("TCP_WRITE_RELAY");
}
