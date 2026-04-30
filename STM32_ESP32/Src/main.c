#include <stdint.h>
#include "projdefs.h"
#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"  
#include "main.h"

#include "My_I2C.h"
#include "SHT30.h"
#include "BH1750.h"
#include "OLED.h"
#include "Task_Creat.h"
#include "Serial.h"
#include "Led_Buzzer_Relay_Init.h"


void SystemClock_Config(void);

int main(void){
    HAL_Init();
    SystemClock_Config();
    Led_Init();
    Buzzer_Init();
    Relay_Init();
    OLED_Init();
    Serial_Init();
    I2C_Init(BH1750_I2C_SDA,BH1750_I2C_SCL,BH1750_GPIOx);
    BH1750_Init_Cycle_Low();
    I2C_Init(SHT30_I2C_SDA,SHT30_I2C_SCL,SHT30_GPIOx);
    SHT30_Init_Cycle();

    FreeRTOS_Demo();
    vTaskStartScheduler();
    while(1);
    
}



/**
 * @brief  系统时钟配置（8MHz外部晶振 → 72MHz系统时钟）
 */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /* 配置内部RC振荡器 */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        while(1);
    }

    /* 配置时钟分频 */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        while(1);
    }
}

void Error_Handler(){
     portDISABLE_INTERRUPTS(); 
    OLED_Clear();
    OLED_ShowString(0, 0,"ERROR!!!", OLED_8X16);
    OLED_Update();
    while(1);
}

/* FreeRTOS 静态内存必需函数：空闲任务内存分配 */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

/* 定时器任务的栈大小，通常在 FreeRTOSConfig.h 中定义为 configTIMER_TASK_STACK_DEPTH */
static StaticTask_t xTimerTaskTCB; /* 定时器任务控制块 (TCB) */
static StackType_t  uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ]; /* 定时器任务栈 */

void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, 
                                      StackType_t **ppxTimerTaskStackBuffer, 
                                      uint32_t *pulTimerTaskStackSize )
{
    /* 传出 TCB 缓冲区的指针 */
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
    
    /* 传出栈缓冲区的指针 */
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;
    
    /* 传出栈的大小 */
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}