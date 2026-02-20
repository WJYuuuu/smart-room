#ifndef _MQTTCONNECT_H_
#define _MQTTCONNECT_H_

#include <AsyncMqttClient.h>
#include "ArduinoJson.h"
#include "protocol.h"
#include "Device.h"
#include "Tools.h"
#include "HeartBag.h"

extern AsyncMqttClient mqttClient;
D:\YM_code\Project_vwjy\include\main.h
void mqtt_begin();


//订阅主题函数
void subTopic(HeartBag konwnDevices[MAX_LD_NUM]);

//从主题提取设备ID
String getDeviceIdFromTopic(const String &topic);
bool isDataTopic(const char *topic);

void TaskRadarHeart(void *parameter);
void TaskMqttConnect(void *pvParameters);

void processHeart(String top);
void processData(String top, String payloadStr);
void sendJD_Sta();
void ctrJD();

#endif