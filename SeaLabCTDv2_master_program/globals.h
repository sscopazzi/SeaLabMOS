#pragma once
// /***************************************************************/
// Global definitions and utilities for SeaLabCTD data logging.
// Manages RTC, SD card, sensor values, CSV header/filename, and
// provides functions for writing data rows and serial output
// /***************************************************************/

#include <Arduino.h>
#include <RTClib.h>   // for DateTime, RTC_DS3231
#include "SdFat.h"    // for SdFat, FsFile
#include <SPI.h>
#include "hardware/clocks.h" // used to set clock speed in firmware
#include "hardware/vreg.h"   // used to set clock speed in firmware

// IF YOU CHANGE THIS HEADER YOU MUST ALSO CHANGE HOW THE DATA IS WRITTEN TO THE SD CARD
// CSV header / filename
String header = "time,deviceMode,battV,ec,sal,tds,dallasTemp,thermTemp,pt100Temp,brFastTemp,brPressure,brTemperature,brDepth";
String bprHeader = "time,deviceMode,battV,brPressure,brTemperature,brDepth";
String timestamp_filename = "";   // YYYY-MM-DD hh-mm-ss in Mode 0 and YYYY-MM-DD in Modes 1 and 2

RTC_DS3231 rtc;
volatile bool rtcAlarmFired = false;
DateTime nextSample;
void rtcWakeISR() { rtcAlarmFired = true; }

inline TimeSpan waitOne() { return TimeSpan(0, 0, WAIT_TIME_ONE, 0); }
inline TimeSpan waitTwo() { return TimeSpan(0, 0, WAIT_TIME_TWO, 0); }

// RTC / SD / filenames owned by the .ino
extern RTC_DS3231 rtc;
extern SdFat      SD;
extern FsFile     myFile;
extern String     header;
extern String     timestamp_filename;
extern bool serialDisplay;

// SD
#define SD_CS_PIN 23
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
float thermTemp   = 999.9; // degC
float pt100Temp   = 999.0; //decC

float ec   = 999.0;
float sal  = 999.0;
float tds  = 999.0;
float grav = 999.0;


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
      if (deviceMode == 4) {
        myFile.println(bprHeader);
      } else {
        myFile.println(header);
      }
    }
    myFile.close();
  } else {
    if (serialDisplay) Serial.println("SD open failed in updateFilenameAndHeader()");
  }
}

void writeDataRow() {
  myFile = SD.open(timestamp_filename + ".csv", FILE_WRITE);

  myFile.print(currentTime.timestamp(DateTime::TIMESTAMP_FULL)); myFile.print(',');
  myFile.print(deviceMode);                                     myFile.print(',');

  myFile.print(battV,           decimalPlaces); myFile.print(',');
  myFile.print(ec,              decimalPlaces); myFile.print(',');
  myFile.print(sal,             decimalPlaces); myFile.print(',');
  myFile.print(tds,             decimalPlaces); myFile.print(',');
  myFile.print(dallasTemp,      decimalPlaces); myFile.print(',');
  myFile.print(thermTemp,       decimalPlaces); myFile.print(',');
  myFile.print(pt100Temp,       decimalPlaces); myFile.print(',');
  myFile.print(brFastTemp,      decimalPlaces); myFile.print(',');
  myFile.print(brPressure,      decimalPlaces); myFile.print(',');
  myFile.print(brTemperature,   decimalPlaces); myFile.print(',');
  myFile.print(brDepth,         decimalPlaces); myFile.println();

  myFile.close();
}

// Serial dump of key values
void serialPrintValues() {
  Serial.print("Time: ");
  Serial.print(currentTime.timestamp(DateTime::TIMESTAMP_FULL));
  Serial.print(" battV: "); Serial.print(battV);

  if (brFastTempBool) { Serial.print(" brFastTemp: "); Serial.print(brFastTemp); }
  if (bar02Bool || bar30Bool || bar100Bool) { 
    Serial.print(" brPress: "); Serial.print(brPressure);
    Serial.print(" brDepth: "); Serial.print(brDepth);
  }
  if (salinityBool)   { Serial.print(" sal: ");         Serial.print(sal);         }
  if (thermTempBool)  { Serial.print(" thermTemp: ");   Serial.print(thermTemp);   }
  if (dallasTempBool) { Serial.print(" dallasTemp: ");  Serial.print(dallasTemp);  }
  if (pressDFBool)    { Serial.print(" pressDF_ft: ");  Serial.print(pressDF_ft);  }
  if (pt100Bool)      { Serial.print(" PT100: ");       Serial.print(pt100Temp);   }

  Serial.println();
}

static inline DateTime alignToNext(DateTime currentTime, uint16_t sec) {
  uint32_t t = currentTime.unixtime();
  return DateTime((t - (t % sec)) + sec);
}


bool bprSampling = false;  // true = sampling active (within :00–:20 window)

// Return hh:00:00 at this hour (if strictly in the future) or next hour.
static inline DateTime topOfNextHour(const DateTime& now) {
  // Next hour boundary with seconds = 0
  uint8_t h = now.hour();
  uint8_t d = now.day();
  uint8_t mo = now.month();
  int16_t y = now.year();

  if (now.minute() > 0 || now.second() > 0) {
    // advance an hour
    DateTime adv(now.unixtime() + (3600 - (now.minute()*60 + now.second())));
    return DateTime(adv.year(), adv.month(), adv.day(), adv.hour(), 0, 0);
  }
  // already at hh:00:00? push to next hour so it's strictly > now
  return DateTime(y, mo, d, (uint8_t)((h + 1) % 24), 0, 0);
}

// Return the next BPR boundary strictly > now:
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


