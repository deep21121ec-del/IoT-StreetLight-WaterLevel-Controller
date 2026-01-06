#include <WiFi.h>
#include "time.h"

// Ultrasonic Sensor Pins
#define TRIG_PIN 5
#define ECHO_PIN 18

// LED Indicator Pins
#define LED_ERROR 13
#define LED_OK 2
#define LED_LOW 27       // < 20cm
#define LED_MID 26       // Between 38- 43 cm
#define LED_HIGH 25      // >= 45cm
#define RELAY_FULL 22    // > 50cm // incase of OLED Make use of D22 and change RELAY_FULL to D33

// Relay and Switch Pins
#define RELAY 14
#define SWITCH_POWER 4
#define SWITCH_INPUT 15

// Level Switch Pins
#define LEVEL_SWITCH_POWER 19
#define LEVEL_SWITCH_INPUT 23

// WiFi Credentials
const char* ssid = "wifi-name";
const char* password = "wifi-password";

// Time Settings
const char* ntpServer = "in.pool.ntp.org";
const long gmtOffset_sec = 0;
const int daylightOffset_sec = 19800; // GMT+5:30

// Relay ON/OFF Time (24-hour format)
const int RELAY_ON_HOUR = 17;//5:30pm
const int RELAY_ON_MIN = 30;
const int RELAY_OFF_HOUR = 22;//10:30pm
const int RELAY_OFF_MIN = 30;

// Tank Height Configuration
const float TANK_HEIGHT_CM = 72.0;  // Actual height of your tank
const int TANK_MAX_LEVEL_CM = 57;   // Max usable water height in tank

// Variables
long duration;
float distanceCM;
bool relayState = false;
bool timerActive = false;
bool timeInitialized = false;

void setup() {
  Serial.begin(115200);

  // Setup LEDs
  pinMode(LED_ERROR, OUTPUT);
  pinMode(LED_OK, OUTPUT);
  pinMode(LED_LOW, OUTPUT);
  pinMode(LED_MID, OUTPUT);
  pinMode(LED_HIGH, OUTPUT);
  pinMode(RELAY_FULL, OUTPUT);
  digitalWrite(RELAY_FULL,HIGH);//First Buzzer OFF

  // Startup LED indication
  digitalWrite(LED_ERROR, HIGH);
  delay(100);

  // Setup Relay and Switches
  pinMode(RELAY, OUTPUT);
  digitalWrite(RELAY, HIGH); // Relay OFF initially
  pinMode(SWITCH_POWER, OUTPUT);
  digitalWrite(SWITCH_POWER, HIGH);
  pinMode(SWITCH_INPUT, INPUT_PULLDOWN);

  // Setup Level Switch
  pinMode(LEVEL_SWITCH_POWER, OUTPUT);
  digitalWrite(LEVEL_SWITCH_POWER, HIGH);
  pinMode(LEVEL_SWITCH_INPUT, INPUT_PULLDOWN);

  // Setup Ultrasonic Pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Connect to WiFi
  Serial.printf("Connecting to WiFi: %s\n", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");

  // Sync Time from NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  getAndPrintLocalTime();

  if (timeInitialized) {
    digitalWrite(LED_ERROR, LOW);
    digitalWrite(LED_OK, HIGH);
  }

  // Turn off WiFi to save power
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}

void loop() {
  delay(1000);
  getAndPrintLocalTime();

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("❌ Failed to obtain time");
    digitalWrite(LED_ERROR, HIGH);
    digitalWrite(LED_OK, LOW);
    return;
  }

  int currentHour = timeinfo.tm_hour;
  int currentMin = timeinfo.tm_min;

  // Level switch logic
  if (digitalRead(LEVEL_SWITCH_INPUT) == HIGH) {
    monitorWaterLevel();
  }
  else{//No Level LED ON
    digitalWrite(LED_LOW, LOW);
    digitalWrite(LED_MID, LOW);
    digitalWrite(LED_HIGH, LOW);
    digitalWrite(RELAY_FULL, HIGH); // OFF
    delay(100);
  }

  // Time-based Relay Control
  if (currentHour == RELAY_ON_HOUR && currentMin == RELAY_ON_MIN && !relayState) {
    digitalWrite(RELAY, LOW); // ON
    relayState = true;
    timerActive = true;
    Serial.println("⏰ Relay ON (Timer)");
    delay(1000);
  } 
  else if (currentHour == RELAY_OFF_HOUR && currentMin == RELAY_OFF_MIN && relayState) {
    Serial.println("⏰ Relay OFF (Timer)");
    performRelayOffSequence();
    relayState = false;
    timerActive = false;
  }

  // Manual Relay Control
  if (digitalRead(SWITCH_INPUT) == HIGH && !relayState && !timerActive) {
    digitalWrite(RELAY, LOW);
    relayState = true;
    Serial.println("✅ Relay ON (Manual)");
    delay(1000);
  } 
  else if (digitalRead(SWITCH_INPUT) == LOW && relayState && !timerActive) {
    digitalWrite(RELAY, HIGH);
    relayState = false;
    Serial.println("❎ Relay OFF (Manual)");
    delay(1000);
  }
}

// ========== FUNCTIONS ==========

void getAndPrintLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    timeInitialized = false;
    return;
  }
  Serial.printf("🕒 Time: %02d:%02d:%02d\n", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
  timeInitialized = true;
}

void monitorWaterLevel() {
  // Send trigger pulse
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Measure echo duration
  duration = pulseIn(ECHO_PIN, HIGH);
  distanceCM = duration * 0.034 / 2;

  float waterLevel = TANK_HEIGHT_CM - distanceCM;
  int waterPercentage = (waterLevel * 100) / TANK_MAX_LEVEL_CM;

  // Print readings
  Serial.printf("📏 Distance: %.2f cm | Water Level: %.2f cm | %d%%\n", distanceCM, waterLevel, waterPercentage);

  // Reset all LEDs
  digitalWrite(LED_LOW, LOW);
  digitalWrite(LED_MID, LOW);
  digitalWrite(LED_HIGH, LOW);
  digitalWrite(RELAY_FULL, HIGH); // OFF

  // Set LED indicators based on level
  if (waterLevel < 20) {
    digitalWrite(LED_LOW, HIGH);
    Serial.println("⚠️ Status: Below 20%");
  }
  else if (waterLevel > 35 && waterLevel < 43) {
    digitalWrite(LED_MID, HIGH);
    Serial.println("💧 Status: Between 50% - 75%");
  }
  else if (waterLevel >= 45) {
    digitalWrite(LED_HIGH, HIGH);
    Serial.println("✅ Status: Above 75%");
  }

  if (waterLevel > 50) {
    digitalWrite(RELAY_FULL, LOW); // ON
    Serial.println("🛑 Status: Tank Full (Above 90%)");
  }

  Serial.println();
  delay(2000);
}

void performRelayOffSequence() {
  digitalWrite(RELAY, HIGH); delay(10000);
  digitalWrite(RELAY, LOW);  delay(20000);
  digitalWrite(RELAY, HIGH); delay(10000);
  digitalWrite(RELAY, LOW);  delay(20000);
  digitalWrite(RELAY, HIGH); delay(1000);
}
