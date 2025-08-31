// Blue Robotics Bar30 (MS5837) Standalone Example
#include <Wire.h>
#include "MS5837.h"

// Sensor object
MS5837 slowBRTempPres;

// Sensor readings
float slowBRPres = 0.0;
float slowBRTemp = 0.0;
float slowBRDepth = 0.0;

void setup() {
  Serial.begin(115200);
  Wire.begin();

  Serial.println("Initializing Blue Robotics Bar30 sensor...");

  int timeoutCount = 42;
  while (!slowBRTempPres.init()) {
    Serial.println("Init failed!");
    Serial.println("Check SDA/SCL connections.");
    Serial.println("Blue Robotics Bar30: White=SDA, Green=SCL");
    Serial.print("Retrying in 1s... ["); Serial.print(timeoutCount); Serial.println(" attempts left]");

    delay(1000);
    timeoutCount--;
    if (timeoutCount <= 0) {
      Serial.println("Sensor not found. Halting.");
      while (true);  // Stop here
    }
  }

  slowBRTempPres.setModel(MS5837::MS5837_30BA);
  slowBRTempPres.setFluidDensity(1025);  // Seawater density

  Serial.println("Sensor initialized successfully.");
}

void loop() {
  slowBRTempPres.read();

  slowBRPres  = slowBRTempPres.pressure();     // mbar
  slowBRTemp  = slowBRTempPres.temperature();  // °C
  slowBRDepth = slowBRTempPres.depth();        // meters

  Serial.print("Pressure (mbar): "); Serial.print(slowBRPres);
  Serial.print(" | Temperature (°C): "); Serial.print(slowBRTemp);
  Serial.print(" | Depth (m): "); Serial.println(slowBRDepth);

  delay(1000);  // 1 second interval
}
