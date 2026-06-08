#pragma once
// /***************************************************************/
// Global definitions and utilities for SeaLabCTD data logging.
// Manages RTC, SD card, sensor values, CSV header/filename, and
// provides functions for writing data rows and serial output
// /***************************************************************/

#include <Arduino.h>
#include <RTClib.h>   // for DateTime, RTC_DS3231, by Adafruit
#include "SdFat.h"    // for SdFat, FsFile
#include <SPI.h>

#include "pico/stdlib.h"   // pico SDK helpers
#include "hardware/sync.h" // provides __wfi()

#include "hardware/clocks.h" // used to set clock speed in firmware
#include "hardware/vreg.h"   // used to set clock speed in firmware

// #define SERIAL_BUFFER_SIZE 1024   
// unsigned long lastOledMs = 0;       // Tracks last OLED update time
// const int oledInterval = 1000;       // Interval between display updates

String header = "time,deviceMode,battV,ec,sal,tds,dallasTemp,thermTemp,pt100Temp,brFastTemp,lt450,lt500,lt550,lt570,lt600,lt650,brPressure,brTemperature,brDepth,gpsLat,gpsLon,gpsAlt,gpsSpeed,gpsFix,gpsSats";
String timestamp_filename = "";   // YYYY-MM-DD hh-mm-ss in Mode 0 and YYYY-MM-DD in Modes 1 and 2

RTC_DS3231 rtc;
volatile bool rtcAlarmFired = false;
DateTime nextSample;
void rtcWakeISR() { rtcAlarmFired = true; }

// RTC / SD / filenames owned by the .ino
extern RTC_DS3231 rtc;
extern SdFat      SD;
extern FsFile     myFile;
extern String     header;
// extern String     timestamp_filename;
extern bool serialDisplay;

// SD
#define SD_CS_PIN 23 // this doesn't change so it's hidden here
SdFat SD;
FsFile myFile;
SdSpiConfig config(SD_CS_PIN, DEDICATED_SPI, SD_SCK_MHZ(16), &SPI1);

// ----------------------------
// Globals for sensor readings
// ----------------------------
float battV  = 0.0;

float brPressure    = 999.0;   // mbar
float brTemperature = 999.0;   // °C
float brDepth       = 999.0;   // m

float pressDF_ft  = 999.0;
float pressDF_m   = 999.0;
float pressDF_psi = 999.0;

float brFastTemp  = 999.0; // degC
float dallasTemp  = 999.0; // degC
byte dallasAddr[8];
float thermTemp   = 999.9; // degC
float pt100Temp   = 999.0; //decC

float ec   = 999.0;
float sal  = 999.0;
float tds  = 999.0;
float grav = 999.0;

float lt450, lt500, lt550, lt570, lt600, lt650; // 6-channel light

// GPS (gps.h) — defined here so globals.h is the single source of truth
float   gpsLat   = 0.0;   // decimal degrees, positive = North
float   gpsLon   = 0.0;   // decimal degrees, positive = East
float   gpsAlt   = 0.0;   // meters above MSL
float   gpsSpeed = 0.0;   // knots
float   gpsAngle = 0.0;   // degrees true
bool    gpsFix   = false;
uint8_t gpsSats  = 0;

// UTILITY
int count = 0;
int decimalPlaces = 2;
static uint32_t lastSaltMs = 0;

DateTime currentTime;
DateTime time1;
DateTime time2;

bool deviceModeZeroInit   = false;
bool deviceModeOneInit    = false;
bool deviceModeTwoInit    = false;
bool deviceModeFourInit   = false;

#define BUTTON_A  9
#define BUTTON_B  6
#define BUTTON_C  5

// === Time helpers ===
#define SECONDS(x)   ((x) * 1000UL)
#define MINUTES(x)   ((x) * 60UL * 1000UL)
#define HOURS(x)     ((x) * 3600UL * 1000UL)

