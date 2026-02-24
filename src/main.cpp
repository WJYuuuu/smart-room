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




bool connectWiFi() {
    Tools::myPrint();
}