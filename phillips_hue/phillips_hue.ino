// Copyright 2017,2020 Mike Stunes.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <BH1750.h>
BH1750 lightMeter;

#include "secret.h"

const char *GROUP_URL  = "http://" BRIDGE "/api/" API_USERNAME "/groups/" ROOM;
const char *ACTION_URL = "http://" BRIDGE "/api/" API_USERNAME "/groups/" ROOM "/action";

const String SCENE_DATA = "{\"scene\": \"" SCENE "\"}";
const String OFF_DATA = "{\"on\": false}";

const int INPUT_PIN = 12;
const int LED_PIN = 22;

const unsigned int BOUNCE_DELAY_MS = 500; // ms

unsigned long lastInterrupt;  // last interrupt time
volatile int shouldTrigger = 0;

const unsigned int CONNECT_TIMEOUT_MS = 30000;  // WiFi connnection timeout (ms)

// connectToWiFi adapted from ESP32 example code. See, e.g.:
// https://github.com/espressif/arduino-esp32/blob/master/libraries/WiFi/examples/WiFiClient/WiFiClient.ino
void connectToWiFi() {
  unsigned long startTime = millis();
  Serial.println("Connecting to: " + String(SSID));

  WiFi.disconnect();
  WiFi.begin(SSID, PWD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");

    if (millis() - startTime > CONNECT_TIMEOUT_MS) {
      Serial.println();
      Serial.println("Failed to connect.");
      return;
    }
  }

  WiFi.setAutoReconnect(true);

  Serial.println();
  Serial.println("Connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  digitalWrite(LED_PIN, LOW);
  delay(100);
  digitalWrite(LED_PIN, HIGH);
}

void putJson(const char *url, String content) {
  Serial.println("putJson");
  Serial.printf("PUT %s: %s\n", url, content.c_str());

  HTTPClient http;
  http.begin(url);
  int httpCode = http.PUT(content);
  if (httpCode > 0) {
    Serial.printf("Code: %d\n", httpCode);
  } else {
    Serial.printf("Error: %s\n", http.errorToString(httpCode).c_str());
  }
  Serial.println(http.getString());
  http.end();
}

String getUrl(const char *url) {
  Serial.println("get");
  Serial.printf("GET %s\n", url);

  HTTPClient http;
  http.begin(url);
  int httpCode = http.GET();
  if (httpCode > 0) {
    Serial.printf("Code: %d\n", httpCode);
  } else {
    Serial.printf("Error: %s\n", http.errorToString(httpCode).c_str());
  }

  return http.getString();
}

void turnLightsOn() {
  Serial.println("turnLightsOn");
  putJson(ACTION_URL, SCENE_DATA);
}

void turnLightsOff() {
  Serial.println("turnLightsOff");
  putJson(ACTION_URL, OFF_DATA);
}

bool lightsOn() {
  Serial.println("lightsOn");
  String jsonBody = getUrl(GROUP_URL);

  StaticJsonDocument<4096> jsonDoc;
  deserializeJson(jsonDoc, jsonBody);
  bool isOn = jsonDoc["state"]["any_on"];
  Serial.printf("isOn: %d\n", isOn);
  return isOn;
}

void toggleLights() {
  if (lightsOn()) {
    turnLightsOff();
  } else {
    turnLightsOn();
  }
}

void setup() {
  Serial.begin(9600);
  Serial.println("Starting");

  // Initialize the I2C bus (BH1750 library doesn't do this automatically)
  Wire.begin();
  // On esp8266 you can select SCL and SDA pins using Wire.begin(D4, D3);
  // For Wemos / Lolin D1 Mini Pro and the Ambient Light shield use Wire.begin(D2, D1);
  lightMeter.begin();
  Serial.println(F("BH1750 Test begin"));

  connectToWiFi();
}

void loop() {
  float lux = lightMeter.readLightLevel();
  Serial.println(lux);
  if(lux < 10 & shouldTrigger == 0) {
     toggleLights();
     shouldTrigger = 1;
  } else {
    shouldTrigger = 0;
  }
   delay(1000);
}