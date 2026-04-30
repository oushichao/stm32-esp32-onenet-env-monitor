#pragma once
#include "stm32f1xx_hal.h"
#include "projdefs.h"
#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "stm32f1xx_hal_def.h"
#include "task.h"  
#include "main.h"
#include <stdint.h>
#include <stdbool.h>

#include "My_I2C.h"
#include "SHT30.h"
#include "BH1750.h"

//命令码
#define CMD_SEND_TEMP_FLOAT      0x01  // 温度
#define CMD_SEND_HUMI_FLOAT      0x02  // 湿度
#define CMD_SEND_LIGHT_INT       0x03  // 光照(BH1750)
#define CMD_SET_RELAY            0x10  // 继电器开关控制
#define CMD_SET_TEMP_LIMIT       0x11  // 温度报警阈值
#define CMD_SET_HUMI_LIMIT       0x12  // 湿度报警阈值

void FreeRTOS_Demo();