#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>

#define WIFI_SSID ""
#define WIFI_PASSWORD ""

#define ARDUINO_SERIAL_SPEED 9600

#define ARDUINO_SERIAL_RX_PIN 13
#define ARDUINO_SERIAL_TX_PIN 14

SoftwareSerial arduinoSerial(ARDUINO_SERIAL_RX_PIN, ARDUINO_SERIAL_TX_PIN);

void setup() {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED)
        delay(500);

    configTzTime("JST-9", "ntp.nict.jp");

    arduinoSerial.begin(ARDUINO_SERIAL_SPEED);
}

void loop() {
    if (arduinoSerial.available() > 0) {
        if (arduinoSerial.read() == 1) {
            time_t t = time(nullptr);
            struct tm *tm = localtime(&t);
            arduinoSerial.write((char *) tm, sizeof(struct tm));
        } else {
            arduinoSerial.print("Bad request");
        }
    }
}