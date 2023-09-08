#pragma once
#ifndef SYSTEM_H
#define SYSTEM_H
#include <Arduino.h>
#include <SPIFFS.h>

#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

const char* filename = "/config.json";

bool load_config(JsonDocument& json) {
    SPIFFS.begin();
    if (SPIFFS.exists(filename)) {
        File config = SPIFFS.open(filename, FILE_READ);
        if (config) {
            DeserializationError error = deserializeJson(json, config);
            if (!error) {
                config.close();
                return true;
                
            } else {
                config.close();
                return false;
            }
        }
    } else { 
        return false;
    }
    return false;
}


bool parse_ip_addr(char* ip_str, IPAddress& out_ip) {
    if (out_ip.fromString(ip_str)) {
        return true;
    } else {
        return false;
    }

}

#endif