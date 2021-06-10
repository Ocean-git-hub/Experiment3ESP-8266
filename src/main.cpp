#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>

#define WIFI_SSID ""
#define WIFI_PASSWORD ""

#define ARDUINO_SERIAL_SPEED 9600
#define PC_SERIAL_SPEED 9600

#define ARDUINO_SERIAL_RX_PIN 13
#define ARDUINO_SERIAL_TX_PIN 14

SoftwareSerial arduinoSerial(ARDUINO_SERIAL_RX_PIN, ARDUINO_SERIAL_TX_PIN);

void setup() {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED)
        delay(500);

    configTzTime("JST-9", "ntp.nict.jp");

    Serial.begin(PC_SERIAL_SPEED);
    Serial.println("Start ESP-8266");

    arduinoSerial.begin(ARDUINO_SERIAL_SPEED);
}

void loop() {
    if (arduinoSerial.available() > 0) {
        if (arduinoSerial.read() == 1) {
            time_t t = time(nullptr);
            struct tm *time = localtime(&t);
            char currentTime[12] = {};
            sprintf(currentTime, "%02d/%02d %02d:%02d",
                    time->tm_mon, time->tm_mday, time->tm_hour, time->tm_min);
            arduinoSerial.print(currentTime);
        } else {
            arduinoSerial.print("Bad request");
        }
    }
}