#pragma once
// /***************************************************************/
// Blue Robotics TSYS01 fast-response temperature sensor (I²C).
// - Initializes sensor (brFastTempSetup)
// - Samples temperature into global brFastTemp (brFastTempSample)
// - Sensor powered permanently via 3.3V (Qwiic/STEMMA QT connector)
// Uses external serialDisplay flag for optional debug output.
// HALTS with red NeoPixel flash if the sensor is not detected at
// boot (matches SD card and RTC failure behavior).
// NOTE: ledDisplay.h must be included before this file in the .ino
//       so redFlash() is visible (it already is).
// /***************************************************************/
#include "TSYS01.h"  // by Blue Robotics

extern bool serialDisplay;
extern float brFastTemp;

TSYS01 brFastTempSensor;

inline void brFastTempSetup() {
  // Probe the bus first — requestFrom() inside init() returns 0 on NACK,
  // but the probe gives an unambiguous presence check before touching PROM.
  Wire.beginTransmission(0x77);
  byte err = Wire.endTransmission();
  if (serialDisplay) {
    Serial.print("Blue Robotics Fast-Response TSYS01: ");
    Serial.println(err == 0 ? "ACK" : "NACK");
  }

  // init() resets the chip and reads the 8 PROM calibration words;
  // returns false if zero bytes were received. One call only.
  bool initOK = brFastTempSensor.init();

  if (err != 0 || !initOK) {
    if (serialDisplay) {
      Serial.println("TSYS01 NOT DETECTED — no fast temp possible. Halting...");
      Serial.flush();
    }
    while (true) {
      redFlash();  // same hard-stop behavior as missing SD card / RTC
    }
  }

  if (serialDisplay) Serial.println("TSYS01 init OK");
}

inline void brFastTempSample() {
  brFastTempSensor.read();
  brFastTemp = brFastTempSensor.temperature();  // degC
}

inline void brFastTempEnduranceSample() {
  brFastTempSensor.read();
  brFastTemp = brFastTempSensor.temperature();  // degC
}