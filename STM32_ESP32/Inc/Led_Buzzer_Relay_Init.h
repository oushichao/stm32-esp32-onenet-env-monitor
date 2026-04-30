#pragma once
void Led_Init();
void Buzzer_Init();
void Relay_Init();
void Process_Data(bool relay_switch,float temp_threshold,float humi_threshold);