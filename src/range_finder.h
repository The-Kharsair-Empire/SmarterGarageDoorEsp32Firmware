#pragma once
#ifndef RANGE_FINDER_H
#define RANGE_FINDER_H
#include <Arduino.h>
#include "SR04.h"

#define TRIGGER_PIN 25
#define ECHO_PIN 26

TaskHandle_t sample_loop_th;

long filtered_distance;
float rc_alpha = 0.2f;

int sample_interval = 1000;

float threshold_min, threshold_max;
bool infer_open_close_status = false;

SR04 range_finder(ECHO_PIN, TRIGGER_PIN);

float get_distance() {
    return filtered_distance;
}

bool infer_is_door_open() {
    if (filtered_distance > threshold_min && filtered_distance < threshold_max) return true;
    return false;
}

void sample_loop(void*) {
    while(isnan(range_finder.Distance())) {
        vTaskDelay(sample_interval / portTICK_PERIOD_MS);
    }

    filtered_distance = range_finder.Distance();
    while (1) {
        filtered_distance += (range_finder.Distance() - filtered_distance) * rc_alpha;
        vTaskDelay(sample_interval / portTICK_PERIOD_MS);
    }
}


#endif