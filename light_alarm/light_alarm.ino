#include <Wire.h>
#include <RTClib.h>

RTC_DS1307 rtc;

// Pins
#define sensorPin 2
#define ledPin 3

// Snap light Variables
unsigned long lastClapTime = 0; // Time of the last clap detected
bool clapDetected = false; // Flag to indicate a clap has been detected
int ledStatus=0;

// Alarm Variables
bool alarmOn = false;
const int fadeDuration = 15; // Time in minutes for the LED fade-in
const float TWILIGHT_RATIO = 410.0 / 3.4; // sunrise lux / start-of-civil-twilight lux (~120)

// Set the alarm time
const int alarmHour = 6;
const int alarmMinute = 0;
const int alarmSecond = 0;

// Set the time for automatic shutoff
const int sleepHour = 24;
const int sleepMinute = 0;
const int sleepSecond = 0;

void setup() {

  pinMode(ledPin, OUTPUT);
  pinMode(sensorPin, INPUT);

  analogWrite(ledPin, 255);
  delay(200);
  analogWrite(ledPin, 0);

  // SETUP RTC MODULE
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1);
  }
  
  // automatically sets the RTC to the date & time on PC this sketch was compiled
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  Serial.begin(9600);
}

void loop() {
  int sensorValue = digitalRead(sensorPin);
  DateTime now = rtc.now();
  unsigned long currentTime = millis();

  // Serial.print(now.hour(), DEC);
  // Serial.print(":");
  // Serial.print(now.minute(), DEC);
  // Serial.print(":");
  // Serial.println(now.second(), DEC);

  // Check if it's time for the alarm
  if (now.hour() == alarmHour && now.minute() == alarmMinute && now.second() == alarmSecond) {
    alarmOn= true;
    fadeLight();
  }

  // Stop light if Sleeping
  if (now.hour() == sleepHour && now.minute() == sleepMinute && now.second() == sleepSecond) {
    stopLight();
  }

  // Snapping Function

  if (sensorValue == 0) {
    Serial.println(clapDetected);
    if (!clapDetected) {
      Serial.println("One clap detected!");
      lastClapTime = currentTime;
      clapDetected = true;
      delay(100);
    } else {
      Serial.println("Clap On the Way!");
      // Check if the time difference between the two claps is at least 200ms
      if ((currentTime - lastClapTime) <= 850) {
        Serial.println("Two consecutive claps detected!");
        // Perform actions here, like turning on an LED
        soundDetected();
        lastClapTime = currentTime; // Reset the last clap time
      }

      clapDetected = false; // Reset the clap detected flag if no clap is detected
    }
  }
}

void fadeLight() {
  stopLight(); // Ensure LED is off
  unsigned long startTime = millis();

  // Fade-in the LED over the specified duration
  while (millis() - startTime <= fadeDuration*60000) {
    Serial.println("Led is Fading");
    // Exponential "civil twilight" curve: real daylight rises ~120x
    // (about 3.4 lux -> 410 lux) between sun at -6 deg and sunrise, roughly
    // exponentially in time. LED light output is proportional to PWM duty, so:
    //   duty(t) = 255 * (1/120)^(1 - t/T)   -> starts at ~2/255, ends at 255
    float progress = (millis() - startTime) / (fadeDuration * 60000.0);
    int alarmBrightness = round(255.0 * pow(TWILIGHT_RATIO, progress - 1.0));
    analogWrite(ledPin, constrain(alarmBrightness, 1, 255));
    delay(100); // Adjust this value to control fade smoothness
  }
  
  // Ensure the LED is fully on
  activateLight();
  alarmOn = false;
}


void soundDetected() {
  if (ledStatus == 0 && !alarmOn) {
    activateLight();
  } else if (ledStatus == 1 && !alarmOn) {
    stopLight();
  }
}

void stopLight() {
  analogWrite(ledPin, 0); // Turn off the LED
  ledStatus = 0;
}

void activateLight() {
  analogWrite(ledPin, 255); // Turn off the LED
  ledStatus = 1;
}