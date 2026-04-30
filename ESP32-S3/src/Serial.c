#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include "driver/gpio.h"
#include "driver/uart.h"
#include "freertos/queue.h"
#include "Serial.h"
#include "FreeRTOS_Task.h"
SemaphoreHandle_t Serial_TxMutex=NULL;
QueueHandle_t uart_queue;
const int uart_buffer_size=(1024*2);

extern bool relay_switch;
extern float temp_threshold;
extern float humi_threshold;

extern float temperature;
extern float humidity;
extern int32_t light;
extern bool relay_state;

extern uint8_t receive_buf[128];

typedef union {
float f_data;         
uint8_t byte_data[4];       //低到高
} Float_Union;

uint8_t CheckSum_Calc(uint8_t *data, uint8_t len){
    uint8_t sum = 0;
    for(uint8_t i=0; i<len; i++)
    {
        sum += data[i];
    }
    return sum;
}

void Serial_Init(){
    if(Serial_TxMutex==NULL) Serial_TxMutex=xSemaphoreCreateMutex();

    uart_config_t uart_config = {
        .baud_rate    = 115200,
        .data_bits    = UART_DATA_8_BITS,
        .parity       = UART_PARITY_DISABLE,
        .stop_bits    = UART_STOP_BITS_1,
        .flow_ctrl    = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 0,
        .source_clk   = UART_SCLK_DEFAULT,   // ← 关键：指定时钟源
    };

    ESP_ERROR_CHECK(uart_param_config(MY_UART_NUM, &uart_config));

    ESP_ERROR_CHECK(uart_set_pin(
        MY_UART_NUM,
        GPIO_NUM_21,    // TX
        GPIO_NUM_20,    // RX
        GPIO_NUM_NC,
        GPIO_NUM_NC
    ));

    ESP_ERROR_CHECK(uart_driver_install(
        MY_UART_NUM,
        uart_buffer_size,
        uart_buffer_size,
        10,
        &uart_queue,
        0
    ));
}

void Serial_SendByte(uint8_t byte){
    if(Serial_TxMutex!=NULL){
        if(xSemaphoreTake(Serial_TxMutex,pdMS_TO_TICKS(100))!=pdTRUE)return;
    }
    uart_write_bytes(MY_UART_NUM,&byte,1);
    if(Serial_TxMutex!=NULL){
        xSemaphoreGive(Serial_TxMutex);
    }
}

void Serial_SendString(char*string){
    if(string==NULL)return;
    if(Serial_TxMutex!=NULL){
    if(xSemaphoreTake(Serial_TxMutex,pdMS_TO_TICKS(100))!=pdTRUE)return;
    }
    uart_write_bytes(MY_UART_NUM,string,strlen(string));
    if(Serial_TxMutex!=NULL){
        xSemaphoreGive(Serial_TxMutex);
    }
}

void Serial_SendNumber(int16_t number){
    char num_buffer[10]={0};
    char temp_buffer[10]={0};
    uint8_t i=0;
    uint8_t j=0;
    if(number<0){
        num_buffer[i++]='-';
        number=-number;
    }
    if(number!=0){
        temp_buffer[j++]=number%10+'0';
        while((number/=10)!=0){
            temp_buffer[j++]=number%10+'0';
        }
        for(int k=j-1; k>=0; k--){
            num_buffer[i++] = temp_buffer[k];
        }
    }
    else{
        num_buffer[i++]='0';
    }

    if(Serial_TxMutex!=NULL){
    if(xSemaphoreTake(Serial_TxMutex,pdMS_TO_TICKS(100))!=pdTRUE)return;
    }
    uart_write_bytes(MY_UART_NUM,num_buffer,i);
    if(Serial_TxMutex!=NULL){
        xSemaphoreGive(Serial_TxMutex);
    }
}

void Serial_SendArray(uint8_t*buf,uint16_t length){
    if(buf==NULL)return;
    if(Serial_TxMutex!=NULL){
    if(xSemaphoreTake(Serial_TxMutex,pdMS_TO_TICKS(100))!=pdTRUE)return;
    }
    uart_write_bytes(MY_UART_NUM,buf,length);
    if(Serial_TxMutex!=NULL){
        xSemaphoreGive(Serial_TxMutex);
    }
}

// 功能：发送 int16 数字（自动组协议帧，带校验、帧头帧尾）
void Serial_SendNumber_Frame(int16_t number,uint8_t cmd_send_number){
    uint8_t frame_buf[16] = {0};  // 协议帧缓冲区
    uint8_t index = 0;
    uint8_t data_len = 2;         // int16固定2字节数据

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

    if(Serial_TxMutex!=NULL){
        if(xSemaphoreTake(Serial_TxMutex,pdMS_TO_TICKS(100))!=pdTRUE)return;
    }
    uart_write_bytes(MY_UART_NUM, frame_buf, index);
    if(Serial_TxMutex!=NULL){
        xSemaphoreGive(Serial_TxMutex);
    }
}

