#include <WiFi.h>
#include <WiFiManager.h>
#include <Arduino.h>
#include "mqtt.h"

#define RESET_PIN 0
WiFiManager wm;

TaskHandle_t wifi_reconnect_th, mqtt_loop_th;

void wifi_check_reconnect_loop(void*) {

    while (1) {
        if (WiFi.status() != WL_CONNECTED) {
            WiFi.disconnect();
            WiFi.reconnect();
            vTaskDelay(4000 / portTICK_PERIOD_MS);

            if (WiFi.status() != WL_CONNECTED) {
                ESP.restart();
            }
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}


void setup() { 
    pinMode(RESET_PIN, INPUT_PULLUP);
    pinMode(BUILTIN_LED, OUTPUT);
    Serial.begin(115200);

    for (unsigned short i = 0; i < 10; i++) { // signal where reset is triggered
        digitalWrite(BUILTIN_LED, HIGH);
        vTaskDelay(50 / portTICK_PERIOD_MS);
        digitalWrite(BUILTIN_LED, LOW);
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }

    WiFi.mode(WIFI_STA);

    wm.setConfigPortalTimeout(1200);
    // wm.setDebugOutput(false);

    WiFiManagerParameter mqtt_broker_ip_textfield("broker_ip", "Broker IP", "", 15);
    WiFiManagerParameter mqtt_broker_port_textfield("broker_port", "Broker port", "1883", 5);
    WiFiManagerParameter mqtt_broker_device_name_textfield("mqtt_device_name", "Device name", "Warden of Garage Door", 30);
    WiFiManagerParameter mqtt_broker_device_id_textfield("mqtt_device_id", "Device ID", "warden_of_garage_door", 30);
    WiFiManagerParameter mqtt_broker_username_textfield("username", "Username", "", 30);
    WiFiManagerParameter mqtt_broker_password_textfield("password", "Password", "", 30);

    wm.addParameter(&mqtt_broker_ip_textfield);
    wm.addParameter(&mqtt_broker_port_textfield);
    wm.addParameter(&mqtt_broker_device_name_textfield);
    wm.addParameter(&mqtt_broker_device_id_textfield);
    wm.addParameter(&mqtt_broker_username_textfield);
    wm.addParameter(&mqtt_broker_password_textfield);

    Serial.println("Finishing Config, starting wifi manager");
    
    bool res = wm.autoConnect("WardenOfTheGarageDoor", "wardenpass");

    if (!res) {
        Serial.println("wifi fail to config, restart");
        ESP.restart();
    }

    strcpy(mqtt_broker, mqtt_broker_ip_textfield.getValue());
    mqtt_port = atoi(mqtt_broker_port_textfield.getValue());
    strcpy(device_name, mqtt_broker_device_name_textfield.getValue());
    strcpy(device_id, mqtt_broker_device_id_textfield.getValue());
    strcpy(username, mqtt_broker_username_textfield.getValue());
    strcpy(password, mqtt_broker_password_textfield.getValue());

    topic_prefix  = "device/";
    topic_prefix  += mqtt_broker_device_id_textfield.getValue();
    topic_prefix  = "/";
    

    xTaskCreatePinnedToCore(
        &wifi_check_reconnect_loop,
        "WIFI Reconnect Task",
        2048,
        NULL,
        1,
        &wifi_reconnect_th,
        app_cpu
    );

    xTaskCreatePinnedToCore(
        &mqtt_task,
        "MQTT Task",
        10240,
        NULL,
        1,
        &mqtt_loop_th,
        app_cpu
    );

    xTaskCreatePinnedToCore(
        &sample_loop,
        "Range Finder",
        2048,
        NULL,
        1,
        &sample_loop_th,
        app_cpu
    );
    Serial.println("Creating all the x task");

    

}

void loop() {
    // Serial.println("monitor reset in a loop");
    if (digitalRead(RESET_PIN) == LOW) {
        Serial.println("reset trigger");
        for (unsigned short i = 0; i < 10; i++) { // signal where reset is triggered
            digitalWrite(BUILTIN_LED, HIGH);
            vTaskDelay(50 / portTICK_PERIOD_MS);
            digitalWrite(BUILTIN_LED, LOW);
            vTaskDelay(50 / portTICK_PERIOD_MS);
        }
        wm.resetSettings();
        ESP.restart();
    }

    vTaskDelay(1000 / portTICK_PERIOD_MS);
}






