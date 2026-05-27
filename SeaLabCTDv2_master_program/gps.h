#pragma once
// /***************************************************************/
// Adafruit GPS FeatherWing interface for SeaLabCTDv2.
//
// TWO OPERATING MODES controlled by deviceMode in the .ino:
//
//   Standard (modes 0-5):
//     Serial1, 9600 baud, RMC+GGA, 1 Hz
//     Used alongside other sensors. GPS timestamp supplements RTC.
//
//   Mode 6 — Surface Float 10 Hz:
//     Serial1, 57600 baud, RMC only, 10 Hz
//     GPS is the ONLY sensor. No RTC needed — GPS provides timestamp.
//     Logs lat, lon, speed, angle, fix quality, sats at ~10 Hz to SD.
//     Baud MUST be raised to 57600 first at 9600, then re-opened at 57600.
//     RMC only — altitude dropped (not in RMC sentence).
//
// ⚠️  SERIAL PORT NOTE:
//     GPS FeatherWing is hardwired to Serial1 (GPIO0/GPIO1).
//     Do not enable salinityBool (Atlas EZO, also Serial1) at the same time.
//     In mode 6, salinityBool should always be false.
// /***************************************************************/

#include <Adafruit_GPS.h>  // by Adafruit

// ---------------------------------------------------------------
// Update this each year. Used to reject GPS week-rollover timestamps
// (e.g. year = 80 for 1980) that look valid but aren't.
// GPS.year is 2-digit, so GPS_PRESENT_YEAR_2DIGIT = 26 means 2026.
// ---------------------------------------------------------------
#define GPS_PRESENT_YEAR      2026
#define GPS_PRESENT_YEAR_2DIGIT (GPS_PRESENT_YEAR % 100)  // = 26

#define GPS_SERIAL Serial1

Adafruit_GPS GPS(&GPS_SERIAL);

#define GPS_ECHO false   // set true to dump raw NMEA to Serial (debug only)

// ---------------------------------------------------------------
// GPS values — defined in globals.h, declared extern here
// ---------------------------------------------------------------
extern bool     serialDisplay;

extern float    gpsLat;    // decimal degrees, positive = North
extern float    gpsLon;    // decimal degrees, positive = East
extern float    gpsAlt;    // meters above MSL (GGA only, 0 in mode 6)
extern float    gpsSpeed;  // knots
extern float    gpsAngle;  // degrees true
extern bool     gpsFix;
extern uint8_t  gpsSats;

// GPS timestamp from RMC sentence (no RTC needed in mode 6)
// Format: "YYYY-MM-DDTHH:MM:SS" — filled by gpsPoll(), always UTC
char gpsTimestamp[20] = "0000-00-00T00:00:00";

// Internal
static bool _gpsNewFix = false;

// ---------------------------------------------------------------
// Helper: convert NMEA ddmm.mmmm → decimal degrees
// ---------------------------------------------------------------
static inline float nmeaToDeg(float nmea, char hemi) {
  int   deg = (int)(nmea / 100);
  float dec = deg + (nmea - deg * 100) / 60.0f;
  return (hemi == 'S' || hemi == 'W') ? -dec : dec;
}

// ---------------------------------------------------------------
// gpsSetup() — standard 1 Hz setup for modes 0-5
// ---------------------------------------------------------------
inline void gpsSetup() {
  GPS.begin(9600);
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);  // RMC + GGA (has altitude)
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
  GPS.sendCommand(PGCMD_ANTENNA);
  delay(1000);

  if (serialDisplay) {
    Serial.println("GPS init: Serial1, 9600 baud, 1 Hz, RMC+GGA");
  }
}

// ---------------------------------------------------------------
// gpsSetup10Hz() — 10 Hz setup for mode 6 (surface float)
//
// MTK3339 requires the baud-rate change command to be sent at
// the current baud (9600), then the serial port reopened at the
// new rate before any further commands are sent.
// RMC+GGA at 57600 baud handles 10 Hz fine and gives us satellite count.
// ---------------------------------------------------------------
inline void gpsSetup10Hz() {
  // Step 1: open at default 9600 and ask module to switch to 57600
  GPS.begin(9600);
  delay(100);
  GPS.sendCommand(PMTK_SET_BAUD_57600);   // "$PMTK251,57600*2C"
  delay(100);

  // Step 2: reopen serial at 57600 to match the module
  GPS_SERIAL.end();
  delay(10);
  GPS_SERIAL.begin(57600);
  delay(100);

  // Step 3: RMC+GGA — gives lat, lon, speed, heading, AND satellite count
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  delay(100);

  // Step 4: 10 Hz
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_10HZ);
  delay(100);

  if (serialDisplay) {
    Serial.println("GPS init: Serial1, 57600 baud, 10 Hz, RMC+GGA");
  }
}

// ---------------------------------------------------------------
// gpsPoll() — non-blocking, call every loop iteration.
// Works for both 1 Hz and 10 Hz modes.
// Updates globals and sets _gpsNewFix = true on each new fix.
// ---------------------------------------------------------------
inline void gpsPoll() {
  char c = GPS.read();
  if (GPS_ECHO && c) Serial.print(c);

  if (!GPS.newNMEAreceived()) return;
  if (!GPS.parse(GPS.lastNMEA())) return;

  gpsFix  = GPS.fix;
  gpsSats = GPS.satellites;

  // Build timestamp from GPS date/time fields (always UTC)
  // GPS.year is 2-digit; GPS.month/day/hour/minute/seconds are uint8_t
  snprintf(gpsTimestamp, sizeof(gpsTimestamp),
           "20%02d-%02d-%02dT%02d:%02d:%02d",
           GPS.year, GPS.month, GPS.day,
           GPS.hour, GPS.minute, GPS.seconds);

  if (GPS.fix) {
    gpsLat   = nmeaToDeg(GPS.latitude,  GPS.lat);
    gpsLon   = nmeaToDeg(GPS.longitude, GPS.lon);
    gpsAlt   = GPS.altitude;
    gpsSpeed = GPS.speed;      // knots
    gpsAngle = GPS.angle;      // degrees true

    _gpsNewFix = true;
  }
}

// ---------------------------------------------------------------
// gpsHasNewFix() — returns true once per new parsed fix.
// ---------------------------------------------------------------
inline bool gpsHasNewFix() {
  if (_gpsNewFix) { _gpsNewFix = false; return true; }
  return false;
}

// ---------------------------------------------------------------
// gpsTimestampValid() — returns true only when the GPS has a
// genuine current-era timestamp from satellites.
//
// GPS.year is a 2-digit value. The MTK3339 week-rollover bug returns
// year=80 (meaning 1980, not 2080). We explicitly reject anything
// outside the range 26–99 (i.e. 2026–2099).
// Update GPS_PRESENT_YEAR at the top of this file each year if needed.
// ---------------------------------------------------------------
inline bool gpsTimestampValid() {
  return (GPS.year >= GPS_PRESENT_YEAR_2DIGIT &&
          GPS.year <  80 &&          // reject 1980 rollover (year==80)
          GPS.month >= 1 && GPS.month <= 12 &&
          GPS.day   >= 1 && GPS.day   <= 31);
}