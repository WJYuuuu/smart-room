#include "Log.h"
#include "Tools.h"
#include "Device.h"
#include <stdarg.h>

static const char* levelChar(LogLevel l){
  switch(l){case LOG_DEBUG: return "D"; case LOG_INFO: return "I"; case LOG_WARN: return "W"; default: return "E";}
}

void Log_begin(){
  Tools::myPrintln("[LOG] relay/radar snapshot upload enabled (server hourly dir)");
}

void Log_write(LogLevel level, const char* tag, const char* fmt, ...){
  char buf[256];  // 减小缓冲大小
  va_list args;
  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);
  unsigned long ms = millis();
  String line = String("[") + ms + String("ms] [") + levelChar(level) + String("] [") + tag + String("] ") + String(buf) + "\n";

  // 统一输出到串口；Tools::myPrint 内部会继续走 TCP 日志上传
  Tools::myPrint(line);
}

void Log_flush(){ }

String Log_readTail(size_t lines){
  (void)lines;
  return String("[LOG] File logging is disabled.");
}

void Log_relayRadarSnapshot(const char* sourceTag)
{
  int lightLevel = digitalRead(LIGHT_PIN);
  int acLevel = digitalRead(AC_PIN);
  int plugLevel = digitalRead(PLUG_PIN);

  String line;
  line.reserve(420);
  line += "[RR_SNAPSHOT] [src=";
  line += (sourceTag ? sourceTag : "unknown");
  line += "]";
  line += " [floor=";
  line += String(Device::getInstance().getFloor());
  line += "]";
  line += " [room=";
  line += String(Device::getInstance().getRoom());
  line += "] ";

  line += "继电器:";
  line += " 灯(PIN" + String(LIGHT_PIN) + "=" + String(lightLevel) + (lightLevel == LOW ? "->开" : "->关") + ")";
  line += " 空调(PIN" + String(AC_PIN) + "=" + String(acLevel) + (acLevel == LOW ? "->开" : "->关") + ")";
  line += " 插座(PIN" + String(PLUG_PIN) + "=" + String(plugLevel) + (plugLevel == LOW ? "->开" : "->关") + ")";

  line += " | 雷达:";
  bool hasRadar = false;
  for (int i = 0; i < MAX_LD_NUM; ++i)
  {
    String topic = knownDevices[i].getTopic();
    if (topic.length() == 0)
    {
      continue;
    }
    hasRadar = true;
    int st = knownDevices[i].getSta();
    const char* stText = (st == 1) ? "有人" : ((st == 2) ? "无人" : "离线");
    line += " ";
    line += topic;
    line += "=";
    line += stText;
  }

  if (!hasRadar)
  {
    line += " 无配置";
  }
  Tools::myPrintln(line);
}

size_t Log_size()
{ 
  return 0; 
}
