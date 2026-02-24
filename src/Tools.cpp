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
	
}