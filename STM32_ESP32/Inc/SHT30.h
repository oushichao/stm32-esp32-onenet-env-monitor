#pragma once
#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"  
#include "main.h"

#define SHT30_ADDR  0x44
#define SHT30_I2C_SDA GPIO_PIN_12
#define SHT30_I2C_SCL GPIO_PIN_13
#define SHT30_GPIOx GPIOB

void SHT30_Init_Cycle();
void SHT30_ReadData(float* temperature,float* humidity);