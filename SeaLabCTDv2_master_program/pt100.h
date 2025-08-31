#pragma once
/***************************************************************/
// PT100 RTD temperature measurement using Adafruit MAX31865 breakout
// - Uses Adafruit_MAX31865 library (SPI-based)
// - Configured for PT100 (RNOMINAL = 100.0, RREF = 430.0)
// - Defaults to 3-wire; change to MAX31865_2WIRE or MAX31865_4WIRE if needed
// - Provides pt100Setup() and getPT100Temp()
// - Matches your working pinout (CS=10, DI=9, DO=6, CLK=5)
/***************************************************************/

#include <Adafruit_MAX31865.h>

// === Pin Configuration (software SPI) ===
#define PT100_CS   10
#define PT100_DI    9
#define PT100_DO    6
#define PT100_CLK   5

// NOTE: define this object in exactly one TU if multiple files include this header.
// If you hit multiple-definition issues, change this to `extern` here and
// put the single definition in a .ino/.cpp.
Adafruit_MAX31865 thermo = Adafruit_MAX31865(PT100_CS, PT100_DI, PT100_DO, PT100_CLK);

// === Calibration Constants ===
#define RREF      430.0   // Reference resistor (430Ω for PT100, 4300Ω for PT1000)
#define RNOMINAL  100.0   // Nominal resistance at 0 °C (100 for PT100, 1000 for PT1000)

// === Globals ===
extern float pt100Temp;   // define once elsewhere, e.g. float pt100Temp = 999.9f;

inline void pt100Setup() {
  Serial.println(F("Adafruit MAX31865 PT100 Sensor Init"));
  thermo.begin(MAX31865_3WIRE);
  // Optional quality-of-life:
  thermo.enable50Hz(true);     // or false for 60 Hz mains
  thermo.autoConvert(true);    // continuous conversions
  thermo.enableBias(true);     // keep bias on for continuous mode
  thermo.clearFault();
  pinMode(11, OUTPUT);
  digitalWrite(11, LOW);
}

inline float getPT100Temp() {
  // Clear any latched fault *first* so it doesn’t hang around
  thermo.clearFault();

  // Kick a fresh conversion path like the demo
  uint16_t rtd = thermo.readRTD();

  // Serial.print(F("RTD value: ")); Serial.println(rtd);
  // float ratio = rtd / 32768.0f;
  // Serial.print(F("Ratio = "));      Serial.println(ratio, 8);
  // Serial.print(F("Resistance = ")); Serial.println(RREF * ratio, 8);

  float tC = thermo.temperature(RNOMINAL, RREF);
  // Serial.print(F("Temperature = ")); Serial.println(tC);

  // uint8_t fault = thermo.readFault();
  // if (fault) {
  //   Serial.print(F("Fault 0x")); Serial.println(fault, HEX);
  //   if (fault & MAX31865_FAULT_HIGHTHRESH) Serial.println(F("RTD High Threshold"));
  //   if (fault & MAX31865_FAULT_LOWTHRESH)  Serial.println(F("RTD Low Threshold"));
  //   if (fault & MAX31865_FAULT_REFINLOW)   Serial.println(F("REFIN- > 0.85 x Bias"));
  //   if (fault & MAX31865_FAULT_REFINHIGH)  Serial.println(F("REFIN- < 0.85 x Bias - FORCE- open"));
  //   if (fault & MAX31865_FAULT_RTDINLOW)   Serial.println(F("RTDIN- < 0.85 x Bias - FORCE- open"));
  //   if (fault & MAX31865_FAULT_OVUV)       Serial.println(F("Under/Over voltage"));
  //   thermo.clearFault();  // fine to clear again, keeps status clean
  // }

  pt100Temp = tC;  // always update, like the demo
  return pt100Temp;
}

