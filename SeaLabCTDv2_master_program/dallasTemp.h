#pragma once
// /***************************************************************/
//  Connection:
//  1.Plugable Terminal Sensor Adapter & Waterproof DS18B20 Digital Temperature Sensor
//                  A   ---->     YELLOW(DATA SIGNAL)
//                  B   ---->     RED   (VIN)
//                  C   ---->     BLACK (GND)

//  2.Waterproof DS18B20 Digital Temperature Sensor & Arduino board
//               1(A)   ---->     Digital Pin12
//               2(B)   ---->     5V/3.3V
//               3(C)   ---->     GND

//      When connect DS18B20 with the adapter,please choose to use the
//      Pull-up Register Jumper
//  ***************************************************************/

// Dallas temp, the slow one
#include <OneWire.h>
// int DS18S20_Pin = 12; //DS18S20 Signal pin on digital 12

extern float dallasTemp;

OneWire ds(12);  // on digital pin 12

float getDallasTemp(){
  //returns the temperature from one DS18S20 in DEG Celsius

  byte data[12];
  byte addr[8];

  if ( !ds.search(addr)) {
      //no more sensors on chain, reset search
      ds.reset_search();
      return -1000;
  }

  if ( OneWire::crc8( addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return -1000;
  }

  if ( addr[0] != 0x10 && addr[0] != 0x28) {
      Serial.print("Device is not recognized");
      return -1000;
  }

  ds.reset();
  ds.select(addr);
  ds.write(0x44,1); // start conversion, with parasite power on at the end

  byte present = ds.reset(); // ignore this error
  ds.select(addr);
  ds.write(0xBE); // Read Scratchpad

  for (int i = 0; i < 9; i++) { // we need 9 bytes
    data[i] = ds.read();
  }

  ds.reset_search();

  byte MSB = data[1];
  byte LSB = data[0];

  float tempRead = ((MSB << 8) | LSB); //using two's compliment
  dallasTemp = tempRead / 16.0;
  
  return dallasTemp;
  // delay(10);
}