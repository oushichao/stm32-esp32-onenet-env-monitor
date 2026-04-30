#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>

#define MY_UART_NUM UART_NUM_1
//================自定义协议帧=====================
#define FRAME_HEAD1      0xAA    // 帧头1
#define FRAME_HEAD2      0x55    // 帧头2
#define FRAME_TAIL1      0xDD    // 帧尾1
#define FRAME_TAIL2      0xEE    // 帧尾2

#define CMD_UP_TEMP      0x01    // 上行：温度上报
#define CMD_DOWN_RELAY   0x81    // 下行：继电器控制

//===================应答帧=====================
#define TASK_SUCCESS    0x0000  //执行成功
#define TASK_FAIL       0x0001  //执行失败

//===================命令码=====================
#define CMD_SEND_TEMP_FLOAT      0x01  // 温度
#define CMD_SEND_HUMI_FLOAT      0x02  // 湿度
#define CMD_SEND_LIGHT_INT       0x03  // 光照(BH1750)
#define CMD_SET_RELAY            0x10  // 继电器开关控制
#define CMD_SET_TEMP_LIMIT       0x11  // 温度报警阈值
#define CMD_SET_HUMI_LIMIT       0x12  // 湿度报警阈值

void Serial_Init();
void Serial_SendByte(uint8_t byte); 
void Serial_SendString(char*string);
void Serial_SendNumber(int16_t number);
void Serial_SendArray(uint8_t*buf,uint16_t length);
void Serial_ReceiveData(uint8_t*buf);

//求和校验
uint8_t CheckSum_Calc(uint8_t *data, uint8_t len);
//带有协议帧的数据段
void Serial_SendNumber_Frame(int16_t number,uint8_t cmd_send_number);
void Serial_SendFloat_Frame(float number,uint8_t cmd_send_number);