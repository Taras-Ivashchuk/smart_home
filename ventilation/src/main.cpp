#include "Arduino.h"
#include <DHT.h>
#include <NanitLib.h>

#define PIN_GAS P1_1
#define PIN_TMP P5_2
bool dc_flag = false;

DHT d11(PIN_TMP, DHT11);

void setup() {
  Nanit_Base_Start();
  pinMode(MOTOR_ENABLE, OUTPUT);
  digitalWrite(MOTOR_ENABLE, HIGH);
  pinMode(MOTOR1_A, OUTPUT);
  pinMode(MOTOR1_B, HIGH);
  pinMode(PIN_GAS, INPUT);
  pinMode(PIN_TMP, INPUT);
  d11.begin();
  Serial.begin(9600);
}

void loop() {
  int gasVal = analogRead(PIN_GAS);
  int tmpVal = d11.readTemperature();
  if (isnan(tmpVal) || isnan(gasVal)) {
    Serial.println("DHT Error");
  } else {
#if DEBUG
    Serial.print("GAS: ");
    Serial.println(gasVal);
    Serial.print("TMP: ");
    Serial.println(tmpVal);
#endif
    if (gasVal > 260 || tmpVal > 40) {
      digitalWrite(MOTOR1_A, HIGH);
      digitalWrite(MOTOR1_B, LOW);
    } else {
      digitalWrite(MOTOR1_A, LOW);
      digitalWrite(MOTOR1_B, LOW);
    }
  }
  delay(200);
}