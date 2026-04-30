#pragma once
#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"  
#include "main.h"

void I2C_Init(uint16_t SDA,uint16_t SCL,GPIO_TypeDef*GPIOx);
void I2C_Write_SDA(uint8_t bitvalue,uint16_t SDA,GPIO_TypeDef*GPIOx);
uint8_t I2C_Read_SDA(uint16_t SDA,GPIO_TypeDef*GPIOx);
void I2C_Write_SCL(uint8_t bitvalue,uint16_t SCL,GPIO_TypeDef*GPIOx);
void I2C_Start(uint16_t SDA,uint16_t SCL,GPIO_TypeDef*GPIOx);
void I2C_Stop(uint16_t SDA,uint16_t SCL,GPIO_TypeDef*GPIOx);
void I2C_SendByte(uint8_t byte,uint16_t SDA,uint16_t SCL,GPIO_TypeDef*GPIOx);
uint8_t I2C_ReceiveByte(uint16_t SDA,uint16_t SCL,GPIO_TypeDef*GPIOx);
void I2C_SendACK(uint8_t AckBit,uint16_t SDA,uint16_t SCL,GPIO_TypeDef*GPIOx);
uint8_t I2C_ReceiveAck(uint16_t SDA,uint16_t SCL,GPIO_TypeDef*GPIOx);