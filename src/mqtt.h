#pragma once
#ifndef MQTT_H
#define MQTT_H


#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include "relay.h"
#include "system.h"
#include "range_finder.h"

WiFiClient client;
PubSubClient mqtt_client(client);

char mqtt_broker[16];
char password[30];
int32_t mqtt_port;

StaticJsonDocument<2048> payload_buffer; 
String payload_string;

String topic_prefix = "device/warden_of_garage_door/";

const char* user = "warden_of_the_garage_door";
const char* name = "Warden of the Garage Door";

bool should_publish = true;
unsigned long prev = 0, now = 0;
unsigned int publish_interval = 10000;

void callback(char* topic, byte* payload, unsigned int length) {
    payload_buffer.clear();
    // Serial.println("Message Arrive");
    // Serial.println(topic);

    DeserializationError error = deserializeJson(payload_buffer, payload, length);

    if (error) {
        // Serial.println("Payload Parsing Failed");
        return;
    }

    if (strcmp(topic, (topic_prefix + "cmd/toggle_relay").c_str()) == 0) {
        Serial.println("Toggle Relay");
        if (payload_buffer.containsKey("toggle") && payload_buffer.containsKey("duration")) {
            if (strcmp(payload_buffer["toggle"], "true") == 0) {
                int on_duration = atoi(payload_buffer["duration"]);
                // Serial.print("toggle relay on for ");
                // Serial.print(on_duration);
                // Serial.print(" MS");
                switch_on_relay(on_duration);  // ms
            }
        }
    } 
    if (strcmp(topic, (topic_prefix + "cmd/update_settings").c_str()) == 0) {
        // update, should publish, publish interval, infer threshold min / max,
        if (payload_buffer.containsKey("infer_threshold")) {
            if (strcmp(payload_buffer["infer_threshold"], "true") == 0) infer_open_close_status = true;
            else infer_open_close_status = false;
        } 
        if (payload_buffer.containsKey("infer_threshold_min") && payload_buffer.containsKey("infer_threshold_max")) {
            
            threshold_min = atoi(payload_buffer["infer_threshold_min"]);
            threshold_max = atoi(payload_buffer["infer_threshold_max"]);
        }
        if (payload_buffer.containsKey("should_publish")) {
            if (strcmp(payload_buffer["should_publish"], "true") == 0) should_publish = true;
            else should_publish = false;
        }
        if (payload_buffer.containsKey("publish_interval")) {
            publish_interval = atoi(payload_buffer["publish_interval"]);
        }
        if (payload_buffer.containsKey("sample_interval")) {
            sample_interval = atoi(payload_buffer["sample_interval"]);
        }
        if (payload_buffer.containsKey("rc_alpha")) {
            rc_alpha = atoff(payload_buffer["rc_alpha"]);
        }
    }
}

void subscribe() {

    mqtt_client.subscribe((topic_prefix + "cmd/toggle_relay").c_str());
    mqtt_client.subscribe((topic_prefix + "cmd/update_settings").c_str());
}

void publish() {
    if (should_publish) {
        now = millis();
        if (now - prev > publish_interval) {
            prev = now;
            payload_buffer.clear();
            payload_string.clear();

            payload_buffer["distance"] = get_distance();

            if (infer_open_close_status) {
                payload_buffer["door_open"] = infer_is_door_open();
            }

            serializeJson(payload_buffer, payload_string);
            mqtt_client.publish((topic_prefix + "stat/garage_door_status").c_str(), 
                payload_string.c_str(), 
                true);
        }
    }
}

void mqtt_reconnect() {
    while(! mqtt_client.connected()) {
        if (mqtt_client.connect(name, user, password)) {
            subscribe();
        }
        else vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

void mqtt_loop() {
    while (1) {
        if (WiFi.status() == WL_CONNECTED && !mqtt_client.connected()) {
            mqtt_reconnect();

        } else if (WiFi.status() != WL_CONNECTED) {
            if (mqtt_client.connected())
                mqtt_client.disconnect();
            continue;
        }
        mqtt_client.loop();
        publish();

    }
}

void mqtt_task(void*) {
    while (WiFi.status() != WL_CONNECTED) { // wait for wifi to connect first
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
    mqtt_client.setBufferSize(2048);
    mqtt_client.setServer(mqtt_broker, mqtt_port);
    mqtt_reconnect(); // it will crash if wifi is not connected
    mqtt_client.setCallback(callback);

    mqtt_loop();
}

#endif