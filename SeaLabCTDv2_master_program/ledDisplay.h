#pragma once
// /***************************************************************/
// LED control utilities for SeaLabCTDv2.
// - Maps deviceMode to NeoPixel colors
// - Blinks mode LED at startup/change
// - Implements Group-Occulting (Oc.Gp 3) beacon pattern
// Separate from battery voltage LED logic.
// /***************************************************************/
#include "battMonitoring.h"   // provides `pixel` and NeoPixel helpers

// Gate prints with your existing flag (defined in your .ino)
extern bool serialDisplay;

static inline uint32_t colorForMode(int deviceMode) {
  switch (deviceMode) {
    case 0:  return pixel.Color(0, 255, 0);      // Fast sampling — Green
    case 1:  return pixel.Color(0, 255, 0);      // 1-min         — Green
    case 2:  return pixel.Color(255, 255, 0);    // 10-min        — Yellow
    case 3:  return pixel.Color(180, 0, 255);    // Charge        — Magenta/Purple
    case 4:  return pixel.Color(0,255,255);       // BPR          — Cyan
    default: return pixel.Color(255, 0, 0);      // Unknown       — Red
  }
}

// Blink the mode color a few times at startup (or when changing modes)
static inline void blinkDeviceModeLED(int deviceMode, uint8_t blinks = 5, uint16_t on_ms = 250, uint16_t off_ms = 250) {
  const uint32_t color = colorForMode(deviceMode);

  if (serialDisplay) {
    switch (deviceMode) {
      case 0:  Serial.println("Mode 0: Fast sampling");             break;
      case 1:  Serial.println("Mode 1: 1-min interval");            break;
      case 2:  Serial.println("Mode 2: 10-min interval");           break;
      case 3:  Serial.println("Mode 3: Charge");                    break;
      case 4:  Serial.println("Mode 4: Bottom Pressure Recorder");  break;
      default: Serial.println("Unknown deviceMode at startup");     break;
    }
  }

  for (uint8_t i = 0; i < blinks; i++) {
    pixel.setPixelColor(0, color);
    pixel.show();
    delay(on_ms);
    pixel.setPixelColor(0, 0);
    pixel.show();
    delay(off_ms);
  }
}

inline void redFlash(){
  // Flash NeoPixel RED
  pixel.setPixelColor(0, pixel.Color(255, 0, 0));
  pixel.show();
  delay(500);
  pixel.clear();
  pixel.show();
  delay(500);
}

// === Beacon configuration ===
const uint32_t ON_TIME     = SECONDS(0.6);   // 600 ms
const uint32_t OFF_TIME    = SECONDS(0.4);   // 400 ms
const uint32_t GROUP_HOLD  = SECONDS(5);     // 5 s
const uint32_t PAUSE_TIME  = SECONDS(20);  // 14.8 s
const uint8_t  GROUP_SIZE  = 3;              // Oc.Gp(3)
const uint8_t  CYCLES      = 1;              // groups before pause

// 15.2 s of flashing/holding light
// 14.8 s OFF pause
// 30.0 s total per cycle

// === Beacon state (persists across calls) ===
static bool inited = false;
static uint8_t  cycle = 0;          // 0..CYCLES
static uint8_t  flash = 0;          // 0..GROUP_SIZE
static bool     led_on = false;
static unsigned long next_ms = 0;

// === Non-blocking beacon ===
static inline void tickBeacon() {
  if (!inited) {
    pinMode(BEACON_PIN, OUTPUT);
    digitalWrite(BEACON_PIN, LOW);
    inited   = true;
    cycle    = 0;
    flash    = 0;
    led_on   = false;
    next_ms  = millis();
  }

  unsigned long now = millis();
  if ((long)(now - next_ms) < 0) return;   // not time yet

  if (cycle < CYCLES) {
    // Inside a cycle: flashes then hold
    if (flash < GROUP_SIZE) {
      if (!led_on) {
        digitalWrite(BEACON_PIN, HIGH);
        led_on  = true;
        next_ms = now + ON_TIME;
      } else {
        digitalWrite(BEACON_PIN, LOW);
        led_on  = false;
        next_ms = now + OFF_TIME;
        flash++;
      }
    } else {
      // Group hold
      digitalWrite(BEACON_PIN, HIGH);
      led_on  = true;
      next_ms = now + GROUP_HOLD;
      flash   = 0;
      cycle++;
    }
  } else {
    // Pause before restarting
    digitalWrite(BEACON_PIN, LOW);
    led_on  = false;
    next_ms = now + PAUSE_TIME;
    cycle   = 0;
    flash   = 0;
  }
}