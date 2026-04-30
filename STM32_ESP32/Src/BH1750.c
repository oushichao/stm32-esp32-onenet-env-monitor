#include "projdefs.h"
#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "stm32f1xx_hal_def.h"
#include "task.h"  
#include "main.h"
#include "My_I2C.h"
#include <stdint.h>
#include "BH1750.h"
#include "OLED.h"


#define BH1750_ADDR   0x23 //0x5C或者0x23，取决于板上ADDR的引脚
#define BH1750_MEASURE_CYCLE_LOW  0x13  //连续测量低分辨率

void BH1750_Init_Cycle_Low(){
    I2C_Start(BH1750_I2C_SDA, BH1750_I2C_SCL, BH1750_GPIOx);
    I2C_SendByte(BH1750_ADDR<<1, BH1750_I2C_SDA, BH1750_I2C_SCL, BH1750_GPIOx);
    I2C_ReceiveAck(BH1750_I2C_SDA, BH1750_I2C_SCL, BH1750_GPIOx);

    I2C_SendByte(BH1750_MEASURE_CYCLE_LOW, BH1750_I2C_SDA, BH1750_I2C_SCL, BH1750_GPIOx);
    I2C_ReceiveAck(BH1750_I2C_SDA, BH1750_I2C_SCL, BH1750_GPIOx);
    I2C_Stop(BH1750_I2C_SDA, BH1750_I2C_SCL, BH1750_GPIOx);
}

void BH1750_ReadData(uint16_t* lux){
    if(lux==NULL)return ;
    uint8_t data[2]={0};
    I2C_Start(BH1750_I2C_SDA, BH1750_I2C_SCL, BH1750_GPIOx);
    I2C_SendByte(BH1750_ADDR<<1|0x01, BH1750_I2C_SDA, BH1750_I2C_SCL, BH1750_GPIOx);
    I2C_ReceiveAck(BH1750_I2C_SDA, BH1750_I2C_SCL, BH1750_GPIOx);


    data[0]=I2C_ReceiveByte(BH1750_I2C_SDA, BH1750_I2C_SCL, BH1750_GPIOx);
    I2C_SendACK(0, BH1750_I2C_SDA, BH1750_I2C_SCL, BH1750_GPIOx);
    data[1]=I2C_ReceiveByte(BH1750_I2C_SDA, BH1750_I2C_SCL, BH1750_GPIOx);
    I2C_SendACK(1, BH1750_I2C_SDA, BH1750_I2C_SCL, BH1750_GPIOx);

    I2C_Stop(BH1750_I2C_SDA, BH1750_I2C_SCL, BH1750_GPIOx);
    *lux=(data[0]<<8|data[1])/1.2f;
}