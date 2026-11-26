// SPDX-FileCopyrightText: 2025-2026 Aspect Wave LLC
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
// /***************************************************************/
// Atlas Scientific EZO-EC conductivity circuit on Serial1 (UART) for SeaLabCTDv2.
// - salinSetup(): starts Serial1 and the circuit's enable pin (off at start)
// - parseEzoReply(): splits a reading "EC,TDS,SAL,SG" into ec, tds, sal and grav
// - salinLoopWithPC(): passes commands typed in the Serial Monitor to the circuit,
//   and its replies back
// - salinLoopWithoutPCAndTempComp() / salinLoopWithoutPCNoTempComp(): one reading
//   on request, with or without temperature compensation
// /***************************************************************/

constexpr size_t EZO_REPLY_MAX = 50;   // EC alone can run to 8+ characters at high conductivity

String pcLine;    // a command typed in the Serial Monitor, on its way to the circuit
String ezoLine;   // one reply line from the circuit

extern float brFastTemp;
extern float dallasTemp;
extern bool ecBool;
extern bool sBool;

inline void salinSetup() {
  Serial1.begin(SALINITY_BAUD);            // the EZO-EC's UART (9600 baud by default)
  pcLine.reserve(10);
  ezoLine.reserve(EZO_REPLY_MAX);
  pinMode(SALINITY_ENABLE_PIN, OUTPUT);    // power-enable for the EZO-EC board
  digitalWrite(SALINITY_ENABLE_PIN, LOW);  // off at system start
}

// A reading is comma-separated, in the order the circuit's output is set up:
// EC, TDS, salinity, specific gravity. A missing field reads 999.
inline void parseEzoReply() {
  char buf[EZO_REPLY_MAX];
  ezoLine.toCharArray(buf, sizeof buf);
  float *dest[] = { &ec, &tds, &sal, &grav };
  char *field = strtok(buf, ",");
  for (float *d : dest) {
    *d = (field != NULL) ? atof(field) : 999.0;
    field = strtok(NULL, ",");
  }
}

// Every reply is echoed to USB serial. A reading starts with a digit and is also parsed;
// status replies (*OK, *ER and the like) are only echoed.
inline void handleEzoReply() {
  Serial.println(ezoLine);
  if (isdigit(ezoLine[0])) parseEzoReply();
}

void salinLoopWithPC() {
  if (Serial.available()) {                    // a command from the Serial Monitor: pass it on, ending in <CR>
    pcLine = Serial.readStringUntil('\r');
    Serial1.print(pcLine);
    Serial1.print('\r');
    pcLine = "";
  }

  ezoLine = "";
  if (Serial1.available()) {                   // a reply from the circuit, up to its <CR>
    ezoLine = Serial1.readStringUntil('\r');
    handleEzoReply();
    ezoLine = "";
  }
}

void debugSerial1Raw() {
    while (Serial1.available()) {
        char c = Serial1.read();                // read one byte
        Serial.print(c);                        // print as character
        Serial.print(" [0x");
        Serial.print(c, HEX);                   // print hex
        Serial.print(" / ");
        Serial.print((int)c);                   // print decimal ASCII
        Serial.println("]");
    }
}

// One reading on request: clear anything left in the UART, send the command, wait up to
// 1.5 s for the reply (about 600 ms) and handle it.
inline void ezoReadOnce(const String &cmd) {
  while (Serial1.available()) Serial1.read();          // flush boot/leftover chatter
  Serial1.print(cmd);

  unsigned long t0 = millis();
  while (!Serial1.available() && (millis() - t0) < 1500) { yield(); }

  ezoLine = "";
  if (Serial1.available()) {
    ezoLine = Serial1.readStringUntil('\r');
    handleEzoReply();
    ezoLine = "";
  }
}

void salinLoopWithoutPCAndTempComp(float tempCForComp) {
  ezoReadOnce("RT," + String(tempCForComp, 2) + "\r");  // read WITH temperature compensation
}

void salinLoopWithoutPCNoTempComp() {
  ezoReadOnce("R\r");                                    // read WITHOUT setting temperature compensation
}
