#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

#define WIFI_SSID ""
#define WIFI_PASSWORD ""

#define WEATHER_ENDPOINT "http://api.openweathermap.org/data/2.5/onecall?lat=35.6907&lon=140.0216&exclude=current,minutely,daily,alerts&units=metric&appid="

#define ARDUINO_SERIAL_SPEED 9600

#define ARDUINO_SERIAL_RX_PIN 13
#define ARDUINO_SERIAL_TX_PIN 14

SoftwareSerial arduinoSerial(ARDUINO_SERIAL_RX_PIN, ARDUINO_SERIAL_TX_PIN);

HTTPClient httpClient;
WiFiClient client;

StaticJsonDocument<20000> weatherJsonData;
DeserializationError jsonDeserializationError;

void getWeather() {
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

void getTime() {
    configTzTime("JST-9", "ntp.nict.jp");
}

uint8_t getHoursLater() {
    time_t t = time(nullptr);
    int hour = localtime(&t)->tm_hour;
    if (hour >= 0 && hour <= 7)
        return 9 - hour;
    else if (hour >= 8 && hour <= 11)
        return 12 - hour;
    else if (hour >= 12 && hour <= 15)
        return 16 - hour;
    else if (hour >= 16 && hour <= 18)
        return 19 - hour;
    else if (hour >= 19 && hour <= 23)
        return 9 + 24 - hour;
    return 255;
}

void sendWeather() {
    int hoursLater = getHoursLater();
    if (hoursLater != 255 && jsonDeserializationError == STATUS::OK)
        arduinoSerial.print(weatherJsonData["hourly"][hoursLater]["weather"][0]["main"].as<const char *>());
    else
        arduinoSerial.print("Error");
}

void sendTime() {
    time_t t = time(nullptr);
    struct tm *times = localtime(&t);
    arduinoSerial.write((char *) times, sizeof(struct tm));
}

void timer0ISR() {
    static uint16 count;
    if (++count > 3600) {
        getTime();
        getWeather();
        count = 0;
    }
    timer0_write(ESP.getCycleCount() + 80000000);
}

void setup() {
    arduinoSerial.begin(ARDUINO_SERIAL_SPEED);
    Serial.begin(9600);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED)
        delay(500);

    Serial.println("Connectd");
    getTime();
    getWeather();

    timer0_isr_init();
    timer0_attachInterrupt(timer0ISR);
    timer0_write(ESP.getCycleCount() + 80000000);
}

void loop() {
    if (arduinoSerial.available() > 0) {
        switch (arduinoSerial.read()) {
            case 1:
                sendTime();
                break;
            case 2:
                sendWeather();
                break;
            default:
                arduinoSerial.print("Bad request");
                break;
        }
    }
}
