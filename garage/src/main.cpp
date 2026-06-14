#include "Arduino.h"
#include <NanitLib.h>
#include <TM1637.h>
#include <TinyStepper.h>

byte in_1 = P4_1;
byte in_2 = P4_2;
byte in_3 = P4_3;
byte in_4 = P4_4;

byte echPin = P5_2;
byte trigPin = P5_1;

byte clkPin = P5_3;
byte dataPin = P5_4;

byte ledGreen = P3_2;
byte ledYellow = P3_3;
byte ledRed = P3_4;

int isOpen = 0;

int halfsteps = 4096;

TinyStepper stepper(halfsteps, in_1, in_2, in_3, in_4);
TM1637 tm(clkPin, dataPin);

int getDistance();
void ledOn(int gLed, int yLed, int rLed);
void moveStepMotor(int angle);

void setup() {
  Nanit_Base_Start();
  pinMode(echPin, INPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(clkPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(ledGreen, OUTPUT);
  pinMode(ledYellow, OUTPUT);
  pinMode(ledRed, OUTPUT);

  stepper.Enable();
  Serial.begin(9600);
  stepper.Move(0);
  tm.begin();
  tm.setBrightness(4);
}

void loop() {
  int cm = getDistance();
#if DEBUG
  Serial.print("cm, is_open: ");
  Serial.print(cm);
  Serial.print(" ");
  Serial.println(isOpen);
#endif
  if (cm < 5 && !isOpen) {
    ledOn(0, 1, 0);
    moveStepMotor(90);
    tm.display("OPEN");
    ledOn(1, 0, 0);
    isOpen = !isOpen;

  } else if (cm > 5 && isOpen) {
    ledOn(0, 1, 0);
    moveStepMotor(-90);
    tm.display("CLSD");
    ledOn(0, 0, 1);
    isOpen = !isOpen;
  }
  delay(100);
}

void moveStepMotor(int angle) { stepper.Move(angle); }

int getDistance() {
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  int cm = pulseIn(echPin, HIGH) / 58;
  return cm;
}

void ledOn(int gLed, int yLed, int rLed) {
  digitalWrite(ledGreen, gLed);
  digitalWrite(ledYellow, yLed);
  digitalWrite(ledRed, rLed);
}
