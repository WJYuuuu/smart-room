#include "Tools.h"

SemaphoreHandle_t Tools::xMutex = nullptr;

void Tools::init() {
	if(xMutex == nullptr){
		xMutex = xSemaphoreCreateMutex();
	}
}
