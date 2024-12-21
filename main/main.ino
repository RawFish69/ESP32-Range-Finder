/*
ESP32 Range Finder
Author: RawFish69
Board: ESP32C3 Dev Module
Sensors: VL53L1X, HC-SR04
*/

#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include "web_main.h"
#include <rgb.h>
#include <Adafruit_VL53L1X.h>

#define TRIG_PIN 5
#define ECHO_PIN 6
#define SERVO_PIN 1
#define SDA 19
#define SCL 4

const int SERVO_FREQ = 50;
const int SERVO_PWM_BITS = 12;
const int SERVO_MIN_US = 500;
const int SERVO_MAX_US = 2500;

#define SERVO_MIN_DUTY ( (SERVO_MIN_US * (1 << SERVO_PWM_BITS)) / 20000 )
#define SERVO_MAX_DUTY ( (SERVO_MAX_US * (1 << SERVO_PWM_BITS)) / 20000 )
#define SERVO_NEUTRAL  ( (SERVO_MIN_DUTY + SERVO_MAX_DUTY) / 2 )

unsigned int angleToDuty(int angle) {
  long pulseUs = SERVO_MIN_US + (long)angle * (SERVO_MAX_US - SERVO_MIN_US) / 180;
  return (pulseUs * (1 << SERVO_PWM_BITS)) / 20000;
}

const char* ssid = "Range Finder Beta";
const char* password = "";
IPAddress local_IP(192, 168, 1, 101);
IPAddress gateway(192, 168, 1, 101);
IPAddress subnet(255, 255, 255, 0);
WebServer server(80);
WebSocketsServer webSocket(81);
Adafruit_VL53L1X tofSensor;
bool sensorsInitialized = false;
int servoAngle = 0;
bool servoDirection = true;
const int SERVO_STEP_DELAY = 20;
const int SERVO_STEP_DEGREES = 2;
const int ULTRASONIC_MAX_CM = 200;
int tofDistance = -1;
const int TOF_MAX_DISTANCE = 4000;
unsigned long lastToFRead = 0;
const unsigned long TOF_READ_INTERVAL = 50;

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA, SCL);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  ledcAttach(SERVO_PIN, SERVO_FREQ, SERVO_PWM_BITS);
  ledcWrite(SERVO_PIN, SERVO_NEUTRAL);
  if (!tofSensor.begin(0x29)) {
    Serial.println("Failed to init ToF sensor!");
  } else {
    tofSensor.startRanging();
    tofSensor.setTimingBudget(50000);
    sensorsInitialized = true;
    Serial.println("ToF sensor OK");
  }
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAP(ssid, password);
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", WEBPAGE);
  });
  server.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  setupRGB();
}

void loop() {
  server.handleClient();
  webSocket.loop();
  static unsigned long lastServoMove = 0;
  unsigned long now = millis();
  if (now - lastServoMove >= SERVO_STEP_DELAY) {
    lastServoMove = now;
    if (servoDirection) {
      servoAngle += SERVO_STEP_DEGREES;
      if (servoAngle >= 180) {
        servoAngle = 180;
        servoDirection = false;
      }
    } else {
      servoAngle -= SERVO_STEP_DEGREES;
      if (servoAngle <= 0) {
        servoAngle = 0;
        servoDirection = true;
      }
    }
    unsigned int duty = angleToDuty(servoAngle);
    ledcWrite(SERVO_PIN, duty);
    int ultrasonicCm = getUltrasonicDistance();
    if (ultrasonicCm < 0 || ultrasonicCm > ULTRASONIC_MAX_CM) {
      ultrasonicCm = ULTRASONIC_MAX_CM;
    }
    if (sensorsInitialized && (now - lastToFRead >= TOF_READ_INTERVAL)) {
      lastToFRead = now;
      if (tofSensor.dataReady()) {
        int dist = tofSensor.distance();
        tofSensor.clearInterrupt();
        if (dist < 0 || dist > TOF_MAX_DISTANCE) {
          dist = TOF_MAX_DISTANCE;
        }
        tofDistance = dist;
      }
    }
    StaticJsonDocument<256> doc;
    doc["angle"] = servoAngle;
    doc["range"] = ultrasonicCm;
    doc["tofDistance"] = tofDistance;
    String json;
    serializeJson(doc, json);
    webSocket.broadcastTXT(json);
    Serial.print("Angle: ");
    Serial.print(servoAngle);
    Serial.print("°, Ultrasonic: ");
    Serial.print(ultrasonicCm);
    Serial.print(" cm, ToF: ");
    Serial.print(tofDistance);
    Serial.println(" mm");
  }
  handleRGB();
}

int getUltrasonicDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  if (duration == 0) {
    return ULTRASONIC_MAX_CM;
  }
  int distance = duration * 0.034 / 2;
  return distance;
}
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  if (type == WStype_TEXT) {
  }
}
