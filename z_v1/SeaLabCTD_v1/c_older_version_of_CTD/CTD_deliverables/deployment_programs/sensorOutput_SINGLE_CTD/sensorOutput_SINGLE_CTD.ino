 //SPECIAL THANKS TO VIZI (Jasper Scozzafava) FOR HELPING CODE - esp the SD and creation of data log file 

//Wiring diagrams in "Wiring" folder

//Base code to test each sensor individually found in "Sensor Codes"
  //Salinity - Salin_Sens_Test
  //Temperature - Temp_Sens_Test
  //Pressure - Press_Sens
  //SD - SD_test

//Datasheets for sensors in "Datasheets_Wiring"
  

//Salinity
#include <SoftwareSerial.h>
#define EZO_RX_PIN 2
#define EZO_TX_PIN 3
SoftwareSerial ezoSerial(EZO_RX_PIN, EZO_TX_PIN);   // EZO-EC conductivity circuit (UNO pin 2 <- its TX, pin 3 -> its RX)
String pcCommand = "";
String ezoReply = "";
boolean pcCommandReady = false;
boolean ezoReplyReady = false;

//Temp
#include <OneWire.h>                   //Include the correct libraries 
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 7                 // Data wire is plugged into digital pin 7
OneWire oneWire(ONE_WIRE_BUS);         // Setup a oneWire instance to communicate with any OneWire device
DallasTemperature sensors(&oneWire);  // Pass oneWire reference to DallasTemperature library
int deviceCount = 0;
float tempC;

//Pressure
#include <Wire.h>
#include <SparkFun_MS5803_I2C.h>      // Click here to get the library: http://librarymanager/All#SparkFun_MS5803-14BA
MS5803 sensor(ADDRESS_HIGH);
double pressure_abs, pressure_relative, altitude_delta, pressure_baseline;

//Vizi
//#include <string>
#include <SD.h>
//char filename[20];
int one_flag = 0;
int time_minute = 0, time_second = 0;
unsigned long start_time = millis(), curr_time = 0;

File test_init;
File sd_log;

void setup() {
  Serial.begin(9600);
  ezoSerial.begin(9600);
  pcCommand.reserve(10);
  ezoReply.reserve(30);
  
  sensors.begin();  // Start up the library
  deviceCount = sensors.getDeviceCount();
  
  Wire.begin();
  sensor.reset();
  sensor.begin();

  ////////
  // VIZI - SD card init
  delay(500);
  //sd_serial.begin(9600);   // Baud for sd card // VIZI - not needed for SD card I think
  
  Serial.print("Initializing SD card");
  Serial.println("...");

  // arduino output -> sd card 10
  pinMode(10, OUTPUT);

  if (!SD.begin(10)) { // if 10 not active
    Serial.println("[[error]] sd card bad init");
    return; // close setup  
  }
  Serial.println("init done");

}


// MOVED DUE TO FUNCTION ERROR
void serialEvent() {
  pcCommand = Serial.readStringUntil(13);
  pcCommandReady = true;
}

void time_master() {
  curr_time = millis();

  int inner_seconds = ((curr_time - start_time) / 1000);
  time_minute = inner_seconds / 60;
  time_second = inner_seconds % 60;
}

void sd_logfile() {
  ////////
  // VIZI - SD card logging start
  // Must open every loop. Might as well toss it here so it's easier to use
  // Should only get here if SD card init has been valid
  // Don't need to copy the code once more
  sd_log = SD.open("log.txt", FILE_WRITE);
  if (!sd_log) {
    Serial.println("[[ SD LOG INACTIVE ]]");
  }
  else if (sd_log) {
    Serial.println("SD_LOG active");
    Serial.print("time is : ");
    Serial.print(time_minute);
    Serial.print(":");
    if (time_second < 10) {
        Serial.print('0');
    }
    Serial.println(time_second);

    if (one_flag <= 0) {
      sd_log.println("########################");
      sd_log.println("########################");
      sd_log.println("########################");
      sd_log.println("###### NEW START #######");
      sd_log.println("########################");
      sd_log.println("########################");
      sd_log.println("########################");
      one_flag++;
    }

  }
  // call this function at the top of the loop 
  // be sure to close it at the end
  ////////   
}

void loop() {
  time_master(); // gimme the time

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
    ////////
    // VIZI - enable log file logging
    sd_logfile();
    ////////

    if (isdigit(ezoReply[0]) == false) {
      Serial.println(ezoReply);
    }
    else
    {
      ////////
      // VIZI - print time elapsed
      sd_log.print(time_minute);
      sd_log.print(":");
      if (time_second < 10) {
        sd_log.print('0');
      }
      sd_log.print(time_second);
      sd_log.print(",");
      ////////

      handleEzoReading();
    }
    ezoReply = "";
    ezoReplyReady = false;
     
    pressure_abs = sensor.getPressure(ADC_256);
    Serial.println(pressure_abs);

    ////////
    // VIZI - Print the presure, and newline
    sd_log.println(pressure_abs);
    ////////
  }
  
  ////////
  // VIZI - close and save
  sd_log.close();
  ////////
}

// A reading from the EZO-EC: its fields are comma-separated (EC first)
void handleEzoReading(void) {
        

  char replyChars[30];
  char *EC;
  float ecValue;
  
  ezoReply.toCharArray(replyChars, 30);
  EC = strtok(replyChars, ",");
 
  Serial.print(EC);
  Serial.print(",");

  ////////
  // VIZI - print EC data to card
  sd_log.print(EC);
  sd_log.print(",");
  ////////

  sensors.requestTemperatures(); 
  
  // goes through all devices, prints by index at sensors
  for (int i = 0;  i < deviceCount;  i++)
  {
   tempC = sensors.getTempCByIndex(i);
   Serial.print(tempC);
   Serial.print(",");

   /////////
   // VIZI - print remaining sensors
   sd_log.print(tempC);
   sd_log.print(",");
   ////////
  }

}
