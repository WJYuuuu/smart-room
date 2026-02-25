#ifndef __MQTTCONNECT_H__
#define __MQTTCONNECT_H__

#include <AsyncMqttClient.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include "protocol.h"
#include "Device.h"
#include "HeartBag.h"
#include "Tools.h"

extern AsyncMqttClient mqttClient;

extern bool isMqttConnect;


// ===== 新增：重连控制 =====
extern unsigned long lastReconnectAttempt;

extern int reconnectDelay;             // 当前重连延迟（ms）
const int MAX_RECONNECT_DELAY = 30000; // 最大30秒
extern int expectedSubscriptions;      // 预期订阅数量
extern int completedSubscriptions;     // 已完成订阅数量
extern int onlineRadarCount;

extern unsigned long mqttReconnectMillis;
extern const unsigned long MQTT_RECONNECT_INTERVAL;
extern bool wasSomeone;
extern volatile unsigned long lastSomeoneDetectedMs;
extern volatile unsigned long lastLightOnMs;
extern volatile unsigned long lastPlugOnMs;
extern const unsigned long MIN_LIGHT_ON_HOLD_MS;
extern const unsigned long MIN_PLUG_ON_HOLD_MS;
extern int MqttconnectCOUNT;

extern int mqtt_port;


void mqtt_begin();
void readarMutexInit();
void mqttConnect(bool sessionPresent);
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
void onMqttMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties,
                    size_t len, size_t index, size_t total);

void onMqttSubscribe(uint16_t packetId, uint8_t qos);

//订阅主题函数
void subTopic(HeartBag konwnDevices[MAX_LD_NUM]);

#endif // __MQTTCONNECT_H__