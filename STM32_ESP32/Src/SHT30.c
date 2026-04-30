#include "projdefs.h"
#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "stm32f1xx_hal_def.h"
#include "task.h"  
#include "main.h"
#include "SHT30.h"
#include "My_I2C.h"
#include <stdint.h>
#include "OLED.h"

//循环模式（周期1Hz）
#define SHT30_MEASURE_CYCLE_H    0x22  // 循环命令高字节
#define SHT30_MEASURE_CYCLE_L    0x36  // 循环命令低字节      


void SHT30_Init_Cycle(){
    I2C_Start(SHT30_I2C_SDA, SHT30_I2C_SCL, SHT30_GPIOx);
    I2C_SendByte(SHT30_ADDR<<1, SHT30_I2C_SDA, SHT30_I2C_SCL, SHT30_GPIOx);  
    I2C_ReceiveAck(SHT30_I2C_SDA, SHT30_I2C_SCL, SHT30_GPIOx);

    I2C_SendByte(SHT30_MEASURE_CYCLE_H, SHT30_I2C_SDA, SHT30_I2C_SCL, SHT30_GPIOx);
    I2C_ReceiveAck(SHT30_I2C_SDA, SHT30_I2C_SCL, SHT30_GPIOx);
    I2C_SendByte(SHT30_MEASURE_CYCLE_L, SHT30_I2C_SDA, SHT30_I2C_SCL, SHT30_GPIOx);
    I2C_ReceiveAck(SHT30_I2C_SDA, SHT30_I2C_SCL, SHT30_GPIOx);    
    I2C_Stop(SHT30_I2C_SDA, SHT30_I2C_SCL, SHT30_GPIOx);
}

void SHT30_ReadData(float* temperature,float* humidity){
    if(temperature==NULL||humidity==NULL)return;
    uint8_t data[6]={0};

    I2C_Start(SHT30_I2C_SDA, SHT30_I2C_SCL, SHT30_GPIOx);
    I2C_SendByte(SHT30_ADDR << 1, SHT30_I2C_SDA, SHT30_I2C_SCL, SHT30_GPIOx);       // 写地址 0x88
    if(I2C_ReceiveAck(SHT30_I2C_SDA, SHT30_I2C_SCL, SHT30_GPIOx) != 0) { I2C_Stop(SHT30_I2C_SDA, SHT30_I2C_SCL, SHT30_GPIOx); return; }
    I2C_SendByte(0xE0, SHT30_I2C_SDA, SHT30_I2C_SCL, SHT30_GPIOx);                   // Fetch Data 命令高字节
    if(I2C_ReceiveAck(SHT30_I2C_SDA, SHT30_I2C_SCL, SHT30_GPIOx) != 0) { I2C_Stop(SHT30_I2C_SDA, SHT30_I2C_SCL, SHT30_GPIOx); return; }
    I2C_SendByte(0x00, SHT30_I2C_SDA, SHT30_I2C_SCL, SHT30_GPIOx);                   // Fetch Data 命令低字节
    if(I2C_ReceiveAck(SHT30_I2C_SDA, SHT30_I2C_SCL, SHT30_GPIOx) != 0) { I2C_Stop(SHT30_I2C_SDA, SHT30_I2C_SCL, SHT30_GPIOx); return; }

    I2C_Start(SHT30_I2C_SDA, SHT30_I2C_SCL, SHT30_GPIOx);                       
    I2C_SendByte( (SHT30_ADDR << 1) | 0x01, SHT30_I2C_SDA, SHT30_I2C_SCL, SHT30_GPIOx ); 
    I2C_ReceiveAck(SHT30_I2C_SDA, SHT30_I2C_SCL, SHT30_GPIOx);                  
    for(uint8_t i=0;i<6;i++){
        data[i]=I2C_ReceiveByte(SHT30_I2C_SDA, SHT30_I2C_SCL, SHT30_GPIOx);
        if(i < 5)I2C_SendACK(0, SHT30_I2C_SDA, SHT30_I2C_SCL, SHT30_GPIOx);        // 前5字节：主机发ACK
        else I2C_SendACK(1, SHT30_I2C_SDA, SHT30_I2C_SCL, SHT30_GPIOx);            // 最后1字节：主机发NAK       
    }
    I2C_Stop(SHT30_I2C_SDA, SHT30_I2C_SCL, SHT30_GPIOx);
    uint16_t raw_temp = (data[0] << 8) | data[1];
    uint16_t raw_humi = (data[3] << 8) | data[4];    

    *temperature=-45 + 175 * (raw_temp / 65535.0);
    *humidity=100 * (raw_humi / 65535.0);
    // # 温度计算（单位：℃）
    // temp = -45 + 175 * (raw_temp / 65535.0)

    // # 湿度计算（单位：%RH）
    // humidity = 100 * (raw_humidity / 65535.0)
}