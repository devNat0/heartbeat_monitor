#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>

#include "secrets.h"

#define YELLOWLED 14
#define REDLED 13

// secrets
const char* ssid = ssid_name;
const char* password = ssid_password;
const char* ntfyHost = ntfyURL;

const char* beatKey = "beat";
const char* pauseKey = "pause";

const unsigned long bootTimeOut = 1000 * 70;
const unsigned long beatTimeOut = 1000 * 2;

enum State { Boot, Running, Paused, Fail };
State state = Boot;

void ntfy(String title = "", String priority = "3", String tags = "") {
    String data = "Heartbeat";
    HTTPClient http;
    http.begin(ntfyHost);
    http.addHeader("Content-Type", "text/plain");
    http.addHeader("Title", title);
    http.addHeader("Priority", priority);
    http.addHeader("Tags", tags);
    int responseCode = http.POST(data);
    Serial.printf("HTTP Response: %d\n", responseCode);
}

unsigned long setupElapsedTime;
void setup() {
    unsigned long setupStartTime = millis();
    pinMode(YELLOWLED, OUTPUT);
    pinMode(REDLED, OUTPUT);
    Serial.begin(9600);

    WiFi.mode(WIFI_STA);
    WiFi.hostname("heartbeat");
    WiFi.begin(ssid, password);
    Serial.println("Connecting to WiFi ..");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }
    Serial.println(WiFi.localIP());
    setupElapsedTime = millis() - setupStartTime;
    Serial.printf("Boot took: %d\n", setupElapsedTime);
}

void loop() {
    static bool isBootFailure = false;
    if (state == Boot) {
        digitalWrite(YELLOWLED, LOW);
        digitalWrite(REDLED, LOW);
        Serial.println("Waiting for boot");
        if (bootTimeOut > setupElapsedTime) {
            Serial.setTimeout(bootTimeOut - setupElapsedTime);
        }
        String input = Serial.readStringUntil('\0');
        Serial.printf("Done reading:%s\n", input);
        Serial.println(input.length());

        if (input == beatKey) {
            Serial.println("Boot complete");
            ntfy("Boot complete!");
            state = Running;
        } else {
            isBootFailure = true;
            state = Fail;
        }
    } else if (state == Running) {
        digitalWrite(YELLOWLED, LOW);
        digitalWrite(REDLED, LOW);
        Serial.setTimeout(beatTimeOut);
        String input = Serial.readStringUntil('\0');
        if (input == beatKey) {
            Serial.println("beep");
            digitalWrite(REDLED, HIGH);
            delay(200);
        } else if (input == pauseKey) {
            state = Paused;
        } else {
            state = Fail;
        }
    } else if (state == Paused) {
        digitalWrite(YELLOWLED, HIGH);
        digitalWrite(REDLED, LOW);
        ntfy("Paused", "2");
        while (Serial.available() == 0) {
        }
        String input = Serial.readStringUntil('\0');
        if (input == beatKey) {
            ntfy("Unpaused", "2");
            state = Running;
        }
    } else if (state == Fail) {
        digitalWrite(YELLOWLED, HIGH);
        digitalWrite(REDLED, HIGH);
        if (isBootFailure) {
            Serial.println("Boot timeout");
            ntfy("Boot Failed", "5", "rotating_light,file_cabinet");
        } else {
            Serial.println("Pulse timeout");
            ntfy("Pulse timed out", "5", "stop_sign,file_cabinet");
        }
        while (Serial.available() == 0) {
        }
        String input = Serial.readStringUntil('\0');
        if (input == beatKey) {
            ntfy("Resumed", "2");
            isBootFailure = false;
            state = Running;
        }
    }
}
