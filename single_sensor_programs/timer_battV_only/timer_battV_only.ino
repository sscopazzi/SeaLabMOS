#include <Adafruit_TinyUSB.h>  // REQUIRED for Serial with TinyUSB on RP2040
#include <Wire.h>
#include "RTClib.h"
#include <Adafruit_NeoPixel.h>
#include <SPI.h>
#include <SdFat.h>

#define GREEN   0
#define BLUE    1
#define ORANGE  2
#define WHITE   3
#define NALGENE 4

#define SYSTEM_NAME ORANGE
// write GREEN, BLUE, ORANGE, WHITE, NALGENE

// ==========================
// DEVICE MODES FOR WAKE TIMING ONLY (no sensors)
//
// 0 = Continuous wake (no sleep)   — benchmark no-sleep current
//     >> LED shows battery voltage color every 0.5s
//
// 1 = Wake every 1 min             — short duty cycle test
//     >> LED shows battery voltage color for 10s on power-on, then turns off
//
// 2 = Wake every 10 min            — long duty cycle test
//     >> LED shows battery voltage color for 10s on power-on, then turns off
//
// 3 = Charge mode                  — LED always on, shows battery status:
//        Green        (≥ 4.20 V) — Fully charged
//        Cyan         (≥ 3.80 V and < 4.20 V)
//        Yellow       (≥ 3.60 V and < 3.80 V)
//        Purple       (≥ 3.40 V and < 3.60 V)
//        Red (steady) (≥ 3.35 V and < 3.40 V)
//        Red (blinking) (< 3.35 V) — Critically low
//
// Any other value = unknown mode (defaults to SAMPLE_MODE behavior)
// Battery voltage range: ~3.45 V (cutoff) to ~4.18 V (fully charged)
int deviceMode = 0;

// ==========================
// BATTERY MONITORING
#define vBatPIN A2
#define LED PIN_NEOPIXEL

Adafruit_NeoPixel pixel(1, LED, NEO_GRB + NEO_KHZ800);

const float vBat_max = 4.2;
const float vBat_min = 3.45;

enum BatteryLEDMode { CHARGE_MODE, SAMPLE_MODE };
BatteryLEDMode currentMode = SAMPLE_MODE;

unsigned long bootMillis = 0;
bool ledDisabled = false;
unsigned long lastPrint = 0;
const unsigned long printInterval = 500;  // milliseconds

void setupBatteryMonitoring(BatteryLEDMode mode) {
  currentMode = mode;
  bootMillis = millis();

  pixel.begin();
  delay(10);            // Required for RP2040
  pixel.setBrightness(10); // 1-3 is dark room, 50 is normal, and 255 is full
  pixel.show();
}

float readBatteryVoltage() {
  float v = analogRead(vBatPIN);
  v /= 4095.0;     // Convert raw ADC to 0–3.3 V
  v *= 3.3;        // Actual voltage at analog pin
  v *= 2.0;        // Undo the voltage divider (assumes 1:1 divider)

  // Apply system-specific correction factor
  #if SYSTEM_NAME == GREEN
    v *= 1.053;
  #elif SYSTEM_NAME == BLUE
    v *= 1.069;
  #elif SYSTEM_NAME == ORANGE
    v *= 1.070;
  #elif SYSTEM_NAME == WHITE
    v *= 1.000;
  #elif SYSTEM_NAME == NALGENE
    v *= 1.047;
  #else
    #warning "Unknown SYSTEM_NAME; using default correction factor of 1.07"
    v *= 1.07;
  #endif

  return v;
}

void updateBatteryLED(float vBat) {
  static bool turnedOffInSampleMode = false;
  static unsigned long lastBlinkTime = 0;
  static bool blinkOn = false;

  if (currentMode == SAMPLE_MODE && millis() - bootMillis > 10000) {
    if (!turnedOffInSampleMode) {
      pixel.setPixelColor(0, 0);
      pixel.show();
      turnedOffInSampleMode = true;
    }
    return;
  }

  uint32_t color;

  if (vBat >= vBat_max) color = pixel.Color(0, 255, 0);        // Green
  else if (vBat >= 3.80) color = pixel.Color(0, 255, 255); // Cyan
  else if (vBat >= 3.60) color = pixel.Color(255, 255, 0); // Yellow
  else if (vBat >= 3.40) color = pixel.Color(180, 0, 255); // Purple
  else if (vBat >= vBat_min) color = pixel.Color(255, 0, 0);   // Red (steady)
  else {
  unsigned long now = millis();
  if (now - lastBlinkTime >= 500) {
    blinkOn = !blinkOn;
    lastBlinkTime = now;
  }
    color = blinkOn ? pixel.Color(255, 0, 0) : pixel.Color(0, 0, 0);
    pixel.setPixelColor(0, color);
    pixel.show();
    return;
  }

  pixel.setPixelColor(0, color);
  pixel.show();
}

// ==========================
// SD SETUP
#define SD_CS_PIN 23
SdFat SD;
FsFile file;
SdSpiConfig sdConfig(SD_CS_PIN, DEDICATED_SPI, SD_SCK_MHZ(16), &SPI1);
bool sdAvailable = false;
String logFilename = "";  // will be set in setup()
const String csvHeader = "timestamp,vBat,deviceMode";

