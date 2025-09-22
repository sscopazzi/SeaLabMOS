#include "Adafruit_AS726x.h"

//create the object
Adafruit_AS726x ams;

//buffer to hold raw values
uint16_t sensorValues[AS726x_NUM_CHANNELS];

//buffer to hold calibrated values (not used by default in this example)
// uint16_t calibratedValues[AS726x_NUM_CHANNELS];

extern bool serialDisplay;

extern float lt450, lt500, lt550, lt570, lt600, lt650;

void setupLight(){
  if (!ams.begin() ) {
    if (serialDisplay) { Serial.println("Could not connect to sensor! Please check your wiring."); }
    while(1);
  }
} 

void sampleLight() {

  // uint8_t lightTemp = ams.readTemperature();   //read the device temperature
  
  //ams.drvOn(); //uncomment this if you want to use the driver LED for readings
  ams.startMeasurement(); //begin a measurement
  
  bool rdy = false;   //wait till data is available
  while(!rdy){
    delay(5);
    rdy = ams.dataReady();
  }

  //ams.drvOff(); //uncomment this if you want to use the driver LED for readings

  ams.readRawValues(sensorValues);
  lt450 = sensorValues[AS726x_VIOLET];
  lt500 = sensorValues[AS726x_BLUE];
  lt550 = sensorValues[AS726x_GREEN];
  lt570 = sensorValues[AS726x_YELLOW];
  lt600 = sensorValues[AS726x_ORANGE];
  lt650 = sensorValues[AS726x_RED];


  // ams.readCalibratedValues(calibratedValues); // this requires the onboard LED to be powered?

  // lt450 = calibratedValues[AS726x_VIOLET];
  // lt500 = calibratedValues[AS726x_BLUE];
  // lt550 = calibratedValues[AS726x_GREEN];
  // lt570 = calibratedValues[AS726x_YELLOW];
  // lt600 = calibratedValues[AS726x_ORANGE];
  // lt650 = calibratedValues[AS726x_RED];

}





/***************************************************************************
  This is a library for the Adafruit AS7262 6-Channel Visible Light Sensor

  This sketch reads the sensor

  Designed specifically to work with the Adafruit AS7262 breakout
  ----> http://www.adafruit.com/products/3779
  
  These sensors use I2C to communicate. The device's I2C address is 0x49
  Adafruit invests time and resources providing this open source code,
  please support Adafruit andopen-source hardware by purchasing products
  from Adafruit!
  
  Written by Dean Miller for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ***************************************************************************/