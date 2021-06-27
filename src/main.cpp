#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

#define WIFI_SSID "TP-LINK_Extender_2.4GHz"
#define WIFI_PASSWORD "6806147117"

#define WEATHER_ENDPOINT "http://192.168.1.73/onecall.json"
//#define WEATHER_ENDPOINT "https://api.openweathermap.org/data/2.5/onecall?lat=35.6907&lon=140.0216&exclude=current,minutely,daily,alerts&units=metric&appid=7aa81065e40021a999cec2ea8bc9d55c"

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
    Serial.begin(9600);
}

void sendTime() {
    time_t t = time(nullptr);
    struct tm *tm = localtime(&t);
    arduinoSerial.write((char *) tm, sizeof(struct tm));
}

HTTPClient httpClient;
WiFiClient client;

StaticJsonDocument<20000> weatherJsonData;

void sendWeather() {
    httpClient.begin(client, WEATHER_ENDPOINT);
    if (httpClient.GET() == HTTP_CODE_OK) {

        DeserializationError error = deserializeJson(weatherJsonData, httpClient.getString());
        if (error) {
            Serial.println(error.f_str());
            return;
        }
        Serial.println(weatherJsonData["hourly"][0]["weather"][0]["main"].as<const char *>());
    }
    httpClient.end();
}

void loop() {
    if (Serial.available() > 0) {
        switch (Serial.read()) {
            case '1':
                sendTime();
                break;
            case '2':
                sendWeather();
                break;
            default:
                arduinoSerial.print("Bad request");
                break;
        }
    }
}
