// #pragma once
// /***************************************************************
//  * Atlas Scientific EZO-EC (UART) interface for SeaLabCTDv2.
//  *  - Robust Serial1 reader that avoids merged frames
//  *  - Diagnostic logging for malformed frames (hex dump)
//  *  - Conservative salvage: use last 4 tokens if >4 tokens present
//  *
//  * Drop-in replacement for your existing header.
//  ***************************************************************/

// #include <Arduino.h>

// // -----------------------------------------------------------------
// // Globals (defined elsewhere in the project)
// extern float brFastTemp;
// extern float dallasTemp;
// extern bool ecBool;
// extern bool sBool;

// // Parsed values (these names used in your original code)
// extern float ec;
// extern float tds;
// extern float sal;
// extern float grav;

// // -----------------------------------------------------------------
// // Buffer reserves
// String inputstring  = "";
// String sensorstring = "";

// // -----------------------------------------------------------------
// // Low-level: read available bytes into an internal buffer, return
// // one complete line (up to first CR/LF). Safe against multiple
// // lines being present in the UART FIFO.
// bool readECLine(String &outLine) {
//     static String rxBuf = "";
//     // Read everything that's available now
//     while (Serial1.available()) {
//         char c = (char)Serial1.read();
//         rxBuf += c;
//         // safety cap to avoid runaway memory growth
//         if (rxBuf.length() > 512) {
//             rxBuf = rxBuf.substring(rxBuf.length() - 256); // keep tail
//         }
//     }

//     // Find first newline delimiter
//     int pos = -1;
//     for (int i = 0; i < (int)rxBuf.length(); ++i) {
//         if (rxBuf[i] == '\r' || rxBuf[i] == '\n') { pos = i; break; }
//     }

//     if (pos >= 0) {
//         outLine = rxBuf.substring(0, pos);
//         // Remove extracted line + any following CR/LF chars
//         int removeLen = pos + 1;
//         while (removeLen < (int)rxBuf.length() && (rxBuf[removeLen] == '\r' || rxBuf[removeLen] == '\n')) ++removeLen;
//         rxBuf = rxBuf.substring(removeLen);
//         return true;
//     }

//     return false;
// }

// // -----------------------------------------------------------------
// // parse a single EC line safely. If it is malformed (not 3 commas),
// // we emit debug info and attempt a conservative salvage (use last 4 tokens).
// void processECLine(const String &lineIn) {
//     String line = lineIn;
//     line.trim();
//     if (line.length() == 0) return;

//     // Count commas quickly
//     int commaCount = 0;
//     for (unsigned int i = 0; i < line.length(); ++i) if (line[i] == ',') ++commaCount;

//     if (commaCount == 3) {
//         // Normal case: exactly 4 fields
//         sensorstring = line;   // keep for legacy code expectations
//         // parse safely with buffer and strtok
//         char buf[128];
//         line.toCharArray(buf, sizeof(buf));
//         char *pEC   = strtok(buf, ",");
//         char *pTDS  = strtok(NULL, ",");
//         char *pSAL  = strtok(NULL, ",");
//         char *pGRAV = strtok(NULL, ",");

//         ec   = (pEC   ? atof(pEC)   : NAN);
//         tds  = (pTDS  ? atof(pTDS)  : NAN);
//         sal  = (pSAL  ? atof(pSAL)  : NAN);
//         grav = (pGRAV ? atof(pGRAV) : NAN);
//         return;
//     }

//     // ---------- Anomaly handling ----------
//     Serial.print("EC ANOMALY (commas=");
//     Serial.print(commaCount);
//     Serial.print(") -> '");
//     Serial.print(line);
//     Serial.println("'");

//     // Hex dump (byte-wise) to help diagnose origin of extra tokens
//     Serial.print("EC ANOMALY HEX: ");
//     for (unsigned int i = 0; i < line.length(); ++i) {
//         unsigned char uc = (unsigned char)line[i];
//         if (uc < 16) Serial.print('0');           // pad to 2 digits
//         Serial.print(uc, HEX);
//         Serial.print(' ');
//     }
//     Serial.println();

//     // Attempt salvage: split into tokens and, if >=4, parse last 4 tokens
//     char buf2[256];
//     line.toCharArray(buf2, sizeof(buf2));
//     char *tokens[64];
//     int n = 0;
//     char *tok = strtok(buf2, ",");
//     while (tok && n < (int)(sizeof(tokens) / sizeof(tokens[0]))) {
//         tokens[n++] = tok;
//         tok = strtok(NULL, ",");
//     }

//     if (n >= 4) {
//         // Use the last 4 tokens as a pragmatic salvage attempt
//         char *pEC   = tokens[n - 4];
//         char *pTDS  = tokens[n - 3];
//         char *pSAL  = tokens[n - 2];
//         char *pGRAV = tokens[n - 1];

//         ec   = (pEC   ? atof(pEC)   : NAN);
//         tds  = (pTDS  ? atof(pTDS)  : NAN);
//         sal  = (pSAL  ? atof(pSAL)  : NAN);
//         grav = (pGRAV ? atof(pGRAV) : NAN);

//         Serial.println("EC ANOMALY: salvage used last 4 tokens.");
//     } else {
//         // not enough data to salvage
//         ec = tds = sal = grav = NAN;
//         Serial.println("EC ANOMALY: not enough tokens to salvage, dropping frame.");
//     }
// }

