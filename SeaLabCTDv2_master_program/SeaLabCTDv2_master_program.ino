// RP2040 NEEDS THIS TO BE IN ARDUINO IDE FOR BOARDS
// https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json

// Coming features...
// 1. GNSS sensor
// 2. Light sensor
// 3. Fix/troubleshoot salinity sensor sometimes buffer overrun issues. Sensor puts more in the the buffer than we can read so it samples every 2 seconds to fix
// 4. Salinity temp comp RT command sometimes works, related to #3
// 5. Auto-select sensors based on system type (CTD, BPR, temp logger, etc)??

// THESE ARE MY SYSTEM NAMES, YOUR NAMES WILL BE DIFFERENCE

// TEMP LOGGER SWITCH COLORS
#define GREEN  0
#define BLUE   1
#define ORANGE 2
#define WHITE  3

// OTHER SYSTEM VBAT NUMBERS
// CURRENTLY ONLY USED FOR VBAT CORRECTION FACTOR AND THERMISTOR RESISTOR VALUES
#define NALGENE 4       // surface float
#define STEEL   5       // extra-deep
#define TWOIN   6       // two-inch PVC
#define BPR     7       // pressure only sensor, larger battery, records first 20min of every hr by default (can use 3 in a triangle for calculating wave height/direction??)
#define PRESS_ONLY 8    // 1.5" pvc continuous pressure 
//#############################
#define SYSTEM_NAME PRESS_ONLY
//#############################

int deviceMode = 5;
// 0 = fast as possible
// 1 uses WAIT_TIME_ONE
// 2 uses WAIT_TIME_TWO
// 3 is charge mode 
// 4 is BPR (samples at 4Hz first 20min of every hr)
// 5 is continuous pressure recording at ~5Hz

bool serialDisplay  = true;  // Set to false to disable all Serial prints
bool displayBool    = false; // Adafruit Feather OLED Display
int timeZone        = -7;    // time zone of commputer time, as program pulls time from computer to set RTC, but must convert

// ###### SENSORS USED BY SYSTEM ######
bool salinityBool = false;  // Atlas Scientific Salinity Sensor
bool ecBool       = false;  // is value enabled on EZO circuit to send?
bool sBool        = false;  // is value enabled on EZO circuit to send?
bool tdsBool      = false;  // is value enabled on EZO circuit to send?
bool sgBool       = false;  // is value enabled on EZO circuit to send?

// ###### TEMPERATURE ######
bool dallasTempBool = false;  // Dallas Temperature sensor
bool thermTempBool  = false;  // Adafruit Thermistor
bool pt100Bool      = false;  // Adafruit PT100
bool brFastTempBool = false;  // Blue Robotics Fast Temperature

// ###### PRESSURE ######
bool pressDFBool  = false;  // DF Robot analog pressure sensor
bool bar02Bool    = false;    // Blue Robotics Bar02
bool bar30Bool    = true;    // Blue Robotics Bar30
bool bar100Bool   = false;   // Blue Robotics Bar100

// ###### UTILITY ######
bool beaconBool = false;  // Surface float LED beacon

// ###### OTHER SENSORS ######
bool lightBool = false;  // Adafruit AS7262 6-channel Visible Light Sensor

#define WAIT_TIME_ONE 1   // 1 min (other: 5 min, 20 min, 30 min, etc.)
#define WAIT_TIME_TWO 10  // 10 min (other: 1 hr, 12 hr, 24 hr, etc.)
#define WAIT_TIME_BPR 20  // First 20 min of the hour (for Bottom Pressure Recorder)

// GPIO PIN SETTINGS ######
#define RTC_INTERRUPT_PIN   25   // RTC interrupt pin
#define BEACON_PIN          11          // enable 3.3V->12V board to power LED surface beacon
#define SALINITY_ENABLE_PIN 4  // enable pin for salinity board

// ANALOG PINS
#define THERMISTOR_PIN  A3  // Analog pin used for thermistor
#define BATTV_PIN       A2  // Analog pin used for battery voltage monitoring

// OTHER VALUES
#define SYSTEM_BAUD   115200  // Serial with computer
#define SALINITY_BAUD 9600  // baud rate of EZO circuit, default is 9600
#define WATER_DENSITY 1029  // 1029 is default seawater for Blue Robotics sensors

