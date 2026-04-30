#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "driver/ledc.h"

#include "WIFI.h"
#include "Serial.h"
#include "OneNET_MQTT.h"
#include "OneNET_dm.h"
#include "FreeRTOS_Task.h"


void app_main(void){
    Serial_Init();
    FreeRTOS_Demo();
}

