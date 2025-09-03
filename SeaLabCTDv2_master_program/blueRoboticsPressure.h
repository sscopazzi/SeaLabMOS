#pragma once
// /***************************************************************/
// Blue Robotics / Keller pressure sensor interface for SeaLabCTDv2.
// - Supports MS5837-02BA, MS5837-30BA, and Keller LD-1 (100 bar)
// - Initializes sensors with retries and fluid density set for seawater
// - Samples pressure, temperature, and depth into global variables
// Controlled by bar02Bool / bar30Bool / bar100Bool flags.
// /***************************************************************/

#include <Wire.h>
#include "MS5837.h"
#include "KellerLD.h"
#include "ledDisplay.h"           // Functions for flashing the NeoPixel and orange beacon for surface assets

// Serial gating
extern bool serialDisplay;

// Sensor selection flags (defined in .ino)
extern bool bar02Bool;
extern bool bar30Bool;
extern bool bar100Bool;

// Ready flags (defined in .ino)
extern bool br02_ready;
extern bool br30_ready;
extern bool br100_ready;

// Pressure outputs (from globals.h)
extern float brPressure;     // mbar
extern float brTemperature;  // °C
extern float brDepth;        // m

#define BR_INIT_RETRIES 42
#define BR_INIT_DELAY_MS 500

// Internal sensor objects (internal linkage)
static MS5837   brSensor02;
static MS5837   brSensor30;
static KellerLD brSensor100;

// Ready flags (set by blueRoboticsPressureSetup)
bool br02_ready  = false;
bool br30_ready  = false;
bool br100_ready = false;

void brPressureSetup() {
  if (bar02Bool) {
    bool ok = false;
    for (int i = 0; i < BR_INIT_RETRIES; ++i) {
      if (brSensor02.init()) { ok = true; break; }
      if (serialDisplay) Serial.println("bar02 init failed! Check SDA/SCL.");
      redFlash();
      delay(BR_INIT_DELAY_MS);
    }
    if (ok) {
      brSensor02.setModel(MS5837::MS5837_02BA);
      brSensor02.setFluidDensity(WATER_DENSITY);
      br02_ready = true;
      if (serialDisplay) Serial.println("bar02 initialized.");
    } else {
      br02_ready = false;
      if (serialDisplay) Serial.println("bar02 not found after retries.");
    }
  }

  if (bar30Bool) {
    bool ok = false;
    for (int i = 0; i < BR_INIT_RETRIES; ++i) {
      if (brSensor30.init()) { ok = true; break; }
      if (serialDisplay) Serial.println("bar30 init failed! Check SDA/SCL.");
      redFlash();
      delay(BR_INIT_DELAY_MS);
    }
    if (ok) {
      brSensor30.setModel(MS5837::MS5837_30BA);
      brSensor30.setFluidDensity(WATER_DENSITY);
      br30_ready = true;
      if (serialDisplay) Serial.println("bar30 initialized.");
    } else {
      br30_ready = false;
      if (serialDisplay) Serial.println("bar30 not found after retries.");
    }
  }

  if (bar100Bool) {
    bool ok = false;
    for (int i = 0; i < BR_INIT_RETRIES; ++i) {
      brSensor100.init();                                   // void
      if (brSensor100.isInitialized()) { ok = true; break; } // check status
      if (serialDisplay) Serial.println("bar100 init failed! Check wiring/power.");
      redFlash();
      delay(BR_INIT_DELAY_MS);
    }
    if (ok) {
      brSensor100.setFluidDensity(WATER_DENSITY);
      br100_ready = true;
      if (serialDisplay) Serial.println("bar100 initialized.");
    } else {
      br100_ready = false;
      if (serialDisplay) Serial.println("bar100 not found after retries.");
    }
  }
  delay(200);
}

void brPressureSample() {
  if (br02_ready) {
    brSensor02.read();
    brPressure    = brSensor02.pressure();
    brTemperature = brSensor02.temperature();
    brDepth       = brSensor02.depth();
  }

  if (br30_ready) {
    brSensor30.read();
    brPressure    = brSensor30.pressure();
    brTemperature = brSensor30.temperature();
    brDepth       = brSensor30.depth();
  }

  if (br100_ready) {
    brSensor100.read();
    brPressure    = brSensor100.pressure();
    brTemperature = brSensor100.temperature();
    brDepth       = brSensor100.depth();
  }
}
