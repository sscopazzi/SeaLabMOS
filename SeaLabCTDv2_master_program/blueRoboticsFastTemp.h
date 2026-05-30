#pragma once
// /***************************************************************/
// Blue Robotics TSYS01 fast-response temperature sensor (I²C).
// - Initializes sensor (brFastTempSetup)
// - Samples temperature into global brFastTemp (brFastTempSample)
// - Sensor powered permanently via 3.3V (Qwiic/STEMMA QT connector)
// Uses external serialDisplay flag for optional debug output.
// /***************************************************************/

#include "TSYS01.h"  // by Blue Robotics

extern bool serialDisplay;
extern float brFastTemp;

TSYS01 brFastTempSensor;

inline void brFastTempSetup() {
  // Wire.begin(); // I2C is started elsewhere; leave commented unless needed here
  Wire.beginTransmission(0x77);
  byte err = Wire.endTransmission();
  if (serialDisplay) {
    Serial.print("TSYS01 probe: ");
    Serial.println(err == 0 ? "ACK" : "NACK");
  }
  brFastTempSensor.init();
  delay(15);
  if (!brFastTempSensor.init()) {
    if (serialDisplay) {
      Serial.println("fastBRTemp failed to initialize!");
      delay(500);
    }
  }
}

inline void brFastTempSample() {
  brFastTempSensor.read();
  brFastTemp = brFastTempSensor.temperature();  // degC
}

inline void brFastTempEnduranceSample() {
  brFastTempSensor.read();
  brFastTemp = brFastTempSensor.temperature();  // degC
}