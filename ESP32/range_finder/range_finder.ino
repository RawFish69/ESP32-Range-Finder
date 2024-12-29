/*
ESP32 Range Finder - TOF Mode
Author: RawFish69
Board: ESP32
Devices: VL53L1X, OLED
*/


#include <Wire.h>
#include <Adafruit_VL53L1X.h>
#include <WiFi.h>
#include <WebSocketsServer.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include "web_page.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define MAX_DISTANCE 4000
#define I2C_SDA 19
#define I2C_SCL 4

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_VL53L1X lox = Adafruit_VL53L1X();
WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    switch(type) {
        case WStype_DISCONNECTED:
            if (Serial) Serial.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_CONNECTED:
            if (Serial) Serial.printf("[%u] Connected!\n", num);
            break;
        case WStype_TEXT:
            break;
    }
}

void setup() {
    if (Serial) {
        Serial.begin(115200);
        Serial.println("USB Connected");
    }
    Wire.begin(I2C_SDA, I2C_SCL);
    
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        if (Serial) Serial.println(F("SSD1306 allocation failed"));
        while(1) {
            digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
            delay(500);
        }
    }
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.println("Initializing...");
    display.display();

    if (!lox.begin(0x29)) {
        if (Serial) Serial.println("Failed to find sensor");
        display.clearDisplay();
        display.setCursor(0,0);
        display.println("ToF Sensor Error!");
        display.display();
        while(1) {
            digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
            delay(500);
        }
    }

    lox.startRanging();
    lox.setTimingBudget(50000);

    IPAddress local_IP(192,168,1,101);
    IPAddress gateway(192,168,1,1);
    IPAddress subnet(255,255,255,0);

    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(local_IP, gateway, subnet);
    WiFi.softAP("O_O", "");
    
    if (Serial) {
        Serial.print("AP IP address: ");
        Serial.println(WiFi.softAPIP());
    }

    display.clearDisplay();
    display.setCursor(0,0);
    display.println("WiFi: O_O");
    display.print("IP: ");
    display.println(WiFi.softAPIP());
    display.display();
    delay(2000);

    server.on("/", []() {
        server.send(200, "text/html", WEBPAGE);
    });
    
    server.begin();
    webSocket.onEvent(webSocketEvent);
    webSocket.begin();
}

float millimetersToInches(float mm) {
    return mm / 25.4;
}

float millimetersToMeters(float mm) {
    return mm / 1000.0;
}

float millimetersToFeet(float mm) {
    return millimetersToInches(mm) / 12.0;
}

void drawFittedText(const String& text, int size, int yPos) {
    display.setTextSize(size);
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
    
    if (w > SCREEN_WIDTH) {
        display.setTextSize(size - 1);
    }
    display.setCursor(0, yPos);
    display.println(text);
}

void drawDistanceBar(int distance) {
    const int BAR_HEIGHT = 6;
    const int BAR_Y = SCREEN_HEIGHT-BAR_HEIGHT-2;
    int barWidth;
    
    if (distance < 0 || distance > MAX_DISTANCE) {
        barWidth = SCREEN_WIDTH;
    } else {
        barWidth = map(distance, 0, MAX_DISTANCE, 0, SCREEN_WIDTH);
    }
    
    display.fillRect(0, BAR_Y, barWidth, BAR_HEIGHT, WHITE);
}

void loop() {
    webSocket.loop();
    server.handleClient();

    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate > 200) {
        lastUpdate = millis();
        
        if (lox.dataReady()) {
            int distance = lox.distance();
            lox.clearInterrupt();
            
            StaticJsonDocument<200> doc;
            if (distance > 0 && distance < MAX_DISTANCE) {
                doc["distance"] = distance;
            } else {
                doc["distance"] = -1;
            }
            
            String jsonString;
            serializeJson(doc, jsonString);
            webSocket.broadcastTXT(jsonString);
            
            if (Serial) {
                Serial.print("Distance: ");
                if (distance > 0 && distance < MAX_DISTANCE) {
                    Serial.print(distance);
                    Serial.println(" mm");
                } else {
                    Serial.println("Out of range");
                }
            }

            display.clearDisplay();
            display.setTextColor(WHITE);
            
            if (distance > 0 && distance < MAX_DISTANCE) {
                display.setTextSize(2);
                display.setCursor(0, 0);
                display.print(distance);
                display.println(" mm");
                
                display.setCursor(0, 20);
                display.print(millimetersToInches(distance), 1);
                display.println(" in");
            } else {
                display.setTextSize(2);
                display.setCursor(0, 10);
                display.println("N/A");
            }
            
            display.setTextSize(1);
            display.setCursor(0, 45);
            display.print("IP: ");
            display.print(WiFi.softAPIP());
            
            drawDistanceBar(distance > 0 ? distance : -1);
            
            display.display();
        }
    }
}
