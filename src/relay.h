#pragma once
#ifndef RELAY_H
#define RELAY_H
#include <Arduino.h>
#include "system.h"

#define RELAY_SIGNAL_PIN 16

TaskHandle_t relay_auto_off_th;
int param_duration;
bool initialized = false;

void initialize() {
    pinMode(RELAY_SIGNAL_PIN, OUTPUT);
    initialized = true;
}

void switch_off_relay() {
    digitalWrite(RELAY_SIGNAL_PIN, LOW);
}


void switch_off_relay_task(void* param) {
    int* duration = (int*) param;
    vTaskDelay((*duration) / portTICK_PERIOD_MS);
    switch_off_relay();
    vTaskDelete(NULL);
}


void switch_on_relay(int duration) {

    if (!initialized) initialize();
    digitalWrite(RELAY_SIGNAL_PIN, HIGH);


    param_duration = duration;
    xTaskCreatePinnedToCore(
        &switch_off_relay_task,
        "Relay Task",
        2048,
        (void*)&param_duration,
        1,
        &relay_auto_off_th,
        app_cpu
    );
}





#endif