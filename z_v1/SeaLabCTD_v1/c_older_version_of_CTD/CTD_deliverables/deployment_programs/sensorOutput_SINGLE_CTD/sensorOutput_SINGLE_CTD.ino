 //SPECIAL THANKS TO VIZI (Jasper Scozzafava) FOR HELPING CODE - esp the SD and creation of data log file 

//Wiring diagrams in "Wiring" folder

//Base code to test each sensor individually found in "Sensor Codes"
  //Salinity - Salin_Sens_Test
  //Temperature - Temp_Sens_Test
  //Pressure - Press_Sens
  //SD - SD_test

//Datasheets for sensors in "Datasheets_Wiring"
  

//Salinity
#include <SoftwareSerial.h>                           //we have to include the SoftwareSerial library, or else we can't use it
#define rx 2                                          //define what pin rx is going to be
#define tx 3                                          //define what pin tx is going to be
SoftwareSerial myserial(rx, tx);                      //define how the soft serial port is going to work
String inputstring = "";                              //a string to hold incoming data from the PC
String sensorstring = "";                             //a string to hold the data from the Atlas Scientific product
boolean input_string_complete = false;                //have we received all the data from the PC
boolean sensor_string_complete = false;               //have we received all the data from the Atlas Scientific product

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

void setup() {                                        //set up the hardware
  Serial.begin(9600);                                 //set baud rate for the hardware serial port_0 to 9600
  myserial.begin(9600);                               //set baud rate for the software serial port to 9600
  inputstring.reserve(10);                            //set aside some bytes for receiving data from the PC
  sensorstring.reserve(30);                           //set aside some bytes for receiving data from Atlas Scientific product
  
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
void serialEvent() {                                  //if the hardware serial port_0 receives a char
  inputstring = Serial.readStringUntil(13);           //read the string until we see a <CR>
  input_string_complete = true;                       //set the flag used to tell if we have received a completed string from the PC
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

void loop() {                                         //here we go...
  time_master(); // gimme the time

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
    ////////
    // VIZI - enable log file logging
    sd_logfile();
    ////////

    if (isdigit(sensorstring[0]) == false) {          //if the first character in the string is a digit
      Serial.println(sensorstring);                   //send that string to the PC's serial monitor
    }
    else                                              //if the first character in the string is NOT a digit
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

      print_EC_data();                                //then call this function 
    }
    sensorstring = "";                                //clear the string
    sensor_string_complete = false;                   //reset the flag used to tell if we have received a completed string from the Atlas Scientific product
     
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

void print_EC_data(void) {         //this function will parse the string 
        

  char sensorstring_array[30];                        //we make a char array
  char *EC;                                           //char pointer used in string parsing
  float f_ec;                                         //used to hold a floating point number that is the EC
  
  sensorstring.toCharArray(sensorstring_array, 30);   //convert the string to a char array 
  EC = strtok(sensorstring_array, ",");               //let's pars the array at each comma
 
  Serial.print(EC);                                 //this is the EC value
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
