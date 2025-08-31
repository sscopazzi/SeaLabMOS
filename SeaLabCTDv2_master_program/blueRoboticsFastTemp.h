#pragma once
// /***************************************************************/
// Blue Robotics TSYS01 fast-response temperature sensor (I²C).
// - Initializes sensor (brFastTempSetup)
// - Samples temperature into global brFastTemp (brFastTempSample)
// Uses external serialDisplay flag for optional debug output.
// /***************************************************************/

#include "TSYS01.h"

extern bool serialDisplay;
extern float brFastTemp;

TSYS01 brFastTempSensor;

inline void brFastTempSetup() {
  // Wire.begin(); // I2C is started elsewhere; leave commented unless needed here
  if (!brFastTempSensor.init()) {
    if (serialDisplay) {
      Serial.println("fastBRTemp failed to initialize!");
      delay(500);
    }
  }
}

inline void brFastTempSample() {
  brFastTempSensor.read();
  brFastTemp = brFastTempSensor.temperature(); // degC
}
