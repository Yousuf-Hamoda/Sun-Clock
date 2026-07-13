/*
 * transmitter.ino - Sun Clock (RTC-scheduled IR lamp trigger)
 *
 * Matches the transmitter schematic:
 *   DS1307 RTC on I2C  (A4 = SDA, A5 = SCL, 32.768 kHz crystal on X1/X2)
 *   IR LED driver on D3 (D3 -> R1 1k -> Q1 base; IR LED via R2 100R from +5V)
 *
 * Libraries (Library Manager): RTClib (Adafruit), IRremote (v4.x)
 *
 * Before it works: capture your lamp remote's ON (and optional OFF) codes with
 * reciever.ino, then paste them into lampOn() / lampOff() below.
 */

#include <Wire.h>
#include <RTClib.h>
#include <IRremote.hpp>

// ---- Config ----
const uint8_t IR_SEND_PIN = 3;      // matches schematic: D3 -> R1 -> Q1 base

const int ALARM_HOUR   = 6;         // turn lamp ON at 06:00
const int ALARM_MINUTE = 0;

const bool USE_OFF_TIME = true;     // set false to skip the auto-off
const int OFF_HOUR      = 7;        // turn lamp OFF at 07:00
const int OFF_MINUTE    = 0;

RTC_DS1307 rtc;
int lastOnDay  = -1;                // fire each event only once per day
int lastOffDay = -1;

void setup() {
  Serial.begin(9600);
  Wire.begin();

  if (!rtc.begin()) {
    Serial.println(F("RTC not found - check SDA=A4 / SCL=A5, VCC, GND, and the crystal."));
    while (1) { delay(10); }
  }

  // Set the clock ONCE: uncomment, upload, then re-comment and upload again.
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  IrSender.begin(IR_SEND_PIN);
  Serial.println(F("Sun clock ready."));
}

void loop() {
  DateTime now = rtc.now();

  if (now.hour() == ALARM_HOUR && now.minute() == ALARM_MINUTE && now.day() != lastOnDay) {
    lampOn();
    lastOnDay = now.day();
    Serial.println(F("Alarm: lamp ON"));
  }

  if (USE_OFF_TIME &&
      now.hour() == OFF_HOUR && now.minute() == OFF_MINUTE && now.day() != lastOffDay) {
    lampOff();
    lastOffDay = now.day();
    Serial.println(F("Auto-off: lamp OFF"));
  }

  delay(1000); // check once per second
}

void lampOn() {
  // TODO: replace with YOUR lamp remote's ON code (capture it with reciever.ino).
  // Example NEC: address 0x00, command 0x45
  IrSender.sendNEC(0x00, 0x45, 0);
}

void lampOff() {
  // TODO: replace with YOUR lamp remote's OFF code.
  // Example NEC: address 0x00, command 0x46
  IrSender.sendNEC(0x00, 0x46, 0);
}
