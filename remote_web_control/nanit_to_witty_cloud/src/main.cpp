#include <NanitLib.h>
#include <SoftwareSerial.h> // data transfer via UART

char bufer_TX;
String byfer_UART;
String Comand;

unsigned long clearCommand = 0;

// PORT STATUS
bool port1 = 0;
bool port3 = 0;

void Comand_Nanit(String Comand);

void setup() {
  Nanit_Base_Start();

  // MOTOR
  pinMode(MOTOR_ENABLE, OUTPUT);
  analogWrite(MOTOR_ENABLE, 140);
  pinMode(MOTOR1_A, OUTPUT);
  pinMode(MOTOR1_B, OUTPUT);
  digitalWrite(MOTOR1_A, 0);
  digitalWrite(MOTOR1_B, 0);

  // RGB
  pinMode(P3_2, OUTPUT); // red
  pinMode(P3_3, OUTPUT); // green
  pinMode(P3_4, OUTPUT); // blue

  Serial.begin(9600);
  Serial3.begin(57600);
}

void loop() {
  if (Serial3.available()) {
    bufer_TX = Serial3.read();

    // Protection against extraneous characters in the buffer
    if ((millis() - clearCommand) > 200) {
      Comand.remove(0);
    }
    clearCommand = millis();

    Serial.println(bufer_TX);
    if (bufer_TX == ' ') {
      Serial.print("command: ");
      Serial.println(Comand);
      Comand_Nanit(Comand);
      Comand.remove(0);
    } else {
      Comand += bufer_TX;
    }
  }

  if (byfer_UART != "") // if there is a need to send data
  {
    Serial3.print(byfer_UART);
    byfer_UART.remove(0);
  }
}

void Comand_Nanit(String value) {
  if (value == "_RXUP") {
    byfer_UART += "_IP? ";
    Serial.println(value += "OK");
  } else if (value == "_SERNOM?") {
    byfer_UART += "0x1S4DF0Q ";
    Serial.println(value += "OK");
  }

  // PORT 1 - DC MOTOR
  if (value == "_PORT1CONNECT") {
    port1 = 1;
    Serial.println(port1);
  } else if (value == "_PORT1DISCONNECT") {
    port1 = 0;
    Serial.println(port1);
  }

  if (port1 == 1 && value == "/p1_3on") {
    Serial.println("motor is spinning");
    digitalWrite(MOTOR1_A, HIGH);
  } else if (port1 == 1 && value == "/p1_3off") {
    digitalWrite(MOTOR1_A, LOW);
  } else if (port1 == 0) {
    digitalWrite(MOTOR1_A, LOW);
  }

  // PORT 3 - RGB
  if (value == "_PORT3CONNECT") {
    port3 = 1;
    Serial.println(port3);
  } else if (value == "_PORT3DISCONNECT") {
    port3 = 0;
    Serial.println(port3);
  }

  if (port3 == 1 && value == "/p3_2on") {
    digitalWrite(P3_2, 1);
  } else if (port3 == 1 && value == "/p3_2off") {
    digitalWrite(P3_2, 0);
  } else if (port3 == 1 && value == "/p3_3on") {
    digitalWrite(P3_3, 1);
  } else if (port3 == 1 && value == "/p3_3off") {
    digitalWrite(P3_3, 0);
  } else if (port3 == 1 && value == "/p3_4on") {
    digitalWrite(P3_4, 1);
  } else if (port3 == 1 && value == "/p3_4off") {
    digitalWrite(P4_4, 0);
  } else if (port3 == 0) {
    digitalWrite(P3_2, 0);
    digitalWrite(P3_3, 0);
    digitalWrite(P3_4, 0);
  }
}