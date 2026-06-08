
#define vBatPIN A2

float readBatteryVoltage() {
  float v = analogRead(vBatPIN);
  v /= 4095.0;     // Convert raw ADC to 0–3.3 V
  v *= 3.3;        // Actual voltage at analog pin
  v *= 2.0;        // Undo the voltage divider (assumes 1:1 divider)

  // CORRECTION FACTOR FOR USE IN MAIN PROGRAM
  v *= 1.064;
  return v;
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  Serial.print("Boot complete. VOLTAGE ONLY.");

}

void loop() {
  float vBat = readBatteryVoltage();
  Serial.print("vBatt = ");
  Serial.println(vBat,2);
  delay(500);
}
