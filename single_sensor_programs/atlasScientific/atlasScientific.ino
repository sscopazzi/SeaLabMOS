// ============================================================
//  SalinityTest.ino
//  Standalone test sketch for Atlas Scientific EZO-EC (UART)
//  on the SeaLabCTDv2 hardware (RP2040 / Raspberry Pi Pico).
//
//  Matches master-program pinouts:
//    Serial1         — EZO-EC UART (same as master)
//    SALINITY_ENABLE_PIN (GPIO 4) — power-enable for salinity board
//    BATTV_PIN (A2)  — battery voltage monitor (optional, shown for reference)
//    SYSTEM_BAUD     — 115200 to USB serial monitor
//    SALINITY_BAUD   — 9600  to EZO-EC
//
//  Two test modes (choose with TEST_MODE below):
//    PC_PASSTHROUGH  — type Atlas commands in Serial Monitor, see raw replies
//    AUTO_SAMPLE     — power-cycles the board, reads EC/TDS/Sal/Grav every ~2 s
// ============================================================

#include <Arduino.h>
#include "hardware/clocks.h"

// ---- Pin / baud definitions (mirror master program) ----
#define SALINITY_ENABLE_PIN  4       // GPIO 4 — enable line for Atlas board
#define BATTV_PIN            A2      // analog battery monitor (read-only here)
#define SYSTEM_BAUD          115200  // USB Serial
#define SALINITY_BAUD        9600    // EZO-EC default

// ---- Choose test mode ----
#define PC_PASSTHROUGH  1   // relay Serial <-> Serial1 (full Atlas command shell)
#define AUTO_SAMPLE     0   // autonomous power-on / read / power-off loop

#define TEST_MODE  PC_PASSTHROUGH   // <-- change to PC_PASSTHROUGH to send commands

// ---- Optional: temperature to send for compensation (AUTO_SAMPLE mode) ----
//   Set to a constant or wire up a temp sensor and substitute its reading.
#define TEMP_C_FOR_COMP  20.0f   // °C

// ============================================================
//  Parsed sensor values (mirrors globals.h)
// ============================================================
float ec   = 999.0;
float sal  = 999.0;
float tds  = 999.0;
float grav = 999.0;

String sensorstring = "";
String inputstring  = "";

// ============================================================
//  Parse a comma-delimited EC reply: "ec,tds,sal,grav"
//  (same logic as salinAtlas.h :: print_EC_data)
// ============================================================
void parse_EC_data() {
  char buf[30];
  sensorstring.toCharArray(buf, sizeof(buf));

  char *EC   = strtok(buf,  ",");
  char *TDS  = strtok(NULL, ",");
  char *SAL  = strtok(NULL, ",");
  char *GRAV = strtok(NULL, ",");

  ec   = (EC   != NULL) ? atof(EC)   : 999.0f;
  tds  = (TDS  != NULL) ? atof(TDS)  : 999.0f;
  sal  = (SAL  != NULL) ? atof(SAL)  : 999.0f;
  grav = (GRAV != NULL) ? atof(GRAV) : 999.0f;
}

// ============================================================
//  Read one line from Serial1 (non-blocking, CR-terminated).
//  Returns true when a complete line is ready in sensorstring.
// ============================================================
bool readSensorLine() {
  sensorstring.remove(0);
  if (Serial1.available()) {
    sensorstring = Serial1.readStringUntil(13);  // 13 = '\r'
    return true;
  }
  return false;
}

// ============================================================
//  SETUP
// ============================================================
void setup() {
  // Keep clock at safe default for sensor timing
  set_sys_clock_khz(133000, true);
  delay(5);

  Serial.begin(SYSTEM_BAUD);
  delay(3000);   // give the Serial Monitor time to connect
  Serial.println("=== SeaLabCTD Salinity Test ===");

#if TEST_MODE == PC_PASSTHROUGH
  Serial.println("Mode: PC PASSTHROUGH");
  Serial.println("Type Atlas commands (e.g. i, R, CAL,dry ...) and press Enter.");
  Serial.println("-----------------------------------------------");
#else
  Serial.println("Mode: AUTO SAMPLE  (EC/TDS/Sal/Grav every ~2 s)");
  Serial.println("-----------------------------------------------");
#endif

  // EZO-EC UART
  Serial1.begin(SALINITY_BAUD);
  sensorstring.reserve(30);
  inputstring.reserve(10);

  // Enable pin — start LOW (off)
  pinMode(SALINITY_ENABLE_PIN, OUTPUT);
  digitalWrite(SALINITY_ENABLE_PIN, LOW);

  analogReadResolution(12);  // match master for battV accuracy

  // Power on the board and let it boot (~900 ms recommended in master)
  // Runs for BOTH modes — passthrough needs the board on to accept commands
  Serial.println("Powering on Atlas EZO-EC board...");
  digitalWrite(SALINITY_ENABLE_PIN, HIGH);
  delay(900);

  // Turn on the LED indicator so we can see the board is alive
  Serial1.print("L,1\r");
  delay(100);

#if TEST_MODE == AUTO_SAMPLE
  // Enable continuous sampling mode (board will stream readings autonomously)
  Serial1.print("C,2\r");
  delay(100);

  // Optional: send temperature compensation once at startup
  // Serial1.print("T," + String(TEMP_C_FOR_COMP, 1) + "\r");
  // delay(100);

  Serial.println("EZO-EC in continuous mode. Reading...");
  Serial.println("time_ms,ec_uS,tds_mg,sal_ppt,grav");
#endif
}

// ============================================================
//  LOOP — PC_PASSTHROUGH
// ============================================================
#if TEST_MODE == PC_PASSTHROUGH
void loop() {
  // Forward anything typed in Serial Monitor to the EZO-EC
  if (Serial.available()) {
    inputstring = Serial.readStringUntil(13);   // read until Enter
    inputstring.trim();
    Serial1.print(inputstring);
    Serial1.print('\r');
    inputstring = "";
  }

  // Forward everything the EZO-EC sends back to Serial Monitor
  if (Serial1.available()) {
    sensorstring = Serial1.readStringUntil(13);
    Serial.print("RAW: "); Serial.println(sensorstring);

    // If it looks like sensor data, also parse and print the values
    if (isdigit(sensorstring[0])) {
      parse_EC_data();
      Serial.print(" PARSED: ec=");   Serial.print(ec,  3);
      Serial.print(" tds=");      Serial.print(tds, 3);
      Serial.print(" sal=");      Serial.print(sal, 4);
      Serial.print(" grav=");     Serial.println(grav, 4);
      Serial.println();
    }
    sensorstring = "";
  }
}

// ============================================================
//  LOOP — AUTO_SAMPLE
// ============================================================
#else
void loop() {
  if (readSensorLine()) {
    if (isdigit(sensorstring[0])) {
      parse_EC_data();

      // Print CSV row: timestamp_ms, ec, tds, sal, grav
      Serial.print(millis());   Serial.print(',');
      Serial.print(ec,  3);     Serial.print(',');
      Serial.print(tds, 3);     Serial.print(',');
      Serial.print(sal, 4);     Serial.print(',');
      Serial.println(grav, 4);
    } else {
      // Non-data reply (status, acknowledgement, error)
      Serial.print("[EZO] ");
      Serial.println(sensorstring);
    }
    sensorstring = "";
  }
}
#endif