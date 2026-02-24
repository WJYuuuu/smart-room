#ifndef OLEDSHOW_H_
#define OLEDSHOW_H_

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// SCL(GPIO22)   SDA(GPIO21)
// 使用默认 I2C 地址 0x3C（常见），若不亮可尝试 0x3D
#define OLED_ADDR 0x3D
// 初始化 OLED 对象（-1 表示无复位引脚）
extern Adafruit_SSD1306 *display;
void displayTask(void *pvParameters);

#endif