void logVoltage(DateTime now, float vBat) {
  if (!sdAvailable) return;

  file = SD.open(logFilename, FILE_WRITE);
  if (file) {
    file.printf("%s,%.3f,%d\n", now.timestamp(DateTime::TIMESTAMP_FULL).c_str(), vBat, deviceMode);
    file.close();
  } else {
    Serial.println("Failed to open log file!");
  }
}

// ==========================
// RTC CONFIG
RTC_DS3231 rtc;
volatile bool rtcAlarmTriggered = false;
#define RTC_INTERRUPT_PIN 25

void rtcWakeISR() {
  rtcAlarmTriggered = true;
}

void blinkOnboardLED(int times, int duration = 300) {
  pinMode(LED_BUILTIN, OUTPUT);
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(duration);
    digitalWrite(LED_BUILTIN, LOW);
    delay(duration);
  }
}

// ==========================
// SETUP
void setup() {
  Serial.begin(115200);
  unsigned long start = millis();
  while (!Serial && (millis() - start < 3000)) {
    delay(10);
  }

  Serial.println("Serial connected?");
  Serial.flush();
  delay(100);

  analogReadResolution(12);
  Wire.begin();
  rtc.begin();
  rtc.writeSqwPinMode(DS3231_OFF);

  pinMode(RTC_INTERRUPT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(RTC_INTERRUPT_PIN), rtcWakeISR, FALLING);
  rtc.clearAlarm(1);  // Only using Alarm1 now

  DateTime now = rtc.now();
  char timestampStr[20];
  sprintf(timestampStr, "%04d-%02d-%02dT%02d-%02d-%02d",
          now.year(), now.month(), now.day(),
          now.hour(), now.minute(), now.second());

  logFilename = String(timestampStr) + ".csv";

  // === Setup RTC Alarm1 based on deviceMode ===
  if (deviceMode == 1) {
    Serial.println("deviceMode1 RTC <3");
    DateTime nextAligned(now.unixtime() - now.second() + 60);
    rtc.setAlarm1(nextAligned, DS3231_A1_Second);  // <== use this

  } else if (deviceMode == 2) {
    Serial.println("deviceMode2 RTC <3");
    uint32_t t = now.unixtime();
    uint32_t next10Min = (t - (t % 600)) + 600;
    rtc.setAlarm1(DateTime(next10Min), DS3231_A1_Second);  // <== use this

  } else if (deviceMode != 0 && deviceMode != 3) {
    Serial.println("IMPROPER DEVICE MODE SELECTION — setting 3s test alarm");
    rtc.setAlarm1(now + TimeSpan(0, 0, 0, 3), DS3231_A1_Second);  // <== use this
  }


  if (!SD.begin(sdConfig)) {
    Serial.println("WARNING: SD card not found. Logging disabled.");
    sdAvailable = false;
  } else {
    sdAvailable = true;
    file = SD.open(logFilename, FILE_WRITE);
    if (file) {
      file.println(csvHeader);
      file.close();
      blinkOnboardLED(3);
    } else {
      Serial.println("Failed to open file to write header.");
    }
  }

  setupBatteryMonitoring((deviceMode == 0 || deviceMode == 3) ? CHARGE_MODE : SAMPLE_MODE);
  Serial.print("Boot complete. DeviceMode: ");
  Serial.println(deviceMode);
}

// ==========================
// LOOP
void loop() {
  if (deviceMode == 0) {
    // Fast sampling, no sleep
    if (millis() - lastPrint >= printInterval) {
      DateTime now = rtc.now();
      float vBat = readBatteryVoltage();
      updateBatteryLED(vBat);

      Serial.print("Mode ");
      Serial.print(deviceMode);
      Serial.print(" | vBat: ");
      Serial.println(vBat,3);

      logVoltage(now, vBat);
      lastPrint = millis();
    }
    return;
  }

  if (!rtcAlarmTriggered) {
    delay(50);
    return;
  }

  rtcAlarmTriggered = false;
  DateTime now = rtc.now();
  float vBat = readBatteryVoltage();

  logVoltage(now, vBat);
  updateBatteryLED(vBat);
  
  Serial.print("Wakeup ");
  Serial.print(now.timestamp(DateTime::TIMESTAMP_FULL));
  Serial.print(" | vBat: ");
  Serial.println(vBat,3);

  rtc.clearAlarm(1);

  // === Reschedule Alarm1 aligned to minute or 10-minute mark ===
  if (deviceMode == 1) {
    // Next full minute
    DateTime nextAligned(now.unixtime() - now.second() + 60);
    rtc.setAlarm1(nextAligned, DS3231_A1_Second);

  } else if (deviceMode == 2) {
    // Next 10-minute mark
    uint32_t t = now.unixtime();
    uint32_t next10Min = (t - (t % 600)) + 600;
    rtc.setAlarm1(DateTime(next10Min), DS3231_A1_Second);
  }
}