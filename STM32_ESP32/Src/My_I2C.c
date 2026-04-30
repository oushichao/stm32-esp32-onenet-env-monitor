#include "stm32_hal_legacy.h"
#include "stm32f103xb.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_gpio.h"
#include "FreeRTOS.h"
#include "stm32f1xx_hal_rcc.h"
#include "task.h"  
#include "main.h"
#include <stdint.h>
#include "My_I2C.h"

void delay_us(uint32_t us)
{
    uint32_t delay = us * (SystemCoreClock / 1000000 / 5);
    while(delay--);
}

void I2C_Init(uint16_t SDA,uint16_t SCL,GPIO_TypeDef*GPIOx){
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct={
        .Mode=GPIO_MODE_OUTPUT_OD,
        .Pin=SDA|SCL,
        .Pull=GPIO_PULLUP,
        .Speed=GPIO_SPEED_FREQ_MEDIUM,
    };
    HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOx, SDA|SCL,1);
}

void I2C_Write_SDA(uint8_t bitvalue,uint16_t SDA,GPIO_TypeDef*GPIOx){
    HAL_GPIO_WritePin(GPIOx,SDA , bitvalue);
    delay_us(3);
}
uint8_t I2C_Read_SDA(uint16_t SDA,GPIO_TypeDef*GPIOx){
    uint8_t val=HAL_GPIO_ReadPin(GPIOx, SDA);
    delay_us(3);
    return val;
}

void I2C_Write_SCL(uint8_t bitvalue,uint16_t SCL,GPIO_TypeDef*GPIOx){
    HAL_GPIO_WritePin(GPIOx, SCL, bitvalue);
    delay_us(3);
}
void I2C_Start(uint16_t SDA,uint16_t SCL,GPIO_TypeDef*GPIOx){
    I2C_Write_SDA(1,SDA,GPIOx);
    I2C_Write_SCL(1,SCL,GPIOx);
    I2C_Write_SDA(0,SDA,GPIOx);
    I2C_Write_SCL(0,SCL,GPIOx);
}
void I2C_Stop(uint16_t SDA,uint16_t SCL,GPIO_TypeDef*GPIOx){
    I2C_Write_SDA(0,SDA,GPIOx);
     I2C_Write_SCL(1,SCL,GPIOx);
     I2C_Write_SDA(1,SDA,GPIOx);
}

void I2C_SendByte(uint8_t byte,uint16_t SDA,uint16_t SCL,GPIO_TypeDef*GPIOx){
    for(int8_t i=7;i>=0;i--){
        I2C_Write_SDA((byte&(0x01<<i)?1:0),SDA,GPIOx);
        I2C_Write_SCL(1,SCL,GPIOx);
        I2C_Write_SCL(0,SCL,GPIOx);
    }
}
uint8_t I2C_ReceiveByte(uint16_t SDA,uint16_t SCL,GPIO_TypeDef*GPIOx){
    uint8_t data=0x00;
    I2C_Write_SDA(1,SDA,GPIOx);
    for(uint8_t i=0;i<8;i++){
        I2C_Write_SCL(1,SCL,GPIOx);
        if(I2C_Read_SDA(SDA,GPIOx)==1)data|=(0x80>>i);
        I2C_Write_SCL(0,SCL,GPIOx);
    }
    return data;
}
void I2C_SendACK(uint8_t AckBit,uint16_t SDA,uint16_t SCL,GPIO_TypeDef*GPIOx){
	I2C_Write_SDA(AckBit,SDA,GPIOx);
	I2C_Write_SCL(1,SCL,GPIOx);	
	I2C_Write_SCL(0,SCL,GPIOx);	
}
uint8_t I2C_ReceiveAck(uint16_t SDA,uint16_t SCL,GPIO_TypeDef*GPIOx){
	I2C_Write_SDA(1,SDA,GPIOx);	
	I2C_Write_SCL(1,SCL,GPIOx);	
	uint8_t AckBit=I2C_Read_SDA(SDA,GPIOx);
	I2C_Write_SCL(0,SCL,GPIOx);	
	return AckBit;
}