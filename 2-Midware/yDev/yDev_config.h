#ifndef YDEV_CONFIG_H
#define YDEV_CONFIG_H

#include "FreeRTOS.h"

#define YDEV_MALLOC(size) pvPortMalloc(size)
#define YDEV_FREE(ptr) vPortFree(ptr)

#endif // YDEV_CONFIG_H