// Update daily CSV filename and ensure header is present.
// NOTE: Use this for modes 1 & 2 only (don't call it in mode 0).
static inline void updateFilenameAndHeader() {
  char dateOnly[11];
  snprintf(dateOnly, sizeof(dateOnly), "%04d-%02d-%02d",
           currentTime.year(), currentTime.month(), currentTime.day());

  String dayname = String(dateOnly);

  // Track current day-based filename
  if (timestamp_filename != dayname) {
    timestamp_filename = dayname;
  }

  // Create/append the file; write header if newly created (size==0)
  myFile = SD.open(timestamp_filename + ".csv", FILE_WRITE);
  if (myFile) {
    if (myFile.size() == 0) {
        myFile.println(header);
    }
    myFile.close();
  } else {
    if (serialDisplay) Serial.println("SD open failed in updateFilenameAndHeader()");
  }
}

void writeDataRow() {
  myFile = SD.open(timestamp_filename + ".csv", FILE_WRITE);

  // IN EVERY FILE
  myFile.print(currentTime.timestamp(DateTime::TIMESTAMP_FULL)); myFile.print(',');
  myFile.print(deviceMode);                                     myFile.print(',');
  myFile.print(battV,           decimalPlaces); myFile.print(',');

  // ###### SALINITY ######
  if (salinityBool) {
    myFile.print(ec, decimalPlaces); myFile.print(',');
    myFile.print(sal, decimalPlaces); myFile.print(',');
    myFile.print(tds, decimalPlaces); myFile.print(',');
  } else {
    myFile.print(",,"); // preserve header alignment
  }

  // ###### TEMPERATURE SENSORS ######
  if (dallasTempBool)   myFile.print(dallasTemp, decimalPlaces);
  myFile.print(',');
  
  if (thermTempBool)    myFile.print(thermTemp, decimalPlaces);
  myFile.print(',');
  
  if (pt100Bool)        myFile.print(pt100Temp, decimalPlaces);
  myFile.print(',');
  
  if (brFastTempBool)   myFile.print(brFastTemp, decimalPlaces);
  myFile.print(',');

  // ###### LIGHT SENSOR ######
  if (lightBool) {
    myFile.print(lt450, decimalPlaces); myFile.print(',');
    myFile.print(lt500, decimalPlaces); myFile.print(',');
    myFile.print(lt550, decimalPlaces); myFile.print(',');
    myFile.print(lt570, decimalPlaces); myFile.print(',');
    myFile.print(lt600, decimalPlaces); myFile.print(',');
    myFile.print(lt650, decimalPlaces); myFile.print(',');
  } else {
    myFile.print(",,,,,,"); // 6 placeholders, preserve header alignment
  }

    // ###### BLUE ROBOTICS PRESSURE ######
  if (bar02Bool || bar30Bool || bar100Bool) {
    myFile.print(brPressure,    decimalPlaces);    myFile.print(',');
    myFile.print(brTemperature, decimalPlaces);    myFile.print(',');
    myFile.print(brDepth,       decimalPlaces);
  } else {
    myFile.print(",,,"); // preserve alignment
  }

  // ###### GPS ######
  myFile.print(',');
  if (gpsBool) {
    myFile.print(gpsLat,   6);   myFile.print(',');
    myFile.print(gpsLon,   6);   myFile.print(',');
    myFile.print(gpsAlt,   decimalPlaces); myFile.print(',');
    myFile.print(gpsSpeed, decimalPlaces); myFile.print(',');
    myFile.print(gpsFix  ? 1 : 0);        myFile.print(',');
    myFile.print(gpsSats);
  } else {
    myFile.print(",,,,,"); // 6 placeholders, preserve header alignment
  }

  myFile.println();
  myFile.close();
}

// Mode 6 — GPS-only surface float, 10 Hz.
// Uses GPS timestamp directly (no RTC). Minimal columns for fast SD writes.
// Header: gpsTime,gpsFix,gpsSats,gpsLat,gpsLon,gpsSpeed_kn,gpsAngle_deg,gpsAlt_m
extern char gpsTimestamp[];  // defined in gps.h
void writeDataRowMode6() {
  myFile = SD.open(timestamp_filename + ".csv", FILE_WRITE);
  myFile.print(gpsTimestamp);            myFile.print(',');
  myFile.print(gpsFix   ? 1 : 0);       myFile.print(',');
  myFile.print(gpsSats);                 myFile.print(',');
  myFile.print(gpsLat,  6);             myFile.print(',');  // 6 dp = ~110mm on Earth
  myFile.print(gpsLon,  6);             myFile.print(',');
  myFile.print(gpsSpeed, 2);            myFile.print(',');
  myFile.print(gpsAngle, 2);            myFile.print(',');
  myFile.print(gpsAlt,   1);
  myFile.println();
  myFile.close();
}

