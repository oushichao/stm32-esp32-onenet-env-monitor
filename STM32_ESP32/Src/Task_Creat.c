#include "BH1750.h"
#include "portmacro.h"
#include "projdefs.h"
#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "stm32f1xx_hal_def.h"
#include "task.h"  
#include "main.h"
#include <stdint.h>
#include "queue.h"
#include <stdbool.h>
#include "event_groups.h"
#include "semphr.h"

#include "SHT30.h"
#include "My_I2C.h"
#include "Task_Creat.h"
#include "Serial.h"
#include "OLED.h"
#include "Led_Buzzer_Relay_Init.h"


//总线任务
#define Total_Task_size                 128
#define Total_Task_Prio                 4
TaskHandle_t Total_Task_Handler;
//串口接收任务
#define Serial_Receive_Task_Size        256
#define Serial_Receive_Task_Prio        3
TaskHandle_t Serial_Receive_Task_Handler;
//串口发送任务
#define Serial_Report_Task_Size         256
#define Serial_Report_Task_Prio         2
TaskHandle_t Serial_Report_Task_Handler;
//OLED显示任务
#define Oled_Show_Task_Size             256
#define Oled_Show_Task_Prio             1
TaskHandle_t Oled_Show_Task_Handler;

bool relay_switch=false;
float temp_threshold=30.0;
float humi_threshold=70.0;
extern EventGroupHandle_t  receive_flag;
extern uint8_t Serial_RxData[31];

uint16_t lux=0;                         //环境光强度
float temperature=0.0, humidity=0.0;    //温度，湿度

void Serial_Report_Task_Init(void *argument){
    (void)argument;
    TickType_t xLastWakeTime=xTaskGetTickCount();
    while(1){
        SHT30_ReadData(&temperature, &humidity);
        BH1750_ReadData(&lux);
        Serial_SendFloat_Frame(temperature, CMD_SEND_TEMP_FLOAT);
        Serial_SendFloat_Frame(humidity, CMD_SEND_HUMI_FLOAT);
        Serial_SendInt_Frame(lux, CMD_SEND_LIGHT_INT);     
        vTaskDelayUntil(&xLastWakeTime, 1000);
    }
}

void Serial_Receive_Task_Init(void *argument){
    (void)argument;
    while(1){
        xEventGroupWaitBits(receive_flag,
                WAIT_DATA,
                pdTRUE,
                pdTRUE,
                portMAX_DELAY);
        Serial_ReceiveData(Serial_RxData);
        Process_Data(relay_switch,temp_threshold,humi_threshold);
    }
}

void Oled_Task_Init(void *argument){
    (void)argument;
    TickType_t xLastWakeTime=xTaskGetTickCount();
    while(1){
        OLED_ShowString(0, 0, "Tem:",OLED_8X16);
        OLED_ShowFloatNum(32, 0, temperature, 2, 2, OLED_8X16);
        
        OLED_ShowString(0, 16, "Hum:",OLED_8X16);
        OLED_ShowFloatNum(32, 16, humidity, 2, 2, OLED_8X16);
        
        OLED_ShowString(0, 32, "Lux:",OLED_8X16);
        OLED_ShowNum(32, 32, lux, 3, OLED_8X16);
          
        OLED_Update();              
        vTaskDelayUntil(&xLastWakeTime, 1000);
    }
} 

void Total_Task_Init(void *argument){
    (void)argument;
    xTaskCreate(Serial_Report_Task_Init, 
                "Serial_Report_Task_Init",
                Serial_Report_Task_Size,
                NULL, 
                Serial_Report_Task_Prio, 
                &Serial_Report_Task_Handler);

    xTaskCreate(Oled_Task_Init, 
                "Oled_Task_Init",
                Oled_Show_Task_Size,
                NULL, 
                Oled_Show_Task_Prio, 
                &Oled_Show_Task_Handler);

    xTaskCreate(Serial_Receive_Task_Init, 
                "Serial_Receive_Task_Init",
                Serial_Receive_Task_Size,
                NULL, 
                Serial_Receive_Task_Prio, 
                &Serial_Receive_Task_Handler);
    vTaskDelete(NULL);    
}

void FreeRTOS_Demo(){
    xTaskCreate(Total_Task_Init,
                "Total_Task_Creat",
                Total_Task_size,
                NULL, 
                Total_Task_Prio, 
                &Total_Task_Handler);
}