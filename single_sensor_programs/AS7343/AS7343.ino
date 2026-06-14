/*
 * AS7343_live_serial.ino
 *
 * Live serial viewer for the Adafruit AS7343 14-channel spectral sensor
 * on an RP2040 (Adafruit Feather/Adalogger, Earle Philhower arduino-pico core).
 *
 * No datalogging. Two output styles selected by the toggle below.
 * Library: Adafruit_AS7343 (Library Manager -> "Adafruit AS7343")
 *
 * Wiring (header-pin breakout, NOT the Qwiic connector):
 *   3V  -> VIN     (RP2040 is a 3V board)
 *   GND -> GND
 *   SCL -> SCL
 *   SDA -> SDA
 * The breakout has on-board 10k I2C pull-ups, so no external resistors needed.
 */

// ============================================================
//  OUTPUT MODE TOGGLE  --  change this one line, then re-upload
//    0 = Serial Monitor : labeled, human-readable text
//    1 = Serial Plotter : one numeric line/sample (Tools -> Serial Plotter)
// ============================================================
#define USE_SERIAL_PLOTTER 1

// Plotter labels (label:value) need Arduino IDE 2.x.
// On IDE 1.8.x the traces still plot, just shown as var0, var1, ...

#include <Adafruit_AS7343.h>

Adafruit_AS7343 as7343;

#define REFRESH_MS 70   // loop refresh rate

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10);  // wait for native USB serial
  }

  // --- Optional: remap I2C pins (Philhower core) ---
  // Uncomment and set if you are NOT on this board's default SDA/SCL pins.
  // Wire.setSDA(2);
  // Wire.setSCL(3);

  if (!as7343.begin()) {
    Serial.println("Could not find AS7343 -- check wiring/I2C pins/address");
    while (1) {
      delay(10);
    }
  }

  // --- Sensor configuration ---
  // Drop gain if channels saturate (65535); raise it in dim light.
  as7343.setGain(AS7343_GAIN_64X);
  as7343.setATIME(29);   // integration cycles
  as7343.setASTEP(599);  // step size  -> ~50 ms integration with ATIME=29

#if !USE_SERIAL_PLOTTER
  // Keep these banners out of plotter mode so it doesn't try to plot them.
  Serial.println("AS7343 live serial viewer");
  Serial.println("AS7343 found");
  Serial.print("Integration time: ");
  Serial.print(as7343.getIntegrationTime());
  Serial.println(" ms\n");
#endif
}

void loop() {
  uint16_t readings[18];  // 14 spectral + VIS + FD across 3 cycles

  // Blocking read: starts a measurement, waits, and fills the buffer.
  if (!as7343.readAllChannels(readings)) {
#if !USE_SERIAL_PLOTTER
    Serial.println("Read failed!");
#endif
    delay(REFRESH_MS);
    return;
  }

#if USE_SERIAL_PLOTTER
  // ---- Serial Plotter: one line, label:value pairs, tab separated ----
  Serial.print("F1_405:");  Serial.print(readings[AS7343_CHANNEL_F1]);  Serial.print('\t');
  Serial.print("F2_425:");  Serial.print(readings[AS7343_CHANNEL_F2]);  Serial.print('\t');
  Serial.print("FZ_450:");  Serial.print(readings[AS7343_CHANNEL_FZ]);  Serial.print('\t');
  Serial.print("F3_475:");  Serial.print(readings[AS7343_CHANNEL_F3]);  Serial.print('\t');
  Serial.print("F4_515:");  Serial.print(readings[AS7343_CHANNEL_F4]);  Serial.print('\t');
  Serial.print("F5_550:");  Serial.print(readings[AS7343_CHANNEL_F5]);  Serial.print('\t');
  Serial.print("FY_555:");  Serial.print(readings[AS7343_CHANNEL_FY]);  Serial.print('\t');
  Serial.print("FXL_600:"); Serial.print(readings[AS7343_CHANNEL_FXL]); Serial.print('\t');
  Serial.print("F6_640:");  Serial.print(readings[AS7343_CHANNEL_F6]);  Serial.print('\t');
  Serial.print("F7_690:");  Serial.print(readings[AS7343_CHANNEL_F7]);  Serial.print('\t');
  Serial.print("F8_745:");  Serial.print(readings[AS7343_CHANNEL_F8]);  Serial.print('\t');
  Serial.print("NIR_855:"); Serial.print(readings[AS7343_CHANNEL_NIR]);
  // VIS/clear is broadband and usually swamps the plotter autoscale.
  // Uncomment to include it:
  // Serial.print('\t'); Serial.print("VIS:"); Serial.print(readings[AS7343_CHANNEL_VIS_TL_0]);
  Serial.println();

#else
  // ---- Serial Monitor: labeled, human-readable ----
  Serial.println("--- Spectral (raw counts) ---");
  Serial.print("F1  405nm violet      : "); Serial.println(readings[AS7343_CHANNEL_F1]);
  Serial.print("F2  425nm violet-blue : "); Serial.println(readings[AS7343_CHANNEL_F2]);
  Serial.print("FZ  450nm blue        : "); Serial.println(readings[AS7343_CHANNEL_FZ]);
  Serial.print("F3  475nm blue-cyan   : "); Serial.println(readings[AS7343_CHANNEL_F3]);
  Serial.print("F4  515nm green       : "); Serial.println(readings[AS7343_CHANNEL_F4]);
  Serial.print("F5  550nm green-yellow: "); Serial.println(readings[AS7343_CHANNEL_F5]);
  Serial.print("FY  555nm yellow-green: "); Serial.println(readings[AS7343_CHANNEL_FY]);
  Serial.print("FXL 600nm orange      : "); Serial.println(readings[AS7343_CHANNEL_FXL]);
  Serial.print("F6  640nm red         : "); Serial.println(readings[AS7343_CHANNEL_F6]);
  Serial.print("F7  690nm deep red    : "); Serial.println(readings[AS7343_CHANNEL_F7]);
  Serial.print("F8  745nm near-IR     : "); Serial.println(readings[AS7343_CHANNEL_F8]);
  Serial.print("NIR 855nm near-IR     : "); Serial.println(readings[AS7343_CHANNEL_NIR]);
  Serial.print("VIS clear             : "); Serial.println(readings[AS7343_CHANNEL_VIS_TL_0]);
  Serial.println();
#endif

  delay(REFRESH_MS);
}