#ifndef LOG_H
#define LOG_H

#include <Arduino.h>

enum LogLevel { LOG_DEBUG=0, LOG_INFO, LOG_WARN, LOG_ERROR };

void Log_begin();
void Log_write(LogLevel level, const char* tag, const char* fmt, ...);
void Log_flush();
String Log_readTail(size_t lines); // 返回最近若干行
void Log_sendViaMQTT(const String &topic, size_t chunkSize=1024);
void Log_relayRadarSnapshot(const char* sourceTag);
#define LOGD(tag,...) Log_write(LOG_DEBUG, tag, __VA_ARGS__)
#define LOGI(tag,...) Log_write(LOG_INFO,  tag, __VA_ARGS__)
#define LOGW(tag,...) Log_write(LOG_WARN,  tag, __VA_ARGS__)
#define LOGE(tag,...) Log_write(LOG_ERROR, tag, __VA_ARGS__)

#endif
