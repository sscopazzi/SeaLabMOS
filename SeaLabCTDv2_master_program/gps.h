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
//     Serial1, 57600 baud, RMC+GGA, 10 Hz
//     GPS is the ONLY sensor. No RTC needed — GPS provides timestamp.
//     Logs lat, lon, speed, angle, fix quality, sats, altitude at ~10 Hz.
//     Baud MUST be raised to 57600 first at 9600, then re-opened at 57600.
//     GGA is kept (not dropped): it provides satellite count and altitude.
//
// ⚠️  SERIAL PORT NOTE:
//     GPS FeatherWing is hardwired to Serial1 (GPIO0/GPIO1).
//     Do not enable salinityBool (Atlas EZO, also Serial1) at the same time.
//     In mode 6, salinityBool should always be false.
//
// ⚠️  SPEED UNITS:
//     NMEA RMC reports speed-over-ground in KNOTS. gpsSpeed is converted
//     to m/s here (× KNOTS_TO_MS) so the logged value matches its column
//     name (gpsSpeed_ms). If you want knots in the log, drop the multiply
//     and rename the column to gpsSpeed_kn — do not log raw GPS.speed as
//     "_ms", that is a 1.94384x overestimate.
// /***************************************************************/

#include <Adafruit_GPS.h>  // by Adafruit

// ---------------------------------------------------------------
// Update this each year. Used to reject GPS week-rollover timestamps
// (e.g. year = 80 for 1980) that look valid but aren't.
// GPS.year is 2-digit, so GPS_PRESENT_YEAR_2DIGIT = 26 means 2026.
// Accepted year range below is 2026–2079 (2-digit 26–79).
// ---------------------------------------------------------------
#define GPS_PRESENT_YEAR      2026
#define GPS_PRESENT_YEAR_2DIGIT (GPS_PRESENT_YEAR % 100)  // = 26

// Knots → m/s. 1 knot = 0.514444 m/s (1 m/s = 1.94384 knots).
#define KNOTS_TO_MS 0.514444f

#define GPS_SERIAL Serial1

Adafruit_GPS GPS(&GPS_SERIAL);

#define GPS_ECHO false   // set true to dump raw NMEA to Serial (debug only)

// ---------------------------------------------------------------
// GPS values — defined in globals.h, declared extern here
// ---------------------------------------------------------------
extern bool     serialDisplay;

extern float    gpsLat;    // decimal degrees, positive = North
extern float    gpsLon;    // decimal degrees, positive = East
extern float    gpsAlt;    // meters above MSL (from GGA; noisy on a surface float)
extern float    gpsSpeed;  // m/s (converted from GPS knots — see SPEED UNITS note)
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
    gpsSpeed = GPS.speed * KNOTS_TO_MS;   // knots → m/s
    gpsAngle = GPS.angle;                 // degrees true

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
// year=80 (meaning 1980, not 2080); a cold module also emits year=00
// with month=00/day=00. We accept only 26–79 (2026–2079) with a sane
// month/day. Update GPS_PRESENT_YEAR at the top of this file each year.
// ---------------------------------------------------------------
inline bool gpsTimestampValid() {
  return (GPS.year >= GPS_PRESENT_YEAR_2DIGIT &&
          GPS.year <  80 &&          // reject 1980 rollover (year==80)
          GPS.month >= 1 && GPS.month <= 12 &&
          GPS.day   >= 1 && GPS.day   <= 31);
}

// ---------------------------------------------------------------
// gpsRecordValid() — call this in the logger BEFORE writing a row.
//
// This is the guard that would have kept the 540 garbage rows
// (2001-00-00 timestamps, gpsSpeed_ms = 12214, gpsLon == gpsLat)
// out of the surface-float CSV. gpsPoll() builds gpsTimestamp on
// every sentence, even bad ones, so the logger must gate writes on
// a real fix + a valid timestamp + a sane speed.
//
//   if (gpsHasNewFix() && gpsRecordValid()) { writeRow(); }
//
// MAX_SOG_MS: a drifting surface float is not exceeding ~10 m/s;
// raise it if you ever tow the package.
// ---------------------------------------------------------------
#define MAX_SOG_MS 10.0f

inline bool gpsRecordValid() {
  return (gpsFix &&
          gpsTimestampValid() &&
          gpsSpeed >= 0.0f && gpsSpeed < MAX_SOG_MS);
}