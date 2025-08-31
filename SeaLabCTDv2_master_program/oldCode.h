
// OLD USING IF ELSE STATEMENTS
// void loop() {
//   // If don't have a display, show battV with the NeoPixel
//   if (display == false){
//     if (deviceMode == 0 || deviceMode == 3) {
//       battV = readBatteryVoltage();
//       updateBatteryLED(battV);
//       ledDisabled = false;
//     } else {
//       if (millis() - bootMillis <= 7000) {
//         battV = readBatteryVoltage();
//         updateBatteryLED(battV);
//       } else if (!ledDisabled) {
//         if (serialDisplay) Serial.println("Disabling LED after 7 sec");
//         pixel.setPixelColor(0, 0);
//         pixel.show();
//         ledDisabled = true;
//       }
//     }
//   }

//   currentTime = rtc.now();

//   // === Mode 0: as fast as possible ===
//   if (deviceMode == 0) {

//     // run init once using bool
//     if (deviceModeOneInit == false){
//       display.display();
//       digitalWrite(4, HIGH);   // turn on salinity sensor
//       delay(100);
//       Serial1.print("L,1\r");  // LED ON
//       delay(2000);
//       Serial1.print("C,1\r");  // continuous sample ON
//       delay(100);
//       deviceModeOneInit   = true;
//     }

//     brPressureSample();
//     brFastTempSample();
//     salinLoopWithPC();        // interactive / continuous
//     // salinLoopWithoutPC(brFastTemp);

//     writeDataRow();

//     serialPrintValues();
//     displayMode0(); 

//     delay(100);
//     yield();
//     return;
//   }

//    else if (deviceMode == 1 || deviceMode == 2) { // Interval sampling (RTC-driven)
//     // Note: deviceMode 3 intentionally does NOT enter here, does not sample (charge only)!
//     if (!rtcAlarmFired) {
//       delay(50);
//       return;
//     }

//     brPressureSample();
//     brFastTempSample();
//     salinLoopWithoutPC(brFastTemp);

//     writeDataRow();
//     serialPrintValues();

//     rtcAlarmFired = false;  // reset
//     rtc.clearAlarm(1);          // clear old alarm
//     if (serialDisplay) Serial.println("RTC Alarm fired");

//     DateTime now = rtc.now();
//     if (deviceMode == 1) {  // Re-arm: 1 min
//       if (serialDisplay) Serial.println("Alarm 1-min trigger");
//       uint32_t nextMinute = now.unixtime() - now.second() + 60;
//       rtc.setAlarm1(DateTime(nextMinute), DS3231_A1_Second);

//     } else if (deviceMode == 2) {  // Re-arm: 10 min
//       if (serialDisplay) Serial.println("Alarm 10-min trigger");
//       uint32_t next10Min = (now.unixtime() - (now.unixtime() % 600)) + 600;
//       rtc.setAlarm1(DateTime(next10Min), DS3231_A1_Second);
//     }
//   }
//   yield();
// }














// --- Group-Occulting (Oc.Gp 3) with pause ---
// Runs 5 full cycles, then pauses OFF for PAUSE_TIME
// Long off pause for heat and power reasons

// void groupAccoulting() {
//   const uint16_t ON_TIME     = 600;    // Light interval between eclipses
//   const uint16_t OFF_TIME    = 400;    // Each eclipse (dark)
//   const uint16_t GROUP_HOLD  = 5000;   // Extra steady ON after 3rd eclipse
//   const uint16_t PAUSE_TIME  = 0;  // OFF time after 5 full cycles

//   pinMode(BEACON_PIN, OUTPUT);
//   digitalWrite(BEACON_PIN, LOW);

//   // Run 5 full Oc.Gp(3) cycles
//   for (int cycle = 0; cycle < 2; cycle++) {
//     for (int i = 0; i < 3; i++) {
//       digitalWrite(BEACON_PIN, HIGH);  // Light ON
//       delay(ON_TIME);

//       digitalWrite(BEACON_PIN, LOW);   // Eclipse (dark)
//       delay(OFF_TIME);
//     }

//     // Final hold ON so light dominates
//     digitalWrite(BEACON_PIN, HIGH);
//     delay(GROUP_HOLD);
//   }

//   // Pause OFF before repeating
//   digitalWrite(BEACON_PIN, LOW);
//   delay(PAUSE_TIME);
// }