// SEALABCTD UTILILITY HEADER FILES
#include "globals.h"         // self made file
#include "display.h"         // Adafruit_SH110x and Adafruit_GFX
#include "ledDisplay.h"      // Functions for flashing the NeoPixel and orange beacon for surface assets
#include "battMonitoring.h"  // Battery monitoring functions

// SEALABCTD SENSOR HEADER FILES
#include "blueRoboticsPressure.h"  // Blue Robotics' pressure sensors
#include "blueRoboticsFastTemp.h"  // Blue Robotics' Fast Temperature sensor
#include "salinAtlas.h"            // Atlas Scientific Conductivity Sensor
#include "dallasTemp.h"            // DS18B20 "dallas" temperature sensors, OneWire library by Jim Studt, Tom Pollard
#include "therm.h"                 // Adafruit 10kΩ NTC thermistor
#include "pt100.h"                 // Adafruit PT100
#include "pressDF.h"               // DF Robot Analog Pressure Sensor (required 3.3V->5V buck)
#include "light.h"                 // 6-channel light sensor
                                   // ############################################################################################################

void setup() {
  // MUST CHANGE CLOCK SPEED BEFORE DOING ANYTHING ELSE
  // Needs more research and testing as sometimes it breaks the timing of certain sensors
  // If having issues sampling with sensors using master code but single sensor code works, use default clock speed 133MHz
  switch (deviceMode) {
    case 0:                             // fast sampling
      set_sys_clock_khz(133000, true);  // safer default
      // set_sys_clock_khz(200000, true); // overclocking, untested
      delay(5);
      yield();
      break;
    case 1:  // WAIT TIME 1
    case 2:  // WAIT TIME 2
    case 4:  // BPR
    case 5:  // PRESS_ONLY always recording pressure
      set_sys_clock_khz(50000, true);
      delay(5);
      yield();
      break;
    default:
      set_sys_clock_khz(133000, true);
      delay(5);
      yield();
      break;
  }

  if (displayBool) {            // Start display first to then show system boot process
    display.clearDisplay();     // Make sure nothing is in the buffer
    display.begin(0x3C, true);  // Address 0x3C default, specify address for use later shutting of the display
    display.setRotation(1);
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    display.display();
    delay(2500);  // display Adafruit splashscreen cuz we love Adafruit Industries
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("SeaLabCTD Start <3");
    display.display();
  }

  if (salinityBool) { salinSetup(); }  // SETUP SALINITY
  if (salinityBool && displayBool) {
    display.println("Salinity init");
    display.display();
  }

  if (serialDisplay) {
    Serial.begin(SYSTEM_BAUD);
    // while (!Serial) { delay(10); }  // THIS MAKES IT HANG SO NFG
    delay(5000);
    Serial.println("SYSTEM STARTUP");
  }

  analogReadResolution(12);  // specify in code for proper battV reference
  Wire.begin();

  delay(100);
  pinMode(THERMISTOR_PIN, INPUT);
  setupBatteryMonitoring((deviceMode == 3) ? CHARGE_MODE : SAMPLE_MODE);  // Battery LED policy
  blinkDeviceModeLED(deviceMode);

  if (!displayBool && (deviceMode == 1 || deviceMode == 2 || deviceMode == 4 || deviceMode == 5)) {
    uint32_t start = millis();
    while (millis() - start <= 7000) {  // keep LED on 7 seconds
      readBatteryVoltage();
      updateBatteryLED(battV);
      yield();
    }
    if (serialDisplay) Serial.println("Disabling LED after 7 sec");
    pixel.setPixelColor(0, 0);
    pixel.show();
  }

  if (dallasTempBool) { setupDallasTemp(); }
  if (dallasTempBool && displayBool) {
    display.println("dallasTemp init");
    display.display();
  }

  if (lightBool) { setupLight(); }
  if (lightBool && displayBool) {
    display.println("6-ch light init");
    display.display();
  }

  if (bar02Bool || bar30Bool || bar100Bool) {
    brPressureSetup();
    if (displayBool) {
      display.println("brPressure init");
      display.display();
    }
  }

  if (brFastTempBool) {
    brFastTempSetup();
    if (displayBool) {
      display.println("brFastTemp init");
      display.display();
    }
  }
  delay(200);

  // thermTemp does not have a setup function
  if (thermTempBool && displayBool) {
    display.println("thermTemp init");
    display.display();
  }

  if (pt100Bool) { pt100Setup(); }
  if (pt100Bool && displayBool) {
    display.println("PT100 init");
    display.display();
  }

  delay(200);

  if (!rtc.begin()) {
    if (serialDisplay) Serial.println("Could not find RTC");
    Serial.flush();
    while (1) {
      redFlash();  // without RTC we do not continue so flash Neopixel red
    }
  }

  rtc.writeSqwPinMode(DS3231_OFF);  // disable square wave because we want to use as interrupt

  if (rtc.lostPower()) {
    setRtcCompileTimeUTC();  // uses the timeZone variable already defined
  }

  // When time needs to be re-set on a previously configured device, the
  // following line sets the RTC to the date & time this sketch was compiled
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // time of computer, no UTC
  // setRtcCompileTimeUTC();

  rtc.clearAlarm(1);
  delay(2);  // tiny debounce delay for some DS3231 boards, not 100% sure if needed
  rtc.clearAlarm(2);
  delay(2);  // tiny debounce delay for some DS3231 boards, not 100% sure if needed

  pinMode(RTC_INTERRUPT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(RTC_INTERRUPT_PIN), rtcWakeISR, FALLING);

  if (displayBool) {
    display.println("RTC init");
    display.display();
  }
  delay(200);

  // Try to initialize SD card
  if (!SD.begin(config)) {
    if (serialDisplay) Serial.println("SD card initialization FAILED!");
    if (displayBool) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("SD init FAILED");
      display.println("Check wiring/config");
      display.display();
      delay(3000);
    }

    // Decide whether to halt or continue without SD
    while (1) {
      if (serialDisplay) Serial.println("No SD card logging possible. Halting...");
      redFlash();
    }
  } else {
    if (serialDisplay) Serial.println("SD card initialized OK");
    if (displayBool) {
      display.println("SD init OK");
      display.display();
    }
  }

  pinMode(BUTTON_A, INPUT_PULLUP);  // not used yet
  pinMode(BUTTON_B, INPUT_PULLUP);  // not used yet
  pinMode(BUTTON_C, INPUT_PULLUP);  // not used yet

  // Set the first alarm based on mode to start sampling on even numbers
  currentTime = rtc.now();

  if (deviceMode == 4) { bprSampling = (currentTime.minute() < 20); }

  if (deviceMode == 1) { nextSample = getNextAlarm(currentTime, WAIT_TIME_ONE);

    if (serialDisplay) { Serial.println("Initial Alarm One set");  }
    if (displayBool)   { alarmDisplay(WAIT_TIME_ONE, "Alarm One"); }

    rtc.setAlarm1(nextSample, DS3231_A1_Minute);

  } else if (deviceMode == 2) {
    nextSample = getNextAlarm(currentTime, WAIT_TIME_TWO);

    if (serialDisplay) Serial.println("Initial Alarm Two set");
    if (displayBool) { alarmDisplay(WAIT_TIME_TWO, "Alarm Two"); }

    rtc.setAlarm1(nextSample, DS3231_A1_Minute);

  } else if (deviceMode == 4) {
    // Decide initial window state once
    bprSampling = (currentTime.minute() < WAIT_TIME_BPR);

    nextSample = nextBprBoundary(currentTime);  // Set Alarm1 to the next boundary (either :20 or next :00), strictly > now
    rtc.setAlarm1(nextSample, DS3231_A1_Date);

    if (serialDisplay) {
      Serial.println(bprSampling
                       ? "BPR: in-window; Alarm One set to arm at twenty past the hour"
                       : "BPR: out-of-window; Alarm One set to fire @ next :00");
    }

    if (displayBool) { alarmDisplay(WAIT_TIME_BPR, "BPR Window"); }
  }

  if (deviceMode == 0 || deviceMode == 5) {
    // full timestamp for fast mode or PRESS_ONLY mode
    timestamp_filename = currentTime.timestamp(DateTime::TIMESTAMP_FULL);
    timestamp_filename.replace(":", "-");                       // Replace colons (not valid in filenames)
    if (serialDisplay) { Serial.println(timestamp_filename); }  // mode 0 has all time valyes YYYY-MM-DD hh-mm-ss
    myFile = SD.open(timestamp_filename + ".csv", FILE_WRITE);
    myFile.println(header);
    myFile.close();

  } else if (deviceMode == 1 || deviceMode == 2 || deviceMode == 4) {
    updateFilenameAndHeader();  // only create a day file for longer sample modes
  }
}

