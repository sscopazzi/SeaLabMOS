// salinity
#include <SoftwareSerial.h>
#define EZO_RX_PIN 2
#define EZO_TX_PIN 3
SoftwareSerial ezoSerial(EZO_RX_PIN, EZO_TX_PIN);   // EZO-EC conductivity circuit (UNO pin 2 <- its TX, pin 3 -> its RX)
String pcCommand = "";
String ezoReply = "";
boolean pcCommandReady = false;
boolean ezoReplyReady = false;

//data init
double pressure_abs, pressure_relative, altitude_delta, pressure_baseline;
float sensTemp, sensPressure, sensDepth;

// pressure & temp
#include <Wire.h>
#include "MS5837.h"
MS5837 newSensor;

// VIZI
//#include <string>
#include <SD.h>
char filename[20];
int linecount = 0;
int one_flag = 0;
int time_minute = 0, time_second = 0;
unsigned long start_time = millis(), curr_time = 0;

// file stuff
File sd_count;
File sd_log;

////////
// VIZI - creates valid file name (skipping previously used names)
//  + attempts to create a file named "log_###.txt"
//    if file already exists, increment count and try again
void file_namer() {
		int count = 0;
		snprintf(filename, sizeof(filename), "log_%03d.txt", count);
		while (SD.exists(filename)) {
				count++;
				snprintf(filename, sizeof(filename), "log_%03d.txt", count);
		}
}
////////

////////
// VIZI - calculate elapsed time
//  + grabs starting time in milliseconds, (essentially 0)
//    subtracts start time from current time
//    divide by 1000 to convert to seconds
//
//  + divide by 60 to convert to minutes
//  + modulo by 60 to convert remaining seconds
void time_master() {
		curr_time = millis();

		int inner_seconds = ((curr_time - start_time) / 1000);
		time_minute = inner_seconds / 60;
		time_second = inner_seconds % 60;
}
////////

////////
// VIZI - Start logging and output debug to console
//  + open log file using 'char filename[20]' created in file_namer()
//  
//  + if log file is NOT opened properly, output error to console
//    otherwise, announce active, filename, lines of data, runtime
void sd_logfile() {
	sd_log = SD.open(filename, FILE_WRITE);
	if (!sd_log) {
		Serial.println("[[ SD LOG INACTIVE ]]");
		Serial.println("[[ !! DATA NOT BEING RECORDED !! ]]");
	}
	else if (sd_log) {
		Serial.println("[[ SD_LOG ACTIVE ]]");
		Serial.print("filename : ");
		Serial.println(filename);
		Serial.print("lines of data : ");
		Serial.println(linecount);

		// print time to console
		Serial.print("runtime (minutes : seconds) : ");
		Serial.print(time_minute);
		Serial.print(":");
		if (time_second < 10) {
				Serial.print('0');
		}
		Serial.println(time_second);
	}
}
////////

// MOVED OUT OF LOOP DUE TO FUNCTION ERROR
void serialEvent() {
	pcCommand = Serial.readStringUntil(13);
	pcCommandReady = true;
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

  // sensors.requestTemperatures(); 
  
  // // goes through all devices, prints by index at sensors
  // for (int i = 0;  i < deviceCount;  i++) {
  //  tempC = sensors.getTempCByIndex(i);
  //  Serial.print(tempC);
  //  Serial.print(",");

  //  /////////
  //  // VIZI - print remaining sensors
  //  sd_log.print(tempC);
  //  sd_log.print(",");
  //  ////////
  //}

}

void setup() {
		// computer stuff

		Serial.begin(9600);                                 

		ezoSerial.begin(9600);                               
		
		// salinity sensor setup

		pcCommand.reserve(10);

		ezoReply.reserve(30);                           

		// begin wire stuff
		Wire.begin();

		// Initialize pressure sensor
		// Returns true if initialization was successful
		// We can't continue with the rest of the program unless we can initialize the sensor
		while (!newSensor.init()) {
		Serial.println("Init failed!");
		Serial.println("Are SDA/SCL connected correctly?");
		Serial.println("Blue Robotics Bar30: White=SDA, Green=SCL");
		Serial.println("\n\n\n");
		delay(5000);
	}

		// .init sets the sensor model for us but we can override it if required.
		// Uncomment the next line to force the sensor model to the MS5837_30BA.
		//sensor.setModel(MS5837::MS5837_30BA);

		newSensor.setFluidDensity(1029); // kg/m^3 (freshwater, 997 for freshwater)

		////////
		// VIZI - SD card init
		delay(500);

		Serial.print("Initializing SD card");
		Serial.println("...");

		// arduino output -> sd card 10
		pinMode(10, OUTPUT);

		if (!SD.begin(10)) { // if 10 not active
		Serial.println("[[error]] sd card bad init");
		return; // close setup  
		}
		Serial.println("init done");

		file_namer();
		////////

}

void loop() {
	// gets time
	time_master();

	////////
	// INPUT INFORMATION
	if (pcCommandReady == true) {
		ezoSerial.print(pcCommand);
		ezoSerial.print('\r');
		pcCommand = "";
		pcCommandReady = false;
	}
	////////
	
	///////
	// READ SALINITY SENSOR
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
		} else {
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

			// get data from MSS5837
			newSensor.read();
			
			// temp data capture
			sensTemp = newSensor.temperature();
			Serial.println(sensTemp);
			// pressure data capture
			sensPressure = newSensor.pressure();
			Serial.println(sensPressure);
			// depth data capture
			sensDepth = newSensor.depth();
			Serial.println(sensDepth);

			////////
			// VIZI - Print the temp, pressure, depth with newlines and incrementations
			linecount++;
			sd_log.println(sensTemp);
			linecount++;
			sd_log.println(sensPressure);
			linecount++;
			sd_log.println(sensDepth);
			////////
		}

		ezoReply = ""; // reset string
		ezoReplyReady = false; // reset capture flag
	}

	////////
	// VIZI - close and save
	sd_log.close();
	////////
}
