#pragma once
// /***************************************************************/
// BATTERY VOLTAGE LED MEANING
// 4.0+      GREEN     — Full
// 3.8-3.6   CYAN      — Mostly full
// 3.6-3.5   YELLOW    — Medium
// 3.5-3.35  PURPLE    — Low
// 3.35-3.2  RED       — Critical (no blink)
// <3.2      RED       — Dangerously low *FLASHES*
// ***************************************************************/

#include <Adafruit_NeoPixel.h>

#define LED PIN_NEOPIXEL
#define SLED PIN_LED

Adafruit_NeoPixel pixel(1, LED, NEO_GRB + NEO_KHZ800);

void redFlash();

// Voltage thresholds
const float battV_max = 4.2;
const float battV_min = 3.4;

// Modes
enum BatteryLEDMode { CHARGE_MODE, SAMPLE_MODE };
BatteryLEDMode currentMode = SAMPLE_MODE;

// Timestamp for boot
unsigned long bootMillis = 0;
bool ledDisabled = false;

inline void setupBatteryMonitoring(BatteryLEDMode mode) {
    currentMode = mode;
    
    pixel.begin();
    pixel.setBrightness(20);  // Fixed low power brightness
    pixel.show();
}

void readBatteryVoltage() {
  delay(5); // stability
  float v = analogRead(BATTV_PIN);
  v /= 4095.0;     // Convert raw ADC to 0–3.3 V
  v *= 3.3;        // Actual voltage at analog pin
  v *= 2.0;        // Undo the voltage divider (assumes 1:1 divider)

  // Apply system-specific correction factor
  // Measure battery voltage with multi-meter while reading from serial
  // positive from switch out to feather, negative from ground pin on feather
  // adjust correction factor until datalogger value is multi-meter value
  // this is a way to correct for now 100% knowing the exact resistor values
  #if SYSTEM_NAME == GREEN
    v *= 1.053;
  #elif SYSTEM_NAME == BLUE
    v *= 1.069;
  #elif SYSTEM_NAME == ORANGE
    v *= 1.070;
  #elif SYSTEM_NAME == WHITE
    v *= 1.056;
  #elif SYSTEM_NAME == NALGENE
    v *= 1.047;
  #elif SYSTEM_NAME == STEEL
    v *= 1.045;
  #elif SYSTEM_NAME == TWOIN
    v *= 1.038;
  #elif SYSTEM_NAME == BPR
    v *= 1.050;
  #elif SYSTEM_NAME == PRESS_ONLY
    v *= 1.047; 
  #else
    // warn in output dialog if unrecongized name
    // 1.053 is the average of all platform values
    #warning "Unknown SYSTEM_NAME; using default correction factor of 1.053"
    v *= 1.053;
  #endif

  battV = v;
}

// Map voltage to distinct color zones
void updateBatteryLED(float battV) {
    static bool turnedOffInSampleMode = false;

    if (currentMode == SAMPLE_MODE && millis() - bootMillis > 10000) {
        if (!turnedOffInSampleMode) {
            pixel.setPixelColor(0, 0);
            pixel.show();
            turnedOffInSampleMode = true;
        }
        return;
    }

    uint32_t color;

    if (battV >= 4.00) {
    color = pixel.Color(0, 255, 0);       // Green — Full
} else if (battV >= 3.80) {
    color = pixel.Color(0, 255, 255);     // Cyan — Mostly full
} else if (battV >= 3.60) {
    color = pixel.Color(255, 255, 0);     // Yellow — Medium
} else if (battV >= 3.50) {
    color = pixel.Color(180, 0, 255);     // Purple — Low
} else if (battV >= 3.20) {
    color = pixel.Color(255, 0, 0);       // Red — Critical (no blink)
} else {
    // RED — Dangerously low *FLASHES*
    while (1) {
        redFlash();   // DO NOT USE UNTIL BATTERY IS CHARGED MORE
    }
}
    // Non-blinking steady color
    pixel.setPixelColor(0, color);
    pixel.show();
}