void runMode0() {
  if (salinityBool) {
    if (deviceModeZeroInit == false) {
      digitalWrite(4, HIGH);
      delay(900);
      Serial1.print("L,1\r");  // Light on so we know it's doing stuff
      delay(100);              // Found 100ms works. Probably can be shorter
      Serial1.print("C,2\r");  // Continuous sample on, we will send T in-situ temp compensation
      delay(100);              // Found 100ms works. Probably can be shorter
      deviceModeZeroInit = true;
    }
  }

  if (beaconBool) { tickBeacon(); }

  currentTime = rtc.now();
  if (bar02Bool || bar30Bool || bar100Bool) { brPressureSample(); }  // the || means or because there will only ever be one type connected at a time
  if (lightBool) {sampleLight(); }
  if (thermTempBool) { thermTemp = getThermTemp(); }
  if (pt100Bool) { pt100Temp = getPT100Temp(); }
  if (pressDFBool) { getPressureDF(); }
  if (dallasTempBool) { dallasTemp = getDallasTemp(); }
  if (brFastTempBool) { brFastTempSample(); }  // Immeditely before salinity as salinity uses temperature
  
  // salinLoopWithPC();
  // salinLoopWithoutPC(pt100Temp);

  // uint32_t saltTime = millis();
  // if ((uint32_t)(saltTime - lastSaltMs) >= 400UL) {
  // //   salinLoopWithPC();
  //   salinLoopWithoutPC(pt100Temp);

  // //   // if (salinityBool && brFastTempBool) { salinLoopWithoutPC(brFastTemp); }  // must be after temperature
  // //   // if (salinityBool && brFastTempBool) { salinLoopWithPC(); }  // must be after temperature
  // //   // debugSerial1Raw();
  // //   // else if (salinityBool && dallasTempBool) { salinLoopWithoutPC(dallasTemp);  }  // must be after temperature
  // //   // else if (salinityBool && dallasTempBool) { salinLoopWithPC();  }  // must be after temperature

  //   lastSaltMs = saltTime;
  // }

  readBatteryVoltage();
  writeDataRow();

  if (serialDisplay) { serialPrintValues(); }
  if (displayBool) { displayMode0(); }

  // OLED update, only if enough time has passed
  // if (displayBool && (millis() - lastOledMs >= oledInterval)) {
  //     lastOledMs = millis();
  //     displayMode0();
  // }

  // delay(20);  // no delay does not allow Serial to connect to IDE, and this  doesn't majorly effect sample speed
  yield();
}