void Serial_SendFloat_Frame(float number,uint8_t cmd_send_number){
    Float_Union fu;
    fu.f_data=number;

    uint8_t frame_buf[16] = {0};  // 协议帧缓冲区
    uint8_t index = 0;
    uint8_t data_len = 4;         // float固定4字节数据

    // 双帧头
    frame_buf[index++] = FRAME_HEAD1;
    frame_buf[index++] = FRAME_HEAD2;
    // 数据长度
    frame_buf[index++] = data_len;
    // 命令码
    frame_buf[index++] = cmd_send_number;
    // 数据段
    frame_buf[index++] = fu.byte_data[3];
    frame_buf[index++] = fu.byte_data[2];
    frame_buf[index++] = fu.byte_data[1];
    frame_buf[index++] = fu.byte_data[0];
    // 校验位（对 长度+命令+数据 求和）
    frame_buf[index++] = CheckSum_Calc(&frame_buf[2], 6);
    // 双帧尾
    frame_buf[index++] = FRAME_TAIL1;
    frame_buf[index++] = FRAME_TAIL2;

    if(Serial_TxMutex!=NULL){
        if(xSemaphoreTake(Serial_TxMutex,pdMS_TO_TICKS(100))!=pdTRUE)return;
    }
    uart_write_bytes(MY_UART_NUM, frame_buf, index);
    if(Serial_TxMutex!=NULL){
        xSemaphoreGive(Serial_TxMutex);
    }
}

void Serial_Store_data(uint8_t* store_buf){
    if(store_buf[0]==4){
        if(store_buf[1]==CMD_SEND_TEMP_FLOAT){
            Float_Union fu;
            fu.byte_data[0]=store_buf[5];
            fu.byte_data[1]=store_buf[4];
            fu.byte_data[2]=store_buf[3];
            fu.byte_data[3]=store_buf[2];
            temperature=fu.f_data;
        }
        else if(store_buf[1]==CMD_SEND_HUMI_FLOAT){
            Float_Union fu;
            fu.byte_data[0]=store_buf[5];
            fu.byte_data[1]=store_buf[4];
            fu.byte_data[2]=store_buf[3];
            fu.byte_data[3]=store_buf[2];
            humidity=fu.f_data;  
        }
    }
    else if(store_buf[0]==2){
        if(store_buf[1]==CMD_SEND_LIGHT_INT){
            light=(uint16_t)store_buf[4]<<8|store_buf[5];
        }
    }
}

void Serial_ReceiveData(uint8_t*buf){
    static int length=0;
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
    //Rx FIFO 缓冲区中可用的字节数
    ESP_ERROR_CHECK(uart_get_buffered_data_len(MY_UART_NUM, (size_t*)&length));
    if(length > sizeof(receive_buf)) length = sizeof(receive_buf);
    static uint8_t send_buf[9]={0};        //应答帧
    static uint8_t current_len=0;
    static uint8_t store_data[6]={0};
    if(length>0){
        uart_read_bytes(MY_UART_NUM,buf,length,100);
    }
    for(int i=0;i<length;i++){
        switch (current_statu)
        {
        case Receive_Head1:
            if(buf[i]==FRAME_HEAD1){
                memset(send_buf, 0, 9); 
                memset(store_data, 0, 6);                
                current_statu=Receive_Head2;
                send_buf[0]=buf[i];
            }
            break;
        case Receive_Head2:
            if(buf[i]==FRAME_HEAD2){
                current_statu=Receive_Len;
                send_buf[1]=buf[i];
            }
            else {
                current_statu=Receive_Head1;
            }
            break;
        case Receive_Len:
            if(buf[i]==2||buf[i]==4){
                current_len=buf[i];
                store_data[0]=buf[i];
                send_buf[2]=0x02;
                current_statu=Receive_Cmd;
            }
            else{
                current_statu=Receive_Head1;
            }   
            break;
        case Receive_Cmd:
            if(buf[i]==CMD_SEND_TEMP_FLOAT||buf[i]==CMD_SEND_HUMI_FLOAT||buf[i]==CMD_SEND_LIGHT_INT){
                current_statu=Receive_Data;
                store_data[1]=buf[i];
                send_buf[3]=buf[i];
            }
            else{
                current_statu=Receive_Head1;
            }
            break;
        case Receive_Data:
            store_data[6-current_len]=buf[i];
            if(--current_len==0){
                current_statu=Receive_Check;
                send_buf[4]=0x00;
                send_buf[5]=0x00;
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
                send_buf[7]=buf[i];
                current_statu=Receive_Tail2;
            }
            else{
                current_statu=Receive_Head1;
            }
            break;
        case Receive_Tail2:
            if(buf[i]==FRAME_TAIL2){
                send_buf[8]=buf[i];
                send_buf[6]=CheckSum_Calc(&send_buf[2],4);                
                Serial_SendArray(send_buf,9);
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