// // -----------------------------------------------------------------
// // Setup
// inline void salinSetup() {
//     Serial1.begin(SALINITY_BAUD);
//     inputstring.reserve(32);
//     sensorstring.reserve(128);
//     pinMode(SALINITY_ENABLE_PIN, OUTPUT);
//     digitalWrite(SALINITY_ENABLE_PIN, LOW);
// }

// // -----------------------------------------------------------------
// // Pass-through mode (PC <-> probe). Processes ALL available EC frames
// inline void salinLoopWithPC() {
//     // PC -> probe
//     if (Serial.available()) {
//         inputstring = Serial.readStringUntil('\r');
//         Serial1.print(inputstring);
//         Serial1.print('\r');
//         inputstring = "";
//     }

//     // Probe -> PC / parser: process every complete line present
//     String line;
//     while (readECLine(line)) {
//         if (line.length() == 0) continue;
//         if (isdigit(line[0])) {
//             // numeric data line
//             processECLine(line);
//             // optional debug print of the parsed values
//             Serial.print("PARSED: ec=");
//             Serial.print(ec);
//             Serial.print(" tds=");
//             Serial.print(tds);
//             Serial.print(" sal=");
//             Serial.print(sal);
//             Serial.print(" grav=");
//             Serial.println(grav);
//         } else {
//             // status / calibration / textual messages
//             Serial.println(line);
//         }
//     }
// }

// // -----------------------------------------------------------------
// // Debug raw dump (hex + ascii) of Serial1 bytes (call manually)
// inline void debugSerial1Raw() {
//     while (Serial1.available()) {
//         int b = Serial1.read();
//         Serial.print("0x");
//         if (b < 16) Serial.print('0');
//         Serial.print(b, HEX);
//         Serial.print(" '");
//         if (b >= 32 && b < 127) Serial.print((char)b); else Serial.print('.');
//         Serial.println("'");
//     }
// }

// // -----------------------------------------------------------------
// // Autonomous mode (no PC). Processes all available EC frames
// inline void salinLoopWithoutPC(float tempCForComp) {
//     // Optionally send temperature compensation before reading (if used)
//     // Serial1.print("T," + String(tempCForComp, 2) + "\r");

//     String line;
//     while (readECLine(line)) {
//         if (line.length() == 0) continue;
//         if (isdigit(line[0])) {
//             processECLine(line);
//         } else {
//             Serial.println(line);
//         }
//     }
// }


















// inline void print_EC_data(void) {  //this function will pars the string

//   char sensorstring_array[30];  //we make a char array
//   char *EC;                     //char pointer used in string parsing
//   char *TDS;                    //char pointer used in string parsing
//   char *SAL;                    //char pointer used in string parsing
//   char *GRAV;                   //char pointer used in string parsing
//   // float f_ec;                                         //used to hold a floating point number that is the EC

//   sensorstring.toCharArray(sensorstring_array, 30);  //convert the string to a char array
//   // Serial.println(sensorstring_array);              //debugging
//   EC = strtok(sensorstring_array, ",");  //let's pars the array at each comma
//   TDS = strtok(NULL, ",");               //let's pars the array at each comma
//   SAL = strtok(NULL, ",");               //let's pars the array at each comma
//   GRAV = strtok(NULL, ",");              //let's pars the array at each comma

//   // Serial.print("EC:");                                //we now print each value we parsed separately
//   // Serial.println(EC);                                 //this is the EC value

//   // Serial.print("TDS:");                               //we now print each value we parsed separately
//   // Serial.println(TDS);                                //this is the TDS value

//   // Serial.print("SAL:");                               //we now print each value we parsed separately
//   // Serial.println(SAL);                                //this is the salinity value

//   // Serial.print("GRAV:");                              //we now print each value we parsed separately
//   // Serial.println(GRAV);                               //this is the specific gravity
//   // Serial.println();                                   //this just makes the output easer to read

//   ec = atof(EC);      //uncomment this line to convert the char to a float
//   sal = atof(SAL);    //uncomment this line to convert the char to a float
//   tds = atof(TDS);    //uncomment this line to convert the char to a float
//   grav = atof(GRAV);  //uncomment this line to convert the char to a float
// }




// old data file saving
// void writeDataRow() {
//   myFile = SD.open(timestamp_filename + ".csv", FILE_WRITE);

//   // IN EVERY FILE
//   myFile.print(currentTime.timestamp(DateTime::TIMESTAMP_FULL)); myFile.print(',');
//   myFile.print(deviceMode);                                     myFile.print(',');
//   myFile.print(battV,           decimalPlaces); myFile.print(',');

//   // SALINITY
//   myFile.print(ec,              decimalPlaces); myFile.print(',');
//   myFile.print(sal,             decimalPlaces); myFile.print(',');
//   myFile.print(tds,             decimalPlaces); myFile.print(',');

//   // TEMPERATURE SENSORS
//   myFile.print(dallasTemp,      decimalPlaces); myFile.print(',');
//   myFile.print(thermTemp,       decimalPlaces); myFile.print(',');
//   myFile.print(pt100Temp,       decimalPlaces); myFile.print(',');
//   myFile.print(brFastTemp,      decimalPlaces); myFile.print(',');

//   // BLUE ROBOTICS PRESSURE, whichever sensor is selected populates these fields
//   myFile.print(brPressure,      decimalPlaces); myFile.print(',');
//   myFile.print(brTemperature,   decimalPlaces); myFile.print(',');
//   myFile.print(brDepth,         decimalPlaces); myFile.print(',');

//   // LIGHT SENSOR
//   float lt450, lt500, lt550, lt570, lt600, lt650; // 6-channel light

//   myFile.println();
//   myFile.close();
// }





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

