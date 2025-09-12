#pragma once
// /***************************************************************/
// Atlas Scientific EZO-EC (UART) interface for SeaLabCTDv2.
// - Initializes EC sensor (salinSetup)
// - Parses EC/TDS/Salinity/Gravity values (print_EC_data)
// - Supports PC passthrough mode and autonomous mode with
//    temperature compensation (salinLoopWithPC / salinLoopWithoutPC).
// - This code was written in the Arduino 2.3.2 IDE
// /***************************************************************/

String inputstring = "";   //a string to hold incoming data from the PC
String sensorstring = "";  //a string to hold the data from the Atlas Scientific product

extern float brFastTemp;

inline void salinSetup() {
  Serial1.begin(SALINITY_BAUD);       //set baud rate for software serial port_3 to 9600 (default of sensor)
  inputstring.reserve(10);   //set aside some bytes for receiving data from the PC
  sensorstring.reserve(30);  //set aside some bytes for receiving data from Atlas Scientific product
  pinMode(SALINITY_ENABLE_PIN, OUTPUT);   // enable pin for salinity board
  digitalWrite(SALINITY_ENABLE_PIN, LOW); // turn off at system start
}


inline void print_EC_data(void) {  //this function will pars the string

  char sensorstring_array[30];  //we make a char array
  char *EC;                     //char pointer used in string parsing
  char *TDS;                    //char pointer used in string parsing
  char *SAL;                    //char pointer used in string parsing
  char *GRAV;                   //char pointer used in string parsing
  // float f_ec;                                         //used to hold a floating point number that is the EC

  sensorstring.toCharArray(sensorstring_array, 30);  //convert the string to a char array
  // Serial.println(sensorstring_array);              //debugging
  EC = strtok(sensorstring_array, ",");  //let's pars the array at each comma
  TDS = strtok(NULL, ",");               //let's pars the array at each comma
  SAL = strtok(NULL, ",");               //let's pars the array at each comma
  GRAV = strtok(NULL, ",");              //let's pars the array at each comma

  // Serial.print("EC:");                                //we now print each value we parsed separately
  // Serial.println(EC);                                 //this is the EC value

  // Serial.print("TDS:");                               //we now print each value we parsed separately
  // Serial.println(TDS);                                //this is the TDS value

  // Serial.print("SAL:");                               //we now print each value we parsed separately
  // Serial.println(SAL);                                //this is the salinity value

  // Serial.print("GRAV:");                              //we now print each value we parsed separately
  // Serial.println(GRAV);                               //this is the specific gravity
  // Serial.println();                                   //this just makes the output easer to read

  ec = atof(EC);      //uncomment this line to convert the char to a float
  sal = atof(SAL);    //uncomment this line to convert the char to a float
  tds = atof(TDS);    //uncomment this line to convert the char to a float
  grav = atof(GRAV);  //uncomment this line to convert the char to a float
}

void salinLoopWithPC() {
  if (Serial.available()) {                    //if a string from the PC has been received in its entirety
    inputstring = Serial.readStringUntil(13);  //read the string until we see a <CR>
    Serial1.print(inputstring);                //send that string to the Atlas Scientific product
    Serial1.print('\r');                       //add a <CR> to the end of the string
    inputstring = "";                          //clear the string
  }

  sensorstring.remove(0);                        // Removes all characters
  if (Serial1.available()) {                     //if a string from the Atlas Scientific product has been received in its entirety
    sensorstring = Serial1.readStringUntil(13);  //read the string until we see a <CR>
    if (isdigit(sensorstring[0]) == false) {     //if the first character in the string is a digit
      Serial.println(sensorstring);              //send that string to the PC's serial monitor
    } else                                       //if the first character in the string is NOT a digit
    {
      print_EC_data();  //then call this function
    }
    sensorstring = "";  //clear the string
  }
}

void salinLoopWithoutPC(float tempCForComp) {
  /* APPLY TEMPERATURE COMPENSATION FROM PROVIDED VALUE
     (from any sensor or constant)
     COMMAND SYNTAX FROM ATLAS SCIENTIFIC MANUAL:
     T,n <cr> | n = any value; floating point or int */ // was RT

  // Send temperature compensation + read command
  // Serial1.print("T," + String(tempCForComp, 2) + "\r"); 
  // delay(50);

  sensorstring.remove(0);                        // Removes all characters
  if (Serial1.available()) {                     //if a string from the Atlas Scientific product has been received in its entirety
    sensorstring = Serial1.readStringUntil(13);  //read the string until we see a <CR>
    if (isdigit(sensorstring[0]) == false) {     //if the first character in the string is a digit
      Serial.println(sensorstring);              //send that string to the PC's serial monitor
    } else                                       //if the first character in the string is NOT a digit
    {
      print_EC_data();  //then call this function
    }
    sensorstring = "";  //clear the string
  }
}
