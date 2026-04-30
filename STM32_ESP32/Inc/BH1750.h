#pragma once
#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"  
#include "main.h"

#define BH1750_I2C_SDA GPIO_PIN_10
#define BH1750_I2C_SCL GPIO_PIN_11
#define BH1750_GPIOx GPIOB

void BH1750_Init_Cycle_Low();
void BH1750_ReadData(uint16_t* lux);