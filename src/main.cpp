#include <WiFi.h>
#include <Arduino.h>
#include "mqtt.h"

#define RESET_PIN 0

TaskHandle_t wifi_reconnect_th, mqtt_loop_th;

char gateway[16];
char static_ip[16];
char dns[16];
char subnet[16];
char wifi_ssid[50];
char wifi_psw[20];

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

    StaticJsonDocument<2048> device_config;
    if (!load_config(device_config)) {
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        ESP.restart();
    }

    strcpy(wifi_ssid, device_config["ssid"]);
    strcpy(wifi_psw, device_config["password"]);

    strcpy(static_ip, device_config["ip"]);
    strcpy(gateway, device_config["gateway"]);
    strcpy(dns, device_config["dns"]);
    strcpy(subnet, device_config["subnet"]);
    strcpy(mqtt_broker, device_config["broker"]);
    strcpy(password, device_config["psw"]);
    mqtt_port = device_config["port"].as<int32_t>();

    Serial.println(wifi_ssid);
    Serial.println(wifi_psw);
    Serial.println(static_ip);
    Serial.println(gateway);
    Serial.println(dns);
    Serial.println(subnet);
    Serial.println(mqtt_broker);
    Serial.println(password);
    Serial.println(mqtt_port);

    IPAddress ip_static_ip, ip_subnet, ip_dns, ip_gateway;
    if (parse_ip_addr(static_ip, ip_static_ip) && parse_ip_addr(subnet, ip_subnet) 
        && parse_ip_addr(dns, ip_dns) && parse_ip_addr(gateway, ip_gateway)) {
        WiFi.config(ip_static_ip, ip_gateway, ip_subnet, ip_dns);
    }

    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    WiFi.begin(wifi_ssid, wifi_psw);
    

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
    vTaskDelete(NULL);

}

void loop() {

}






