// ============================================================
// RTC_FastTemp_Test.ino
// DS3231 RTC  on Wire  — GPIO 2 (SDA)  / GPIO 3 (SCL)   [I2C1, default]
// TSYS01 fast temp on Wire1 — GPIO 12 (SDA) / GPIO 13 (SCL) [I2C0]
//   Wire your TSYS01 SDA -> D10, SCL -> D11 on the Feather header
//
// TSYS01 powered from GPIO 5, fully off between samples.
// RTC alarm wakes from sleep every WAIT_MINUTES (mode 1 style).
// ============================================================

#include <Arduino.h>
#include <Wire.h>
#include <RTClib.h>   // Adafruit RTClib
#include "TSYS01.h"   // Blue Robotics TSYS01

#include "pico/stdlib.h"
#include "hardware/sync.h"  // __wfi()

// ---------------------------------------------------------------
// USER CONFIG
// ---------------------------------------------------------------
#define BR_SENSOR_POWER_PIN   5     // GPIO powering TSYS01 VCC
#define RTC_INTERRUPT_PIN     25    // DS3231 SQW/INT pin
#define WAIT_MINUTES          1     // alarm interval
#define SYSTEM_BAUD           115200
#define TIME_ZONE             -7    // local offset for compile-time UTC set

// Wire1 pins — D10 = GPIO10 (SDA1), D11 = GPIO11 (SCL1)
#define WIRE1_SDA_PIN  10
#define WIRE1_SCL_PIN  11
// ---------------------------------------------------------------

RTC_DS3231 rtc;
TSYS01     fastTempSensor(&Wire1);  // TSYS01 on Wire1

volatile bool rtcAlarmFired = false;
void rtcWakeISR() { rtcAlarmFired = true; }

// ---------------------------------------------------------------
// Next alarm aligned to WAIT_MINUTES boundary
// ---------------------------------------------------------------
DateTime getNextAlarm(DateTime now, uint8_t waitMinutes) {
  uint8_t  nextMinute = ((now.minute() / waitMinutes) + 1) * waitMinutes;
  uint8_t  nextHour   = now.hour();
  uint8_t  nextDay    = now.day();
  uint8_t  nextMonth  = now.month();
  uint16_t nextYear   = now.year();

  if (nextMinute >= 60) {
    nextMinute %= 60;
    nextHour++;
    if (nextHour >= 24) { nextHour = 0; nextDay++; }
  }
  return DateTime(nextYear, nextMonth, nextDay, nextHour, nextMinute, 0);
}