void runMode1and2() {
  // only call this function in Mode 1, 2, or 4
  // we want a new file each day,
  updateFilenameAndHeader();  // only create a new file each day multi-day sample modes

  if (salinityBool) {
    if (deviceModeOneInit == false || deviceModeTwoInit == false) {  // the || means or
      digitalWrite(4, HIGH);
      delay(1200);
      Serial1.print("L,0\r");  // light off to save power
      delay(100);
      Serial1.print("C,0\r");  // continuous sample off
      delay(100);
      digitalWrite(4, LOW);      // once configured we can turn off till we need it to sample
      deviceModeOneInit = true;  // ensure this chunk only runs once
      deviceModeTwoInit = true;
    }
  }

  if (displayBool == true) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Salinity powered off");
    display.println("Display turning off");
    display.print("Goodbye!");
    display.display();
    delay(4000);

    oledSleep();  // Put the SH1107 into power-save (microamps)
  }

  while (!rtcAlarmFired) {
    __wfi();  // CPU halts until RTC interrupt triggers
  }
  delay(10);  // stability from wakeup

  currentTime = rtc.now();

  if (bar02Bool || bar30Bool || bar100Bool) { brPressureSample(); }  // the || means or because there will only ever be one type connected at a time
  if (thermTempBool) { thermTemp = getThermTemp(); }
  if (pt100Bool) { pt100Temp = getPT100Temp(); }
  if (pressDFBool) { getPressureDF(); }
  if (dallasTempBool) { dallasTemp = getDallasTemp(); }
  if (brFastTempBool) { brFastTempSample(); }  // Immeditely before salinity as salinity uses temperature

  if (salinityBool) {
    digitalWrite(4, HIGH);  // Turn on to sample
    delay(1200);
    salinLoopWithoutPC(dallasTemp);
    // salinLoopWithoutPC(dallasTemp);  // must be after temperature
    delay(1200);
    digitalWrite(4, LOW);  // Turn off after sampling to save power
  }

  readBatteryVoltage();
  writeDataRow();
  if (serialDisplay) { serialPrintValues(); }

  // Re-arm alarm
  rtcAlarmFired = false;
  rtc.clearAlarm(1);

  if (deviceMode == 1) {
    if (serialDisplay) Serial.println("Alarm One Trigger");
    DateTime alarmOne = getNextAlarm(currentTime, WAIT_TIME_ONE);
    rtc.setAlarm1(alarmOne, DS3231_A1_Minute);
  } else if (deviceMode == 2) {
    if (serialDisplay) Serial.println("Alarm Two Trigger");
    DateTime alarmTwo = getNextAlarm(currentTime, WAIT_TIME_TWO);
    rtc.setAlarm1(alarmTwo, DS3231_A1_Minute);
  }
}

