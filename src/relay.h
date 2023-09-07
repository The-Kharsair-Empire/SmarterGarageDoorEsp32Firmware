#pragma once
#ifndef RELAY_H
#define RELAY_H
#include <Arduino.h>
#include "system.h"

#define RELAY_SIGNAL_PIN 16

TaskHandle_t relay_auto_off_th;
int param_duration = 0;

static SemaphoreHandle_t relay_auto_off_sem_binary;

void switch_off_relay_task(void*) {
    while (1) {
        if (xSemaphoreTake(relay_auto_off_sem_binary, portMAX_DELAY) == pdTRUE) {
            vTaskDelay(param_duration / portTICK_PERIOD_MS);
            digitalWrite(RELAY_SIGNAL_PIN, LOW);
        }
    }
}

void initialize_relay() {
    pinMode(RELAY_SIGNAL_PIN, OUTPUT);
    relay_auto_off_sem_binary = xSemaphoreCreateBinary();

    xTaskCreatePinnedToCore(
        &switch_off_relay_task,
        "Relay Task",
        2048,
        NULL,
        1,
        &relay_auto_off_th,
        app_cpu
    );
}


void switch_on_relay(int duration) {
    param_duration = duration;
    digitalWrite(RELAY_SIGNAL_PIN, HIGH);
    xSemaphoreGive(relay_auto_off_sem_binary);

}

#endif