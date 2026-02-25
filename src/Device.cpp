#include "Device.h"


void Device::getDev(DEVINFO *p) {
    if(xSemaphoreTake(dMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        //成功获取锁，进入临界区
        strcpy(p->room, this->room);
        strcpy(p->ip, this->ip);
        p->eq = this->eq;
        p->floor = this->floor;
        p->jd_num = this->jd_num;
        p->enable = this->enable;
        p->s_eq = this->s_eq;
        for (ssize_t i = 0; i < this->jd_num; i++) {
            p->jd_sta[i] = this->jd_sta[i];
        }
        
        p->ldinfo.ld_num = this->ld_num;
        for(ssize_t i = 0; i < this->ld_num; i++) {
            p->ldinfo.ld_sta[i] = this->ld_sta[i];
            strcpy(p->ldinfo.lid[i], this->lid[i]);
        }
        xSemaphoreGive(dMutex);
    }
}