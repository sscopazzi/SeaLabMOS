//COAST IMU data logger Arduino code, by Carson Alexander // modified by ACL 
 
//includes necessary libraries
#include <SD.h>
#include <Wire.h>

//declares the test text file and the IMUDATA text file
File test;
File IMUDATA;
 

void setup(void){
//delays the program to begin once the instrumentation is in the housing and ready to deploy
delay(500);
//sets the baud rate
  Serial.begin(9600);
 
//prints to the serial monitor to ensure SD is initialized correctly
  Serial.print("Initializing SD card");
  Serial.println("...");
//sets the output from the Arduino to the micro SD reader
   pinMode(10, OUTPUT);
 
  if (!SD.begin(10)) {
    Serial.println("initialization bad. check card");
    return;
  }
  Serial.println("initialization done.");



 
//creates a test file to ensure proper operation
  test = SD.open("test.txt", FILE_WRITE);
 
//if the file opened alright, write a test to it and say it is all good:
  if (test) {
    Serial.print("SD card read ok...");
    test.println("testing it works");
// closes the file to ensure the file was written:
    test.close();
    Serial.println("done.");
 
  }
}
void loop(void){
 
}
 
