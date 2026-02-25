#include "main.h"
#include "Tools.h" 
#include "Log.h"

/**
 * @author: WangJingYu
 */

char ssid[20] = "BOLU"; //WIFI名称
char password[30] = "BL86588330D34"; //密码

char SeverIP[15] = "47.112.116.248"; //服务器IP地址
uint16_t log_port = 20011;
uint16_t host_port = 20012;

bool connectWiFi();     //判断wifi是否连接成功

mqtt_begin();



bool connectWiFi() {
  Tools::myPrint("ESP32正在连接WiFi: ");
  Tools::myPrint(ssid);
  WiFi.begin(ssid, password);

  // static unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED)
  {
    vTaskDelay(pdMS_TO_TICKS(500));
    Tools::myPrint(".");
  }

  if(WiFi.status() == WL_CONNECTED) {
    Tools::myPrintln("✅ESP32->WiFi连接成功! IP: " + WiFi.localIP().toString());
    Device::getInstance().setIp(WiFi.localIP().toString().c_str());
    isWiFiConnect = true;
    return true;
  }
  isWiFiConnect = false;
  return false;
}