// ---------------------------------------------------------------
// Set RTC from compile-time, adjusted to UTC
// ---------------------------------------------------------------
void setRtcCompileTimeUTC() {
  const char* months = "JanFebMarAprMayJunJulAugSepOctNovDec";
  char monthStr[4];
  int month, day, year, hour, minute, second;

  sscanf(__DATE__, "%3s %d %d", monthStr, &day, &year);
  month = (strstr(months, monthStr) - months) / 3 + 1;
  sscanf(__TIME__, "%d:%d:%d", &hour, &minute, &second);

  hour -= TIME_ZONE;

  int daysInMonth[] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
  bool isLeap = (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
  if (isLeap) daysInMonth[1] = 29;

  while (hour < 0) {
    hour += 24; day--;
    if (day < 1) {
      month--;
      if (month < 1) { month = 12; year--; }
      day = daysInMonth[month - 1];
    }
  }
  while (hour >= 24) {
    hour -= 24; day++;
    if (day > daysInMonth[month - 1]) {
      day = 1; month++;
      if (month > 12) { month = 1; year++; }
    }
  }

  rtc.adjust(DateTime(year, month, day, hour, minute, second));
  Serial.println("RTC set to compile-time UTC.");
}

// ---------------------------------------------------------------
// Power-cycle TSYS01 on Wire1 and take one reading
// ---------------------------------------------------------------
float sampleFastTemp() {
  digitalWrite(BR_SENSOR_POWER_PIN, HIGH);
  delay(50);

  if (!fastTempSensor.init()) {
    Serial.println("  TSYS01 init FAILED!");
    digitalWrite(BR_SENSOR_POWER_PIN, LOW);
    return NAN;
  }

  fastTempSensor.read();
  delay(40);
  float t = fastTempSensor.temperature();

  digitalWrite(BR_SENSOR_POWER_PIN, LOW);
  return t;
}

// ---------------------------------------------------------------
// setup()
// ---------------------------------------------------------------
void setup() {
  set_sys_clock_khz(50000, true);
  delay(5);
  yield();

  Serial.begin(SYSTEM_BAUD);
  delay(3000);
  Serial.println("=== RTC + FastTemp Diagnostic ===");
  Serial.println("RTC  -> Wire  (GPIO 2 SDA / GPIO 3 SCL)");
  Serial.println("TSYS01 -> Wire1 (GPIO 12 SDA / GPIO 13 SCL)");

  // TSYS01 power off before touching any I2C bus
  pinMode(BR_SENSOR_POWER_PIN, OUTPUT);
  digitalWrite(BR_SENSOR_POWER_PIN, LOW);
  delay(100);

  // Wire  — RTC, 100 kHz
  Wire.begin();
  Wire.setClock(100000);
  delay(50);

  // Wire1 — TSYS01, explicit pins, 100 kHz
  Wire1.setSDA(WIRE1_SDA_PIN);
  Wire1.setSCL(WIRE1_SCL_PIN);
  Wire1.begin();
  Wire1.setClock(100000);
  delay(50);

  // --- RTC on Wire ---
  if (!rtc.begin()) {
    Serial.println("ERROR: DS3231 not found on Wire (GPIO2/GPIO3)!");
    Serial.println("Check wiring and pull-ups on SDA/SCL.");
    while (1) { delay(500); Serial.println("Halted — no RTC."); }
  }
  Serial.println("RTC found OK.");

  rtc.writeSqwPinMode(DS3231_OFF);

  if (rtc.lostPower()) {
    Serial.println("RTC lost power — setting from compile time.");
  }
  setRtcCompileTimeUTC(); // comment out once time is confirmed correct

  rtc.clearAlarm(1); delay(2);
  rtc.clearAlarm(2); delay(2);

  pinMode(RTC_INTERRUPT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(RTC_INTERRUPT_PIN), rtcWakeISR, FALLING);

  // --- TSYS01 test read on Wire1 ---
  Serial.println("Testing TSYS01 on Wire1 (GPIO10/GPIO11)...");
  float testTemp = sampleFastTemp();
  if (isnan(testTemp)) {
    Serial.println("WARNING: TSYS01 returned NaN — check D10/D11 wiring!");
  } else {
    Serial.print("TSYS01 test read: ");
    Serial.print(testTemp, 2);
    Serial.println(" C  — OK");
  }

  // --- First alarm ---
  DateTime now = rtc.now();
  Serial.print("RTC time: ");
  Serial.println(now.timestamp(DateTime::TIMESTAMP_FULL));

  DateTime firstAlarm = getNextAlarm(now, WAIT_MINUTES);
  rtc.setAlarm1(firstAlarm, DS3231_A1_Minute);

  Serial.print("First alarm: ");
  Serial.println(firstAlarm.timestamp(DateTime::TIMESTAMP_FULL));
  Serial.println("Sleeping...");
}

// ---------------------------------------------------------------
// loop()
// ---------------------------------------------------------------
void loop() {
  while (!rtcAlarmFired) {
    __wfi();
  }

  rtcAlarmFired = false;
  rtc.clearAlarm(1);

  DateTime now = rtc.now();
  float temp   = sampleFastTemp();

  Serial.print(now.timestamp(DateTime::TIMESTAMP_FULL));
  Serial.print("  brFastTemp: ");
  if (isnan(temp)) {
    Serial.println("ERROR (NaN)");
  } else {
    Serial.print(temp, 2);
    Serial.println(" C");
  }

  DateTime nextAlarm = getNextAlarm(now, WAIT_MINUTES);
  rtc.setAlarm1(nextAlarm, DS3231_A1_Minute);
  Serial.print("Next alarm: ");
  Serial.println(nextAlarm.timestamp(DateTime::TIMESTAMP_FULL));
}