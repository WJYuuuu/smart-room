#include "Device.h"

/**
 * 单例模式实现Device,确保全局唯一性
 */
Device &Device::getInstance() {
       static Device instance;
       return instance;
}

/**
 * 构造函数,当创建新实例时调用
 */
Device::Device() {
}

/**
 * 返回值为常量:const char *表示返回的是一个指向常量字符的指针,调用者不能修改这个字符串内容
 * 函数内部不修改对象成员:const修饰成员函数,表示这个函数不会修改对象的成员变量,可以在const对象上调用
 */
const char *Device::getIp() const {
       return ip;
}

void Device::setIp(const char *newIp) {
       if(newIp){
              strncpy(ip, newIp, sizeof(ip)-1);
              ip[sizeof(ip)-1] = '\0';
       }
}

/**
 * 读楼层
 */
int8_t Device::getFloor() const{
       return floor;
}

/**
 * 设置楼层
 */
void Device::setFloor(int8_t newFloor){
       floor = newFloor;
}

/**
 * 读房间号
 */
const char *Device::getRoom() const {
       return room;
}
 /**
  * 设置房间号
  */
void Device::setRoom(const char *newRoom) {
       if(newRoom){
              strncpy(room, newRoom, sizeof(room)-1);
              room[sizeof(room)-1] = '\0';
       }
}




/**
 * 析构函数,当对象被销毁时调用!
 * 显示默认析构~Device() = default; 明确告诉编译器用标准默认析构
 * 和 ~Device() {} 的区别在于,前者允许编译器优化,后者会生成一个空的析构函数,可能会增加一些开销
 */
Device::~Device() = default;