void runMode4() {
  // only call this function in Mode 1, 2, or 4
  updateFilenameAndHeader();  // only create a new file each day multi-day sample modes

  static uint32_t lastSampleMs = 0;

  currentTime = rtc.now();

  // FOR TESTING to bypass first 20 MIN of hour alarm
  // bprSampling = true;

  // Sample only if within window (:00–:20) of hour
  if (bprSampling) {
    const uint32_t nowMs = millis();
    if (nowMs - lastSampleMs >= 200) {
      lastSampleMs = nowMs;

      if (bar02Bool || bar30Bool || bar100Bool) { brPressureSample(); }  // Only have one sensor in this platform
      readBatteryVoltage();

      writeDataRow();

      if (serialDisplay) {
        Serial.print("battV: ");
        Serial.print(battV);
      }
      if (serialDisplay) {
        Serial.print(" brPress: ");
        Serial.println(brPressure);
      }
    }
  } else {
    // Outside sampling window
    while (!rtcAlarmFired) {
      __wfi();  // CPU halts until RTC interrupt triggers
    }
  }

  if (rtcAlarmFired) {
    rtcAlarmFired = false;
    rtc.clearAlarm(1);

    bprSampling = !bprSampling;

    DateTime now2 = rtc.now();
    DateTime nextBoundary = nextBprBoundary(now2);
    if (nextBoundary <= now2) {
      nextBoundary = nextBprBoundary(DateTime(now2.unixtime() + 1));
    }
    rtc.setAlarm1(nextBoundary, DS3231_A1_Date);

    if (serialDisplay) {
      Serial.print("BPR: ");
      Serial.print(bprSampling ? "START " : "STOP ");
      Serial.print("sampling; next boundary @ ");
      Serial.println(nextBoundary.timestamp(DateTime::TIMESTAMP_FULL));
      Serial.print("battV");
      Serial.println(battV);
    }
  }
  yield();
}

void runMode5() {

  currentTime = rtc.now();
  if (bar02Bool || bar30Bool || bar100Bool) { brPressureSample(); }
  else   // Only have one sensor in this platform
  readBatteryVoltage();

  writeDataRow();
  serialPrintValues();
    
  delay(20); // run at ~13Hz
  // No delay is ~17Hz
  yield();
}

void loop() {
  // Central dispatcher for different modes
  switch (deviceMode) {
    case 0:
      runMode0(); // Profile
      break;
    case 1:
    case 2:
      runMode1and2(); // WAIT_TIME_ONE or WAIT_TIME_TWO
      break;
    case 4:
      runMode4(); // BPR
      break;
    case 5:
      runMode5(); // PRESS_ONLY profile
      break;
    default:
      redFlash();
      if (serialDisplay) Serial.println("INVALID MODE");
      break;
  }

  // Code that will run in all modes goes here
}
