#pragma once
// /***************************************************************/
// Thermistor temperature measurement for SeaLabCTDv2.
// - Uses 10kΩ NTC thermistor with Beta coefficient model
// - Series resistor value selected per SYSTEM_NAME in main .ino file
// - Averages multiple ADC samples (NUM_SAMPLES), converts to resistance,
//   applies Steinhart–Hart equation, returns °C
// - Uses 12-bit analogReadResolution
// /***************************************************************/

#include <math.h>

#define THERMISTORNOMINAL  10000.0   // Ω at 25°C, the nominal reported by datasheet
#define TEMPERATURENOMINAL 25.0      // °C
#define BCOEFFICIENT       3950.0    // from datasheet
#define NUMSAMPLES         20        // found to work well with not taking too long
#define ADC_MAX            4095.0    // 12-bit resolution

// 10 kΩ resistor used in series for sampling is not perfect
// Measure with a multimeter and enter here so the temp value is more accurate (by a small amount)
#if SYSTEM_NAME == GREEN
  #define SERIESRESISTOR 10220.0
  #warning "SYSTEM_NAME: GREEN"
#elif SYSTEM_NAME == BLUE
  #define SERIESRESISTOR 9990.0
  #warning "SYSTEM_NAME: BLUE"
#elif SYSTEM_NAME == ORANGE
  #define SERIESRESISTOR 9890.0
  #warning "SYSTEM_NAME: ORANGE"
#elif SYSTEM_NAME == WHITE
  #define SERIESRESISTOR 9970.0
  #warning "SYSTEM_NAME: WHITE"
#elif SYSTEM_NAME == NALGENE
  #define SERIESRESISTOR 9970.0 
  #warning "SYSTEM_NAME: NALGENE"
#elif SYSTEM_NAME == STEEL
  #define SERIESRESISTOR 10000.0
  #warning "SYSTEM_NAME: STEEL"
#else
  #warning "UNKNOWN SYSTEM_NAME — USING DEFAULT VALUE FOR SERIESRESISTOR"
  #warning "IF NO THERMISTOR, IGNORE THIS MESSAGE" (THIS MESSAGE IS FROM THERM.H)
  #define SERIESRESISTOR 1000.0
#endif

int samples[NUMSAMPLES];

inline float getThermTemp() {
  float sum = 0.0f;
  for (uint8_t i = 0; i < NUMSAMPLES; i++) {
    sum += analogRead(THERMISTOR_PIN);
    delay(5);
  }
  const float avg = sum / NUMSAMPLES;

  if (avg <= 0.0f || avg >= ADC_MAX) return NAN;

  // Compute Rtherm both ways for one-shot debugging
  const float Rt_notflipped = SERIESRESISTOR *        avg  / (ADC_MAX - avg); // VCC->Rseries->Ain->Rtherm->GND
  const float Rt_flipped    = SERIESRESISTOR * (ADC_MAX - avg) /        avg;  // VCC->Rtherm->Ain->Rseries->GND

  // if (serialDisplay) {
  //   Serial.print("avgADC="); Serial.print(avg);
  //   Serial.print("  SERIES="); Serial.print(SERIESRESISTOR);
  //   Serial.print("  Rt_notflipped="); Serial.print(Rt_notflipped);
  //   Serial.print("  Rt_flipped="); Serial.println(Rt_flipped);
  // }

  // Use the standard Adafruit wiring first (not flipped)
  float Rt = Rt_notflipped;

  // Steinhart–Hart (Beta form)
  float invT = (1.0f / (TEMPERATURENOMINAL + 273.15f)) + (log(Rt / THERMISTORNOMINAL) / BCOEFFICIENT);
  float T = (1.0f / invT) - 273.15f;

  return T;
}
