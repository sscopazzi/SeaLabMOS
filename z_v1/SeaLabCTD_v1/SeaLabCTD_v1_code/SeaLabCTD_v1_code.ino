// Order of CSV
// EC , TDS , SAL , mbar, C , meters 
// TDS - Total Dissolved Solids

//This code was written in the Arduino 1.8.57.0
// Old cal values, unknown date
// 12,940 for 12880 23.0C      12,396 is actual thing to input 
// 80,000 for 61601 at 22.4C   76,048 as actual thing to input 

// Salinity calibrated 2024.4.20 

//Blue Robotics
#include <Wire.h>
#include "MS5837.h"
MS5837 sensor;

//EC
#include <SoftwareSerial.h>
#define EZO_RX_PIN 2
#define EZO_TX_PIN 3

SoftwareSerial ezoSerial(EZO_RX_PIN, EZO_TX_PIN);   // EZO-EC conductivity circuit (UNO pin 2 <- its TX, pin 3 -> its RX)

String pcCommand = "";
String ezoReply = "";
boolean pcCommandReady = false;
boolean ezoReplyReady = false;

//SD
#include <SD.h>
File dataFile;

void setup() {
  Serial.begin(9600);
  
  ezoSerial.begin(9600);
  pcCommand.reserve(10);
  ezoReply.reserve(30);

//BLUE ROBOT
  // Initialize pressure sensor
  // Returns true if initialization was successful
  // We can't continue with the rest of the program unless we can initialize the sensor
  Wire.begin();
  while (!sensor.init()) {
    Serial.println("Init failed!");
    Serial.println("Are SDA/SCL connected correctly?");
    Serial.println("Blue Robotics Bar30: White=SDA, Green=SCL");
    Serial.println("\n\n\n");
    delay(5000);
  }
  
  sensor.setModel(MS5837::MS5837_30BA);
  sensor.setFluidDensity(1029); //kg/m^3 (saltwater, 997 for freshwater)

//sets the output from the Arduino to the micro SD reader
   pinMode(10, OUTPUT);
 if (!SD.begin(10)) {
    Serial.println("initialization bad. check card"); //if doesn't init
    return;
  }
  Serial.println("initialization done."); //if does init

File dataFile = SD.open("dataFile.txt", FILE_WRITE);
  dataFile.print("#################################");//between data when turned on/off between deployments
  dataFile.println();
  dataFile.close();

}

void serialEvent() {
  pcCommand = Serial.readStringUntil(13);
  pcCommandReady = true;
}

void loop() {

  if (pcCommandReady == true) {
    ezoSerial.print(pcCommand);
    ezoSerial.print('\r');
    pcCommand = "";
    pcCommandReady = false;
  }

  if (ezoSerial.available() > 0) {
    char rxChar = (char)ezoSerial.read();
    ezoReply += rxChar;
    if (rxChar == '\r') {
      ezoReplyReady = true;
    }
  }

  if (ezoReplyReady == true) {
    if (isdigit(ezoReply[0]) == false) {
      Serial.println(ezoReply);
    }
    else
    {
      handleEzoReading();
    }
    ezoReply = "";
    ezoReplyReady = false;
  }
}

// A reading from the EZO-EC: its fields are comma-separated (EC first)
void handleEzoReading(void) {

  char replyChars[30];
  char *EC;
  char *TDS;
  char *SAL;
  char *GRAV;
  float ecValue;
  
  ezoReply.toCharArray(replyChars, 30);
  EC = strtok(replyChars, ",");
  TDS = strtok(NULL, ",");
  SAL = strtok(NULL, ",");
  GRAV = strtok(NULL, ",");

//PRINT TO SERIAL MONITOR
  //Serial.print("EC:");
//  Serial.print(EC);
//  Serial.print(",");

  //Serial.print("TDS:");
//  Serial.print(TDS);
//  Serial.print(",");
  
  //Serial.print("SAL:");
//  Serial.print(SAL);
//  Serial.print(",");
  
  //Serial.print("GRAV:");
//  Serial.print(GRAV);
//  Serial.print(",");
  //Serial.println();
  
//ecValue= atof(EC);

///////BLUE ROBOT, read sensor
//sensor.read();
//  Serial.print(sensor.pressure()); //mbar
//  Serial.print(",");
//  Serial.print(sensor.temperature()); //C
//  Serial.print(",");
//  Serial.print(sensor.depth()); //meters
//  
//  Serial.println();
  
//  delay(250); //found this delay works best

//Save EC data to SD card
File dataFile = SD.open("dataFile.txt", FILE_WRITE);

  dataFile.print(EC);
  dataFile.print(",");
  dataFile.print(TDS);
  dataFile.print(",");
  dataFile.print(SAL);
  dataFile.print(",");
  //dataFile.print(GRAV);
  //dataFile.print(",");
  
//Save Blue Robot data to SD card
sensor.read();
  dataFile.print(sensor.pressure());    //mbar
  dataFile.print(",");
  dataFile.print(sensor.temperature()); //C
  dataFile.print(",");
  dataFile.print(sensor.depth());       //meters
  
  dataFile.println();

// closes the file to ensure the file was written:
    dataFile.close();
  
delay(100);
}