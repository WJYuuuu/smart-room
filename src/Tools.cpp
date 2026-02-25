#include "Tools.h"
#include "esp_timer.h"
#include "esp_log.h"

const char *toolIP = "47.112.116.248";

bool isWiFiConnect = false;
bool g_tcpLogForwarding = false; // 是否正在进行 TCP 日志转发
bool tcpLogUploadEnabled = true;

void TcpLogUpload_send(const String &msg) {
    if (!tcpLogUploadEnabled)
        return;
    if (g_tcpLogForwarding)
        return;
    if (!isWiFiConnect)
        return;
    if (msg.length() == 0) 
		return;
    TcpClientManager &tcp = TcpClientManager::getInstance();

    g_tcpLogForwarding = true;
    tcp.sendLogLine(msg.c_str());
    g_tcpLogForwarding = false;
}





// 将 "HH:MM:SS" 转为从 00:00:00 开始的秒数（0 ~ 86399）
int parseTimeToSeconds(const char *timeStr)
{
    if (!timeStr || strlen(timeStr) < 8)
        return -1;

    // 格式必须是 HH:MM:SS
    int hour = (timeStr[0] - '0') * 10 + (timeStr[1] - '0');
    int min = (timeStr[3] - '0') * 10 + (timeStr[4] - '0');
    int sec = (timeStr[6] - '0') * 10 + (timeStr[7] - '0');

    if (hour >= 0 && hour < 24 &&
        min >= 0 && min < 60 &&
        sec >= 0 && sec < 60)
    {
        return hour * 3600 + min * 60 + sec;
    }
    return -1; // 格式错误
}