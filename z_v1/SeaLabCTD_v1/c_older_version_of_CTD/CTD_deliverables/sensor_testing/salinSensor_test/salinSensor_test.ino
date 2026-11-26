// EZO-EC conductivity circuit test for SeaLabCTD v1 (Arduino UNO).
// Type EZO-EC commands in the Serial Monitor (9600 baud, line ending: carriage return)
// and the circuit's replies come back. A reading is printed as EC,TDS,SAL,SG.

// TDS - Total Dissolved Solids
// 12,940 for 12880 23.0C      12, 396 is actual thing to input
// 80,000 for 61601 at 22.4C   76,048 as actual thing


#include <SoftwareSerial.h>

const uint8_t EZO_RX_PIN = 2;   // UNO pin wired to the circuit's TX
const uint8_t EZO_TX_PIN = 3;   // UNO pin wired to the circuit's RX
SoftwareSerial ezoSerial(EZO_RX_PIN, EZO_TX_PIN);

String ezoReply;                // the reply being received, up to its <CR>

void setup() {
  Serial.begin(9600);
  ezoSerial.begin(9600);        // the EZO-EC's default UART speed
  ezoReply.reserve(30);
}

void loop() {
  if (Serial.available()) {                       // a command from the Serial Monitor: pass it on, ending in <CR>
    String cmd = Serial.readStringUntil('\r');
    ezoSerial.print(cmd);
    ezoSerial.print('\r');
  }

  while (ezoSerial.available()) {                 // collect the reply one character at a time
    char c = ezoSerial.read();
    if (c != '\r') {
      ezoReply += c;
      continue;
    }
    if (isdigit(ezoReply[0])) printReading();     // a reading starts with a digit
    else Serial.println(ezoReply);                // anything else (*OK, *ER, ...) is shown as it came
    ezoReply = "";
  }
}

// A reading's fields are comma-separated: EC, TDS, salinity, specific gravity.
// Print them the same way, one reading per line.
void printReading() {
  char buf[30];
  ezoReply.toCharArray(buf, sizeof buf);
  const char *sep = "";
  for (char *field = strtok(buf, ","); field != NULL; field = strtok(NULL, ",")) {
    Serial.print(sep);
    Serial.print(field);
    sep = ",";
  }
  Serial.println();
}
