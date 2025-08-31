#pragma once
// /***************************************************************/
// OLED display handling for SeaLabCTDv2.
// Provides menu, mode-specific readouts, countdown splash screens,
// and OLED power management (sleep/wake).
// /***************************************************************/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

Adafruit_SH1107 display = Adafruit_SH1107(64, 128, &Wire);
#define OLED_ADDR 0x3C   // FeatherWing SH1107 default I2C address

inline void displaySystemSelection(){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0,0);
  display.println(" <<< SeaLabCTDv2 >>>"); 
  display.print(" "); display.println(currentTime.timestamp(DateTime::TIMESTAMP_FULL)); display.println(); // display.println();
  display.println("Pick a setting"); display.println();
  display.println("A: Profile");
  display.println("B: Alarm One");
  display.println("C: Alarm Two");
}

inline void displayMode0(){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0,0);
  display.println("        PROFILE");  
  display.print(" ");             display.println(currentTime.timestamp(DateTime::TIMESTAMP_FULL));
  if (salinityBool) {   display.print("Sal:");          display.print(sal,2);             display.print(" | TDS:");  display.println(tds,0); }
  if (brFastTempBool) { display.print("brFastTemp:");   display.println(brFastTemp,2);  }
  if (bar02Bool || bar30Bool || bar100Bool) {           display.print("brPressure :");    display.println(brPressure,2); }   
  if (dallasTempBool) { display.print("dallasTemp:");   display.println(dallasTemp,2);  }
  if (thermTempBool) {  display.print("thermTemp :");    display.println(thermTemp,2);   }    
  if (pressDFBool) {    display.print("pressDF_ft:");   display.println(pressDF_ft,2);  }   
  if (pressDFBool) {    display.print("pressDF_ft:");   display.println(pressDF_ft,2);  } 
  if (pt100Bool) {    display.print("PT100       :");   display.println(pt100Temp,2);  } 
  display.print("battV  :");      display.println(battV,2);      
  display.display(); 
}

void displayModeNalgene(){
  // PLACEHOLDER
  // display.print("BRDept:");          display.println(slowBRDepth,2);        
  // display.print("EC    : ");          display.println(ec);  
  // display.print("BRTemp :");          display.println(slowBRTemp,2);   
  // display.print("Dallas :");          display.println(slowDallasTemp,2); // not in profile setting
}

inline void oledSleep() {       // 0xAE = Display OFF (enters power-save)
  Wire.beginTransmission(OLED_ADDR);
  Wire.write(0x00);      // control byte: Co=0, D/C#=0 (next byte is a command)
  Wire.write(0xAE);      // sleep command
  Wire.endTransmission();
}

inline void oledWake() {        // 0xAF = Display ON (exit power-save)
  Wire.beginTransmission(OLED_ADDR);
  Wire.write(0x00);      // control byte: Co=0, D/C#=0 (next byte is a command)
  Wire.write(0xAF);      // wake command
  Wire.endTransmission();
}

static inline void alarmDisplay(uint32_t wait_seconds, const char* alarm_label) {
  int count = 10;  // seconds to show before sleeping

  while (count >= 0) {
    display.clearDisplay();
    display.setCursor(0, 0);

    display.print(alarm_label);
    display.print(" is ");
    display.print(wait_seconds);
    display.println(" sec");

    display.println("Display turns off in 10 sec:");
    display.println();
    display.print(count);
    display.print(" sec");
    display.display();

    delay(1000);
    count--;
  }

  // Optional "Goodbye" splash
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Goodbye!");
  display.display();
  delay(1000);

  oledSleep();  // SH1107 power-save (microamps)
}


