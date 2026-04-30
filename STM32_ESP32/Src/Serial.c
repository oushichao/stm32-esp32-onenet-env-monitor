#include "OLED.h"
#include "main.h"
#include "FreeRTOSConfig.h"
#include "stm32f1xx_hal.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "FreeRTOS.h"
#include "semphr.h"
#include <stdint.h>
#include <stdbool.h>
#include "stm32f1xx_hal_uart.h"
#include "event_groups.h"
#include "portmacro.h"

#include "Task_Creat.h"
#include "Serial.h"


#define FRAME_HEAD1      0xAA    // 帧头1
#define FRAME_HEAD2      0x55    // 帧头2
#define FRAME_TAIL1      0xDD    // 帧尾1
#define FRAME_TAIL2      0xEE    // 帧尾2


extern bool relay_switch;
extern float temp_threshold;
extern float humi_threshold;

UART_HandleTypeDef huart1;
uint8_t Serial_RxData[31]={0};
volatile uint8_t Serial_RxFlag;

SemaphoreHandle_t Serial_TxMutex = NULL;
EventGroupHandle_t  receive_flag;


void Error_Handler();
//RTS（请求发送） 和 CTS（清除发送）


/**
 * @brief  串口1初始化（115200波特率，8位数据位，1停止位，无校验，收发中断）
 * @param  无
 * @retval 无
 */
void Serial_Init(void)
{
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    receive_flag=xEventGroupCreate();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    /* TX引脚 PA9 复用推挽输出 */
    GPIO_InitStruct.Pin =       GPIO_PIN_9;
    GPIO_InitStruct.Mode =      GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed =     GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    /* RX引脚 PA10 上拉输入 */
    GPIO_InitStruct.Pin =       GPIO_PIN_10;
    GPIO_InitStruct.Mode =      GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull =      GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    huart1.Instance =           USART1;
    huart1.Init.BaudRate =      115200;
    huart1.Init.WordLength =    UART_WORDLENGTH_8B;     // 8位数据位
    huart1.Init.StopBits =      UART_STOPBITS_1;        // 1停止位
    huart1.Init.Parity =        UART_PARITY_NONE;       // 无校验
    huart1.Init.Mode =          UART_MODE_TX_RX;        // 收发模式
    huart1.Init.HwFlowCtl =     UART_HWCONTROL_NONE;    // 无硬件流控
    huart1.Init.OverSampling =  UART_OVERSAMPLING_16;   // 16倍过采样，适合115200波特率，误差较小
    if (HAL_UART_Init(&huart1) != HAL_OK)
    {
        Error_Handler();
    }

    HAL_NVIC_SetPriority(USART1_IRQn, 5, 0); 
    HAL_NVIC_EnableIRQ(USART1_IRQn);    // 使能USART1中断

    HAL_UART_Receive_IT(&huart1, (uint8_t *)&Serial_RxData, 31);

    if(Serial_TxMutex == NULL)
    {
        Serial_TxMutex = xSemaphoreCreateMutex();
    }
}

/**
 * @brief  串口发送单个字节（线程安全）
 * @param  Byte 要发送的字节数据
 * @retval 无
 */
void Serial_SendByte(uint8_t Byte)
{
    if(Serial_TxMutex != NULL && xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED){
        if(xSemaphoreTake(Serial_TxMutex, pdMS_TO_TICKS(100)) != pdTRUE){
            return; 
        }
    }
    else if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED){
        Error_Handler();
    }

    HAL_UART_Transmit(&huart1, &Byte, 1, pdMS_TO_TICKS(100)); // 超时100ms，避免永久阻塞

    if(Serial_TxMutex != NULL && xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED){
        xSemaphoreGive(Serial_TxMutex);
    }
}

/**
 * @brief  串口发送数组（线程安全）
 * @param  Array 要发送的数组首地址
 * @param  Length 数组长度
 * @retval 无
 */
void Serial_SendArray(uint8_t *Array, uint16_t Length)
{
    if(Array == NULL || Length == 0) return; // 入参非空校验

    if(Serial_TxMutex != NULL && xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED){
        if(xSemaphoreTake(Serial_TxMutex, pdMS_TO_TICKS(100)) != pdTRUE){
            return;
        }
    }
    else if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED){
        Error_Handler();
    }

    HAL_UART_Transmit(&huart1, Array, Length, pdMS_TO_TICKS(100));

    if(Serial_TxMutex != NULL && xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED){
        xSemaphoreGive(Serial_TxMutex);
    }
}

/**
 * @brief  串口发送字符串（线程安全）
 * @param  String 要发送的字符串首地址
 * @retval 无
 */
