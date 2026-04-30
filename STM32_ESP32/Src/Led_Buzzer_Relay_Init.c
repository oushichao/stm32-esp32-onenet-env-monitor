#include "stm32f103xb.h"
#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "stm32f1xx_hal_gpio.h"
#include "stm32f1xx_hal_rcc.h"
#include <stdbool.h>

#include "Led_Buzzer_Relay_Init.h"

#define LED_GPIO        GPIOA
#define LED_GPIO_NUM    GPIO_PIN_11

#define BUZZER_GPIO     GPIOA
#define BUZZER_GPIO_NUM GPIO_PIN_6

#define RELAY_GPIO      GPIOA
#define RELAY_GPIO_NUM  GPIO_PIN_8

extern uint16_t lux;
extern float temperature;
extern float humidity;
void Process_Data(bool relay_switch, float temp_threshold, float humi_threshold){
    // 蜂鸣器监控温度
    if(temperature >= temp_threshold){
        HAL_GPIO_WritePin(BUZZER_GPIO, BUZZER_GPIO_NUM, GPIO_PIN_RESET);
    } else {
        HAL_GPIO_WritePin(BUZZER_GPIO, BUZZER_GPIO_NUM, GPIO_PIN_SET);
    }
    // LED监控湿度
    if(humidity >= humi_threshold){
        HAL_GPIO_WritePin(LED_GPIO, LED_GPIO_NUM, GPIO_PIN_RESET);
    } else {
        HAL_GPIO_WritePin(LED_GPIO, LED_GPIO_NUM, GPIO_PIN_SET);
    }
    // 继电器：纯云端控制
    if(relay_switch){
        HAL_GPIO_WritePin(RELAY_GPIO, RELAY_GPIO_NUM, GPIO_PIN_RESET);
    } else {
        HAL_GPIO_WritePin(RELAY_GPIO, RELAY_GPIO_NUM, GPIO_PIN_SET);
    }
}
void Led_Init(){
    GPIO_InitTypeDef GPIO_InitStruct;
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitStruct.Mode=GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pin=LED_GPIO_NUM;
    GPIO_InitStruct.Pull=GPIO_NOPULL;
    GPIO_InitStruct.Speed=GPIO_SPEED_FREQ_MEDIUM;
    HAL_GPIO_Init(LED_GPIO,&GPIO_InitStruct);
    HAL_GPIO_WritePin(LED_GPIO, LED_GPIO_NUM, GPIO_PIN_SET);
}

void Buzzer_Init(){
    GPIO_InitTypeDef GPIO_InitStruct;
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitStruct.Mode=GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pin=BUZZER_GPIO_NUM;
    GPIO_InitStruct.Pull=GPIO_NOPULL;
    GPIO_InitStruct.Speed=GPIO_SPEED_FREQ_MEDIUM;
    HAL_GPIO_Init(BUZZER_GPIO,&GPIO_InitStruct);
    HAL_GPIO_WritePin(BUZZER_GPIO, BUZZER_GPIO_NUM, GPIO_PIN_SET);   
}

void Relay_Init(){
    GPIO_InitTypeDef GPIO_InitStruct;
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitStruct.Mode=GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pin=RELAY_GPIO_NUM;
    GPIO_InitStruct.Pull=GPIO_NOPULL;
    GPIO_InitStruct.Speed=GPIO_SPEED_FREQ_MEDIUM;
    HAL_GPIO_Init(RELAY_GPIO,&GPIO_InitStruct);
    HAL_GPIO_WritePin(RELAY_GPIO, RELAY_GPIO_NUM, GPIO_PIN_SET);   
}

