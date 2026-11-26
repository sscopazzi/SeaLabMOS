// SPDX-FileCopyrightText: 2025-2026 Aspect Wave LLC
// SPDX-License-Identifier: MIT

/***************************************************************
  Standalone DS18B20 / DS18S20 temperature reader with optional
  microSD logging.

  Sensor wiring (Plugable Terminal Sensor Adapter, pull-up jumper ON):
      A  YELLOW (DATA)  ---->  ONE_WIRE_PIN
      B  RED    (VIN)   ---->  5V / 3.3V
      C  BLACK  (GND)   ---->  GND

  Board: Adafruit Feather RP2040 Adalogger
  Onboard microSD is on SPI1 (SCK 18, MOSI 19, MISO 20), CS = GPIO23.

  Libraries: OneWire (Jim Studt, Tom Pollard), SdFat - Adafruit Fork

  This code was
  1) Made from https://github.com/sscopazzi/SeaLabMOS/blob/main/SeaLabCTDv2_master_program/dallasTemp.h
  2) Which itself is from the libraries example
  3) Made using Claude Opus 5.5 Medium

  2026.10.2 Sophie is at sea and does not have the hardware to test this. Will do later.
 ***************************************************************/

#include <OneWire.h>
#include <SPI.h>
#include "SdFat.h"   // "SdFat - Adafruit Fork"

// ----------------------- USER SETTINGS -----------------------
bool saveToSD      = true;    // true = log to microSD, false = serial only
bool serialDisplay = true;    // true = print to Serial

const uint8_t  ONE_WIRE_PIN    = 12;           // DS18B20 data pin
const uint8_t  SD_CS_PIN       = 23;           // Adalogger onboard microSD CS
const char     LOG_FILENAME[]  = "DALLAS.CSV"; // 8.3 filename
const uint32_t LOG_INTERVAL_MS = 1000;         // sample period (min ~750 ms)
const uint32_t SERIAL_BAUD     = 115200;
// -------------------------------------------------------------

OneWire ds(ONE_WIRE_PIN);

SdFs SD;
SdSpiConfig sdConfig(SD_CS_PIN, DEDICATED_SPI, SD_SCK_MHZ(16), &SPI1);

float dallasTemp = NAN;
byte  dallasAddr[8];
bool  dallasFound = false;
bool  sdReady     = false;

void setupDallasTemp() {
  delay(500);  // wait for sensor power-up
  ds.reset_search();

  dallasFound = false;
  for (int attempt = 0; attempt < 5; attempt++) {
    if (ds.search(dallasAddr)) {
      if (OneWire::crc8(dallasAddr, 7) == dallasAddr[7] &&
          (dallasAddr[0] == 0x10 || dallasAddr[0] == 0x28)) {
        dallasFound = true;
        break;
      }
    }
    ds.reset_search();
    delay(200);
  }

  if (!dallasFound) {
    if (serialDisplay) Serial.println("DS18B20 sensor not found after multiple attempts!");
    return;
  }

  if (serialDisplay) {
    Serial.print("DS18B20 found at address: ");
    for (int i = 0; i < 8; i++) {
      if (dallasAddr[i] < 0x10) Serial.print("0");
      Serial.print(dallasAddr[i], HEX);
      if (i < 7) Serial.print(":");
    }
    Serial.println();
  }
}

// Returns temperature in deg C, or NAN on failure.
float getDallasTemp() {
  if (!dallasFound) {
    dallasTemp = NAN;
    return dallasTemp;
  }

  byte data[9];

  ds.reset();
  ds.select(dallasAddr);
  ds.write(0x44);  // start conversion

  delay(750);      // 750 ms max for 12-bit

  if (!ds.reset()) {           // no presence pulse: sensor disconnected
    dallasTemp = NAN;
    return dallasTemp;
  }
  ds.select(dallasAddr);
  ds.write(0xBE);  // read scratchpad

  for (int i = 0; i < 9; i++) data[i] = ds.read();

  if (OneWire::crc8(data, 8) != data[8]) {   // corrupted read
    dallasTemp = NAN;
    return dallasTemp;
  }

  int16_t raw = (data[1] << 8) | data[0];

  if (dallasAddr[0] == 0x10) {
    // DS18S20: 9-bit, 0.5 C/LSB; extend to 1/16 C using COUNT_REMAIN
    raw = raw << 3;
    if (data[7] == 0x10) raw = (raw & 0xFFF0) + 12 - data[6];
  }

  dallasTemp = raw / 16.0;
  return dallasTemp;
}

void setupSD() {
  if (!saveToSD) return;

  if (!SD.begin(sdConfig)) {
    if (serialDisplay) Serial.println("microSD init failed - logging disabled.");
    sdReady = false;
    return;
  }

  bool isNew = !SD.exists(LOG_FILENAME);
  FsFile f = SD.open(LOG_FILENAME, FILE_WRITE);
  if (!f) {
    if (serialDisplay) Serial.println("Could not open log file - logging disabled.");
    sdReady = false;
    return;
  }
  if (isNew) f.println("millis,temp_C");
  f.close();

  sdReady = true;
  if (serialDisplay) {
    Serial.print("Logging to ");
    Serial.println(LOG_FILENAME);
  }
}

void logToSD(uint32_t t, float tempC) {
  if (!saveToSD || !sdReady) return;

  FsFile f = SD.open(LOG_FILENAME, FILE_WRITE);   // appends
  if (!f) {
    if (serialDisplay) Serial.println("SD write failed.");
    return;
  }
  f.print(t);
  f.print(",");
  if (isnan(tempC)) f.println("NaN");
  else              f.println(tempC, 4);
  f.close();   // close every sample so data survives power loss
}

void setup() {
  if (serialDisplay) {
    Serial.begin(SERIAL_BAUD);
    uint32_t start = millis();
    while (!Serial && millis() - start < 2000) {}  // don't hang without USB
  }

  setupDallasTemp();
  setupSD();
}

void loop() {
  static uint32_t lastSample = 0;
  uint32_t now = millis();

  if (now - lastSample >= LOG_INTERVAL_MS || lastSample == 0) {
    lastSample = now;

    float t = getDallasTemp();   // blocks ~750 ms

    if (serialDisplay) {
      Serial.print(now);
      Serial.print(",");
      if (isnan(t)) Serial.println("NaN");
      else          Serial.println(t, 4);
    }

    logToSD(now, t);
  }
}