void Serial_SendString(char *String)
{
    if(String == NULL) return; // 入参非空校验

    uint16_t Length = strlen(String); // 标准库函数，效率更高

    if(Serial_TxMutex != NULL && xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
    {
        if(xSemaphoreTake(Serial_TxMutex, pdMS_TO_TICKS(100)) != pdTRUE)
        {
            return;
        }
    }
    else if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED){
        Error_Handler();
    }

    HAL_UART_Transmit(&huart1, (uint8_t *)String, Length, pdMS_TO_TICKS(100));

    if(Serial_TxMutex != NULL && xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
    {
        xSemaphoreGive(Serial_TxMutex);
    }
}

/**
 * @brief  幂运算辅助函数（用于数字发送，防溢出）
 * @param  X 底数
 * @param  Y 指数
 * @retval 计算结果 X^Y
 */
uint32_t Serial_Pow(uint32_t X, uint32_t Y)
{
    uint32_t Result = 1;
    // 限制最大指数，防止uint32_t溢出
    if(Y > 9) Y = 9;
    while (Y--)
    {
        Result *= X;
    }
    return Result;
}

/**
 * @brief  printf重定向（支持printf直接输出到串口，线程安全）
 * @param  ch 字符
 * @param  f 文件指针
 * @retval 发送的字符
 */
int fputc(int ch, FILE *f)
{
    Serial_SendByte((uint8_t)ch);
    return ch;
}

// 适配STM32CubeIDE GCC编译器，避免printf重定向失效
__attribute__((weak)) int __io_putchar(int ch)
{
    Serial_SendByte((uint8_t)ch);
    return ch;
}

/**
 * @brief  串口格式化输出（兼容sprintf用法，线程安全+防栈溢出）
 * @param  format 格式化字符串
 * @param  ... 可变参数列表
 * @retval 无
 */
void Serial_Printf(char *format, ...)
{
    if(format == NULL) return; // 入参非空校验

    char String[128];
    va_list arg;
    va_start(arg, format);
    // 安全版本vsnprintf，限制最大长度，彻底避免栈溢出
    vsnprintf(String, sizeof(String), format, arg);
    va_end(arg);

    Serial_SendString(String);
}

/**
 * @brief  获取串口接收标志位（读取后自动清零）
 * @param  无
 * @retval 1=接收到新数据，0=无新数据
 */
uint8_t Serial_GetRxFlag(void)
{
    if (Serial_RxFlag == 1)
    {
        Serial_RxFlag = 0;
        return 1;
    }
    return 0;
}

// /**
//  * @brief  获取串口接收到的数据
//  * @param  无
//  * @retval 最近一次接收到的字节数据
//  */
// uint8_t Serial_GetRxData(void)
// {
//     return Serial_RxData;
// }

/**
 * @brief  USART1 中断服务函数
 * @param  无
 * @retval 无
 */
void USART1_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart1);
}

/**
 * @brief  串口接收完成回调函数（中断内调用）
 * @param  huart 串口句柄
 * @retval 无
 */
uint16_t callback_count=0;
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1){
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xEventGroupSetBitsFromISR(receive_flag,WAIT_DATA,&xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        HAL_UART_Receive_IT(&huart1, (uint8_t *)&Serial_RxData, 31);       
    }
}


static uint8_t CheckSum_Calc(uint8_t *data, uint8_t len){
    uint8_t sum = 0;
    for(uint8_t i=0; i<len; i++)
    {
        sum += data[i];
    }
    return sum;
}


// 功能：发送 int16 数字（自动组协议帧，带校验、帧头帧尾）
void Serial_SendInt_Frame(int16_t number,uint8_t cmd_send_number){
    uint8_t frame_buf[9] = {0};  // 协议帧缓冲区
    uint8_t index = 0;
    uint8_t data_len = 2;         // int16固定2字节数据

    // ========== 1. 组帧：按顺序填充 ==========
    // 双帧头
    frame_buf[index++] = FRAME_HEAD1;
    frame_buf[index++] = FRAME_HEAD2;
    // 数据长度
    frame_buf[index++] = data_len;
    // 命令码
    frame_buf[index++] = cmd_send_number;
    // 数据段（int16拆分：高字节+低字节，大端序，通用标准）
    frame_buf[index++] = (number >> 8) & 0xFF;
    frame_buf[index++] = number & 0xFF;
    // 校验位（对 长度+命令+数据 求和）
    frame_buf[index++] = CheckSum_Calc(&frame_buf[2], 4);
    // 双帧尾
    frame_buf[index++] = FRAME_TAIL1;
    frame_buf[index++] = FRAME_TAIL2;

    Serial_SendArray(frame_buf, index);
}

typedef union {
    float f_data;          // 浮点型数据
    uint8_t byte_data[4];  // 拆分成4个字节（串口传输）
} Float_Union;

