#include <Wire.h>
#include "TSYS01.h"

TSYS01 fastBRTempSensor;
float fastBRTemp;

void setup() {
  Serial.begin(115200);
  Wire.begin();

  while (!Serial);  // Wait for Serial monitor (optional, useful for debugging)

  // Attempt to initialize the sensor
  while (!fastBRTempSensor.init()) {
    Serial.println("fastBRTemp failed to initialize!");
    Serial.println("CHECK fastBRTemp sensor connection.");
    Serial.println("Is it plugged into the I2C QWIIC ports?");
    delay(2000);
  }

  Serial.println("fastBRTemp sensor initialized successfully!");
}

void loop() {
  fastBRTempSensor.read();
  fastBRTemp = fastBRTempSensor.temperature();  // °C

  Serial.print("Temperature (°C): ");
  Serial.println(fastBRTemp);

  delay(500);  // 1 second delay between readings
}
