#ifndef _HEARTBAG_H_
#define _HEARTBAG_H_

#include <Arduino.h>
#include <time.h>

class HeartBag {

public:
    HeartBag(String topic = "");
    ~HeartBag();

    bool check(uint32_t timeout);

    void updateTime();
    uint32_t getLastMillis() const;

    String getTopic() const;
    void setTopic(String topic);

    String getSubTopic() const;
    void setSubTopic(String topic);

    String getHeartTopic() const;
    void setHeartTopic(String topic);

    uint16_t getIdTop() const;
    void setIdTop(uint16_t val);

    uint16_t getIdHeart() const;
    void setIdHeart(uint16_t val);

    int getSta() const;
    void setSta(int val);

    bool IsOnline() const;
    void resetIsOnline(bool v = true);
    
    uint8_t topnum;

private:
    bool isOnline;  
    int sta;    //在线状态
    String topic;
    uint16_t idTop;   //数据主题索引
    String subtopic;  //完整的数据主题
    uint16_t idHeart;  //心跳主题索引
    String hearttopic;  //完整的心跳主题
    uint32_t lastMillis; //上次收到心跳的时间戳
};

#endif
