#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "WIFI.h"
#include "OneNET_MQTT.h"
#include "OneNET_dm.h"
#include "Serial.h"

/*总任务初始化*/
#define Start_Total_Task_Size                   2048
#define Start_Total_Task_Prio                   5
TaskHandle_t Start_Total_Task_Handler;
/*连接WIFI与MQTT任务*/ 
#define WIFI_MQTT_Task_Size                     8192
#define WIFI_MQTT_Task_Prio                     4
TaskHandle_t WIFI_MQTT_Task_Handler;
/*上行响应任务*/
#define OneNET_Upload_Task_Size                 4096
#define OneNET_Upload_Task_Prio                 3
TaskHandle_t OneNET_Upload_Task_Handler;
/*串口接收数据任务*/
#define Serial_ReceiveData_Task_Size            4096
#define Serial_ReceiveData_Task_Prio            1
TaskHandle_t Serial_ReceiveData_Task_Handler;



uint8_t receive_buf[128]={0};

void OneNET_Upload_Task(){
    xEventGroupWaitBits(wifi_ev, BIT1, pdFALSE, pdTRUE, portMAX_DELAY);
    TickType_t last_wake_time=xTaskGetTickCount();
    while(1){
        cJSON* property_js=OneNET_Property_Upload();
        char* data=cJSON_PrintUnformatted(property_js);
        Connect_Post_Data(data);
        cJSON_Delete(property_js);
        cJSON_free(data);    
        vTaskDelayUntil(&last_wake_time,pdMS_TO_TICKS(1000));
    }
}

void WIFI_MQTT_Task(){
    Wifi_Sta_Init();
    xEventGroupWaitBits(wifi_ev,BIT0,pdTRUE,pdTRUE,portMAX_DELAY);
    OneNET_Start();
    vTaskDelete(NULL);
}

void Serial_ReceiveData_Task(){
    TickType_t last_wake_time=xTaskGetTickCount();
    while(1){
        Serial_ReceiveData(receive_buf);
        vTaskDelayUntil(&last_wake_time,pdMS_TO_TICKS(1000));
    }
}

void Start_Total_Task(){
    xTaskCreate(
        Serial_ReceiveData_Task,
        "Serial_ReceiveData_Task",
        Serial_ReceiveData_Task_Size,
        NULL,
        Serial_ReceiveData_Task_Prio,
        &Serial_ReceiveData_Task_Handler
    );    

    xTaskCreate(
        WIFI_MQTT_Task,
        "WIFI_MQTT_Task",
        WIFI_MQTT_Task_Size,
        NULL,
        WIFI_MQTT_Task_Prio,
        &WIFI_MQTT_Task_Handler
    );    
    
    xTaskCreate(
        OneNET_Upload_Task,
        "OneNET_Upload_Task",
        OneNET_Upload_Task_Size,
        NULL,
        OneNET_Upload_Task_Prio,
        &OneNET_Upload_Task_Handler
    );
    vTaskDelete(NULL);        
}

void FreeRTOS_Demo(){
    xTaskCreate(
        Start_Total_Task,
        "Start_Task",
        Start_Total_Task_Size,
        NULL,
        Start_Total_Task_Prio,
        &Start_Total_Task_Handler
    );
}