#include "HeartBag.h"

HeartBag::HeartBag(String topic) {
    this->topic = topic;
    //初始化lastMillis为当前时间，避免 millis() 较小时空 slot 被误判为超时离
    this->lastMillis = millis();
    isOnline = false;
    sta = -1;
    topnum = 0; //默认0，表示未设置
}

HeartBag::~HeartBag() {

}

bool HeartBag::check(uint32_t timeout) {
    return (millis() - lastMillis) >= timeout;
}

void HeartBag::updateTime() {
    lastMillis = millis();
}

uint32_t HeartBag::getLastMillis() const {
    return lastMillis;
}

String HeartBag::getTopic() const {
    return this->topic;
}

void HeartBag::setTopic(String topic) {
    this->topic = topic;
}

String HeartBag::getSubTopic() const {
    return this->subtopic;
}

void HeartBag::setSubTopic(String topic) {
    this->subtopic = topic;
}

String HeartBag::getHeartTopic() const {
    return this->hearttopic;
}

void HeartBag::setHeartTopic(String topic) {
    this->hearttopic = topic;
}

uint16_t HeartBag::getIdTop() const {
    return this->idTop;
}

void HeartBag::setIdTop(uint16_t val) {
    this->idTop = val;
}

//心跳主题索引
uint16_t HeartBag::getIdHeart() const {
    return this->idHeart;
}

void HeartBag::setIdHeart(uint16_t val) {
    this->idHeart = val;
}

bool HeartBag::IsOnline() const {
    return this->isOnline;
}

void HeartBag::resetIsOnline(bool v) {
    this->isOnline = v;
}

int HeartBag::getSta() const {
    return this->sta;
}

void HeartBag::setSta(int val) {
    this->sta = val;
}