void serialPrintValues() {
  Serial.print("Time: ");
  Serial.print(currentTime.timestamp(DateTime::TIMESTAMP_FULL));
  Serial.print(" battV: "); Serial.print(battV);

  if (brFastTempBool) { Serial.print(" brFastTemp: "); Serial.print(brFastTemp); }

  if (bar02Bool || bar30Bool || bar100Bool) { 
    // Serial.print(" brPress: "); Serial.print(brPressure);
    Serial.print(" brDepth: "); Serial.print(brDepth);
  }

  if (salinityBool)   { Serial.print(" sal: ");         Serial.print(sal);         }
  if (thermTempBool)  { Serial.print(" thermTemp: ");   Serial.print(thermTemp);   }
  if (dallasTempBool) { Serial.print(" dallasTemp: ");  Serial.print(dallasTemp);  }
  if (pressDFBool)    { Serial.print(" pressDF_ft: ");  Serial.print(pressDF_ft);  }
  if (pt100Bool)      { Serial.print(" PT100: ");       Serial.print(pt100Temp);   }

  if (lightBool) {
    Serial.print(" 450nm: "); Serial.print(lt450);
    Serial.print(" 500nm: "); Serial.print(lt500);
    Serial.print(" 550nm: "); Serial.print(lt550);
    Serial.print(" 570nm: "); Serial.print(lt570);
    Serial.print(" 600nm: "); Serial.print(lt600);
    Serial.print(" 650nm: "); Serial.print(lt650);
  }

  if (gpsBool) {
    Serial.print(" GPS fix: "); Serial.print(gpsFix ? "YES" : "NO");
    if (gpsFix) {
      Serial.print(" Lat: ");   Serial.print(gpsLat, 6);
      Serial.print(" Lon: ");   Serial.print(gpsLon, 6);
      Serial.print(" Alt: ");   Serial.print(gpsAlt);
      Serial.print("m Sats: "); Serial.print(gpsSats);
    }
  }

  Serial.println();
}

bool bprSampling = false;  // true = sampling active (within :00–:20 window)

// Return the next BPR boundary
//   if now < hh:20:00  -> hh:20:00
//   else               -> (hh+1):00:00
static inline DateTime nextBprBoundary(const DateTime& now) {
    // Time at hh:20:00 today
    DateTime twenty(now.year(), now.month(), now.day(), now.hour(), WAIT_TIME_BPR, 0);
    
    if (now < twenty) {
        // less than :20 → return hh:20:00
        return twenty;
    }
    // else return top of next hour
    return DateTime(now.year(), now.month(), now.day(), now.hour(), 0, 0) + TimeSpan(0, 1, 0, 0);
}

DateTime getNextAlarm(DateTime now, uint8_t waitTimeMinutes) {
    uint8_t currentMinute = now.minute();
    uint8_t currentHour = now.hour();
    uint8_t currentDay = now.day();
    uint8_t currentMonth = now.month();
    uint16_t currentYear = now.year();

    // Compute next multiple of waitTimeMinutes after currentMinute
    uint8_t nextMinute = ((currentMinute / waitTimeMinutes) + 1) * waitTimeMinutes;
    uint8_t nextHour = currentHour;
    uint8_t nextDay = currentDay;
    uint8_t nextMonth = currentMonth;
    uint16_t nextYear = currentYear;

    if (nextMinute >= 60) {
        nextMinute = nextMinute % 60;
        nextHour += 1;
        if (nextHour >= 24) {
            nextHour = 0;
            nextDay += 1;
            // Very basic day/month rollover (not accounting for months < 31)
            // Use RTC library DateTime add functions for full correctness
        }
    }

    return DateTime(nextYear, nextMonth, nextDay, nextHour, nextMinute, 0);
}

inline void setRtcCompileTimeUTC() {
  if (serialDisplay) Serial.println("RTC lost power, let's set the time!");
  DateTime compiled = DateTime(F(__DATE__), F(__TIME__));
  // adjust for timezone offset to get UTC
  rtc.adjust(DateTime(compiled.unixtime() - (timeZone * 3600)));
  if (serialDisplay) Serial.println("RTC set in UTC!");
}