void Serial_SendFloat_Frame(float number, uint8_t cmd_send_float){
    uint8_t frame_buf[11] = {0}; // float=4字节，帧长度比int16长2字节
    uint8_t index = 0;
    uint8_t data_len = 4;        // float 固定 4 字节数据

    // 浮点数转4字节
    Float_Union fu;
    fu.f_data = number;

    // ========== 组帧 ==========
    frame_buf[index++] = FRAME_HEAD1;
    frame_buf[index++] = FRAME_HEAD2;
    frame_buf[index++] = data_len;      // 长度改为4
    frame_buf[index++] = cmd_send_float;// 浮点命令码
    // 4字节数据（大端序：高字节在前）
    frame_buf[index++] = fu.byte_data[3];
    frame_buf[index++] = fu.byte_data[2];
    frame_buf[index++] = fu.byte_data[1];
    frame_buf[index++] = fu.byte_data[0];
    // 校验位：长度(1)+命令(1)+数据(4) = 共6字节参与校验
    frame_buf[index++] = CheckSum_Calc(&frame_buf[2], 6);
    frame_buf[index++] = FRAME_TAIL1;
    frame_buf[index++] = FRAME_TAIL2;

    Serial_SendArray(frame_buf, index);
}

void Serial_Store_data(uint8_t* store_buf){
    if(store_buf[0]==4){
        if(store_buf[1]==CMD_SET_TEMP_LIMIT){
            Float_Union fu;
            fu.byte_data[0]=store_buf[5];
            fu.byte_data[1]=store_buf[4];
            fu.byte_data[2]=store_buf[3];
            fu.byte_data[3]=store_buf[2];
            temp_threshold=fu.f_data;
        }
        else if(store_buf[1]==CMD_SET_HUMI_LIMIT){
            Float_Union fu2;
            fu2.byte_data[0]=store_buf[5];
            fu2.byte_data[1]=store_buf[4];
            fu2.byte_data[2]=store_buf[3];
            fu2.byte_data[3]=store_buf[2];
            humi_threshold=fu2.f_data;  
        }
    }
    else if(store_buf[0]==2){
        if(store_buf[1]==CMD_SET_RELAY){
            relay_switch=(uint16_t)store_buf[4]<<8|store_buf[5];
        }
    }
}


uint16_t frame_ok_count = 0;
void Serial_ReceiveData(uint8_t*buf){
    uint8_t length=31;
    enum statu{
        Receive_Head1,
        Receive_Head2,
        Receive_Len,
        Receive_Cmd,
        Receive_Data,    
        Receive_Check,
        Receive_Tail1,
        Receive_Tail2
    };
    static enum statu current_statu=Receive_Head1;
    static uint8_t current_len=0;
    static uint8_t store_data[6]={0};
    for(int i=0;i<length;i++){
        switch (current_statu)
        {
        case Receive_Head1:
            if(buf[i]==FRAME_HEAD1){
                memset(store_data, 0, 6);                
                current_statu=Receive_Head2;
            }
            break;
        case Receive_Head2:
            if(buf[i]==FRAME_HEAD2){
                current_statu=Receive_Len;
            }
            else {
                current_statu=Receive_Head1;
            }
            break;
        case Receive_Len:
            if(buf[i]==2||buf[i]==4){
                current_len=buf[i];
                store_data[0]=buf[i];
                current_statu=Receive_Cmd;
            }
            else{
                current_statu=Receive_Head1;
            }   
            break;
        case Receive_Cmd:
            if(buf[i] == CMD_SET_TEMP_LIMIT || buf[i] == CMD_SET_HUMI_LIMIT || buf[i] == CMD_SET_RELAY){
                current_statu=Receive_Data;
                store_data[1]=buf[i];
            }
            else{
                current_statu=Receive_Head1;
            }
            break;
        case Receive_Data:
            store_data[6-current_len]=buf[i];
            if(--current_len==0){
                current_statu=Receive_Check;
            }
            break;
        case Receive_Check:
            if(buf[i]==CheckSum_Calc(store_data,6)){
                current_statu=Receive_Tail1;
            }
            else{
                current_statu=Receive_Head1;
            }
            break;    
        case Receive_Tail1:
            if(buf[i]==FRAME_TAIL1){
                current_statu=Receive_Tail2;
            }
            else{
                current_statu=Receive_Head1;
            }
            break;
        case Receive_Tail2:
            if(buf[i]==FRAME_TAIL2){
                callback_count++;
                current_statu=Receive_Head1;
                Serial_Store_data(store_data);
            }
            else{
                current_statu=Receive_Head1;
            }
            break;             
        default:
            break;
        }
    }
}