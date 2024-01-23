  _    _                      _                          _____ _______ _____  
 | |  | |                    | |                        / ____|__   __|  __ \ 
 | |__| | ___  _ __ ___   ___| |__  _ __ _____      __ | |       | |  | |  | |
 |  __  |/ _ \| '_ ` _ \ / _ \ '_ \| '__/ _ \ \ /\ / / | |       | |  | |  | |
 | |  | | (_) | | | | | |  __/ |_) | | |  __/\ V  V /  | |____   | |  | |__| |
 |_|  |_|\___/|_| |_| |_|\___|_.__/|_|  \___| \_/\_/    \_____|  |_|  |_____/ 

// Order of CSV
// EC , TDS , SAL , mbar, C , meters 

//This code was written in the Arduino 1.8.57.0

// TDS - Total Dissolved Solids
// 12,940 for 12880 23.0C      12,396 is actual thing to input 
// 80,000 for 61601 at 22.4C   76,048 as actual thing to input 

//Blue Robotics
#include <Wire.h>
#include "MS5837.h"
MS5837 sensor;

//EC
#include <SoftwareSerial.h>                           //we have to include the SoftwareSerial library, or else we can't use it
#define rx 2                                          //define what pin rx is going to be
#define tx 3                                          //define what pin tx is going to be

SoftwareSerial myserial(rx, tx);                      //define how the soft serial port is going to work

String inputstring = "";                              //a string to hold incoming data from the PC
String sensorstring = "";                             //a string to hold the data from the Atlas Scientific product
boolean input_string_complete = false;                //have we received all the data from the PC
boolean sensor_string_complete = false;               //have we received all the data from the Atlas Scientific product

//SD
#include <SD.h>
File dataFile;

void setup() {                                        //set up the hardware
  Serial.begin(9600);                                 //set baud rate for the hardware serial port_0 to 9600
  
  myserial.begin(9600);                               //set baud rate for the software serial port to 9600
  inputstring.reserve(10);                            //set aside some bytes for receiving data from the PC
  sensorstring.reserve(30);                           //set aside some bytes for receiving data from Atlas Scientific product

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

void serialEvent() {                                  //if the hardware serial port_0 receives a char
  inputstring = Serial.readStringUntil(13);           //read the string until we see a <CR>
  input_string_complete = true;                       //set the flag used to tell if we have received a completed string from the PC
}

void loop() {

  if (input_string_complete == true) {                //if a string from the PC has been received in its entirety
    myserial.print(inputstring);                      //send that string to the Atlas Scientific product
    myserial.print('\r');                             //add a <CR> to the end of the string
    inputstring = "";                                 //clear the string
    input_string_complete = false;                    //reset the flag used to tell if we have received a completed string from the PC
  }

  if (myserial.available() > 0) {                     //if we see that the Atlas Scientific product has sent a character
    char inchar = (char)myserial.read();              //get the char we just received
    sensorstring += inchar;                           //add the char to the var called sensorstring
    if (inchar == '\r') {                             //if the incoming character is a <CR>
      sensor_string_complete = true;                  //set the flag
    }
  }

  if (sensor_string_complete == true) {               //if a string from the Atlas Scientific product has been received in its entirety
    if (isdigit(sensorstring[0]) == false) {          //if the first character in the string is a digit
      Serial.println(sensorstring);                   //send that string to the PC's serial monitor
    }
    else                                              //if the first character in the string is NOT a digit
    {
      print_EC_data();                                //then call this function 
    }
    sensorstring = "";                                //clear the string
    sensor_string_complete = false;                   //reset the flag used to tell if we have received a completed string from the Atlas Scientific product
  }
}

void print_EC_data(void) {                            //this function will pars the string  

  char sensorstring_array[30];                        //we make a char array
  char *EC;                                           //char pointer used in string parsing
  char *TDS;                                          //char pointer used in string parsing
  char *SAL;                                          //char pointer used in string parsing
  char *GRAV;                                         //char pointer used in string parsing
  float f_ec;                                         //used to hold a floating point number that is the EC
  
  sensorstring.toCharArray(sensorstring_array, 30);   //convert the string to a char array 
  EC = strtok(sensorstring_array, ",");               //let's pars the array at each comma
  TDS = strtok(NULL, ",");                            //let's pars the array at each comma
  SAL = strtok(NULL, ",");                            //let's pars the array at each comma
  GRAV = strtok(NULL, ",");                           //let's pars the array at each comma

//PRINT TO SERIAL MONITOR
  //Serial.print("EC:");     //we now print each value we parsed separately
  Serial.print(EC);          //this is the EC value
  Serial.print(",");

  //Serial.print("TDS:");    //we now print each value we parsed separately
  Serial.print(TDS);         //this is the TDS value
  Serial.print(",");
  
  //Serial.print("SAL:");    //we now print each value we parsed separately
  Serial.print(SAL);         //this is the salinity value
  Serial.print(",");
  
  //Serial.print("GRAV:");   //we now print each value we parsed separately
  Serial.print(GRAV);        //this is the specific gravity
  Serial.print(",");
  //Serial.println();        //this just makes the output easier to read
  
//f_ec= atof(EC);            //uncomment this line to convert the char to a float

///////BLUE ROBOT, read sensor
sensor.read();
  Serial.print(sensor.pressure()); //mbar
  Serial.print(",");
  Serial.print(sensor.temperature()); //C
  Serial.print(",");
  Serial.print(sensor.depth()); //meters
  Serial.println();
  
  delay(250); //found this delay works best

//Save EC data to SD card
File dataFile = SD.open("dataFile.txt", FILE_WRITE);

  dataFile.print(EC);       //this is the EC value
  dataFile.print(",");
  dataFile.print(TDS);      //this is the TDS value
  dataFile.print(",");
  dataFile.print(SAL);      //this is the salinity value
  dataFile.print(",");
  //dataFile.print(GRAV);   //this is the specific gravity
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
  
delay(750);
}
