#pragma once
#include "stm32f1xx_hal.h"
#include "event_groups.h"

#define WAIT_DATA        0x00000001

void Serial_Init(void);
void Serial_SendByte(uint8_t Byte);
void Serial_SendArray(uint8_t *Array, uint16_t Length);
void Serial_SendString(char *String);
void Serial_Printf(char *format, ...);

void Serial_SendInt_Frame(int16_t number,uint8_t cmd_send_number);
void Serial_SendFloat_Frame(float number, uint8_t cmd_send_float);
void Serial_ReceiveData(uint8_t*buf);
void Serial_Store_data(uint8_t* store_buf);
uint8_t Serial_GetRxFlag(void);
uint8_t Serial_GetRxData(void);

extern uint8_t Serial_RxData[31];
extern UART_HandleTypeDef huart1;
extern EventGroupHandle_t  receive_flag;
