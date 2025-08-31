#include <Adafruit_NeoPixel.h>

#define LED PIN_NEOPIXEL

Adafruit_NeoPixel pixel(1, LED, NEO_GRB + NEO_KHZ800);

// New distinct color palette
uint32_t voltageColors[] = {
  // 0: Red        (≤ 3.59 V)
  pixel.Color(255, 0, 0),
  // 1: Purple     (3.60–3.74 V)
  pixel.Color(180, 0, 255),
  // 2: Yellow     (3.75–3.89 V)
  pixel.Color(255, 255, 0),
  // 3: Cyan       (3.90–4.04 V)
  pixel.Color(0, 255, 255),
  // 4: Green      (≥ 4.05 V)
  pixel.Color(0, 255, 0)
};

const int numColors = sizeof(voltageColors) / sizeof(voltageColors[0]);

void setup() {
  pixel.begin();
  pixel.setBrightness(40);  // fixed low brightness for field use
  pixel.show();
}

void loop() {
  for (int i = 0; i < numColors; i++) {
    pixel.setPixelColor(0, voltageColors[i]);
    pixel.show();
    delay(1000);  // hold each color for 2 seconds
  }
}
