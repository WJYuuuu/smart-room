#ifndef __MAIN_H__
#define __MAIN_H__

#include "Modbus.h"
#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include "MqttConnect.h"
#include "Tools.h"
#include "Device.h"
#include "OledShow.h"
#include "MyWtd.h"
#include <ModbusMaster.h>
#include "TcpClientManager.h"
#include <ArduinoJson.h>
#include <LittleFS.h>

#define RECONNECT_DELAY_MS 5000  // 重连间隔？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？

extern int WiFiconnectCOUNT;

extern SemaphoreHandle_t wifiMutex_;
void TaskWifiManager(void *pvParameters );

void DeviceSetUp();

#endif