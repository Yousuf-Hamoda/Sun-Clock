/*
 * reciever.ino - IR capture rig
 *
 * Matches the receiver schematic:
 *   IR receiver module OUT on D2
 *   Vs via R3 100R from +5V, with C1 4.7uF from Vs to GND
 *
 * Library (Library Manager): IRremote (v4.x)
 *
 * Upload this, open Serial Monitor at 9600 baud, point your LAMP remote at the
 * receiver, and press a button. It prints the protocol, address, and command,
 * AND a ready-to-paste IrSender line to drop into transmitter.ino.
 */

#include <IRremote.hpp>

#define IR_RECEIVE_PIN 2   // matches schematic: receiver OUT -> D2

void setup() {
  Serial.begin(9600);
  delay(200);
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);
  Serial.println(F("Ready. Point the lamp remote at the receiver and press a button."));
  Serial.println(F("----------------------------------------------------------------"));
}

void loop() {
  if (IrReceiver.decode()) {
    Serial.println();

    IrReceiver.printIRResultShort(&Serial); // protocol, address, command
    IrReceiver.printIRSendUsage(&Serial);   // ready-to-paste IrSender.sendXXX(...) line

    if (IrReceiver.decodedIRData.protocol == UNKNOWN) {
      Serial.println(F("Protocol UNKNOWN - your remote may not use a standard protocol."));
      Serial.println(F("Use the IRremote 'ReceiveDump' example to capture raw timing"));
      Serial.println(F("and send it with IrSender.sendRaw() instead."));
    }

    IrReceiver.resume(); // ready for the next press
  }
}
