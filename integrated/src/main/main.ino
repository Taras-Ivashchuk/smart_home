#include <Keypad.h>
#include <NanitLib.h>
#include <Servo.h>
#include <DHT.h>
#include <TM1637.h>

#define PORT4_SERVO P4_3
#define PORT4_LINE P4_2

#define PORT3_RLED P3_2
#define PORT3_GLED P3_3
#define PORT3_BLED P3_4

#define PORT5_PIR P5_1
#define PORT5_BUZZER P5_2
#define PORT5_SOUND P5_3

#define KEYPAD_ROWS 4
#define KEYPAD_COLS 4

#define PORT6_TEMP P6_1
#define PORT6_GAS P6_2
#define PORT6_PHOTO P6_3

#define PORT9_CLK P9_1
#define PORT9_DATA P9_2
#define PORT9_TRIG P9_3
#define PORT9_ECHO P9_4

byte keyboardRows[KEYPAD_ROWS]{P7_1, P7_2, P7_3, static_cast<byte>(P7_4)};
byte keyboardCols[KEYPAD_COLS]{P10_1, static_cast<byte>(P10_2),
                               static_cast<byte>(P10_3), P10_4};
char myKeyMap[KEYPAD_ROWS][KEYPAD_COLS]{{'1', '2', '3', 'A'},
                                        {'4', '5', '6', 'B'},
                                        {'7', '8', '9', 'C'},
                                        {'*', '0', '#', 'D'}};

Keypad myKeyboard (makeKeymap(myKeyMap), keyboardRows, keyboardCols,
                           KEYPAD_ROWS, KEYPAD_COLS);
Servo myServo;
DHT d11(PORT6_TEMP, DHT11);
TM1637 tm(PORT9_CLK, PORT9_DATA);

// forward declarations
void initP3();
void initP4();
void initP5();
void initP6();
void initP9();
void port5Display();
void port6Display();
void port9Display();
void port4Display();
int getDistance();
void ledOn(byte rLed, byte gLed, byte bLed);


int prevCm; // to keep track of distance
int curtainsClosed = 0;


void setup() {
  Nanit_Base_Start();
  Serial.begin(9600);
  // initP3();
  initP4();
  // initP5();
  // initP6();
  // initP9();
  // ledOn(0, 1, 0);
}

void loop() {
  port4Display();
  delay(100);
}

void initP3() {
  pinMode(PORT3_RLED, OUTPUT);  
  pinMode(PORT3_GLED, OUTPUT);
  pinMode(PORT3_BLED, OUTPUT);    
}

void initP4() {
  pinMode(PORT4_LINE, INPUT);
  myServo.attach(PORT4_SERVO);
  myServo.write(0);
}

void initP5() {
  pinMode(PORT5_PIR, INPUT);
  pinMode(PORT5_BUZZER, OUTPUT);
  pinMode(PORT5_SOUND, INPUT);
}

void initP6() {
  pinMode(PORT6_GAS, INPUT);
  pinMode(PORT6_PHOTO, INPUT);
  d11.begin();
}

void initP9() {
  tm.begin();
  tm.setBrightness(4);
  pinMode(PORT9_TRIG, OUTPUT);
  pinMode(PORT9_ECHO, INPUT);
  digitalWrite(PORT9_TRIG, 0);
}

void ledOn(byte rLed, byte gLed, byte bLed) {
  digitalWrite(PORT3_RLED, rLed);
  digitalWrite(PORT3_GLED, gLed);
  digitalWrite(PORT3_BLED, bLed);
}

void port5Display() {
  const byte pir = digitalRead(PORT5_PIR);
  Serial.print("pir: ");
  Serial.println(pir);

  tone(PORT5_BUZZER, 800);

  const byte sound = analogRead(PORT5_SOUND);
  Serial.print("sound: ");
  Serial.println(sound);

}

void port6Display() {
  const int temp = d11.readTemperature();
  Serial.print("temp: ");
  Serial.println(temp);

  const byte gas = analogRead(PORT6_GAS);
  Serial.print("gas: ");
  Serial.println(gas);

  const byte light = analogRead(PORT6_PHOTO);
  Serial.print("light: ");
  Serial.println(light);
}

void port9Display() {
  const int cm {getDistance()};
  if (cm != prevCm) {
    tm.clearScreen();
    delay(10);
    tm.display(cm);
    prevCm = cm;
  } 
}

int getDistance() {
  digitalWrite(PORT9_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PORT9_TRIG, LOW);

  int cm = pulseIn(PORT9_ECHO, HIGH) / 58;
  return cm;
}

void port4Display() {
  const byte line = digitalRead(PORT4_LINE);
  Serial.print("line: ");
  Serial.println(line);
  if (line == 1) {
    myServo.write(0);
    curtainsClosed = 0;
  } else if (line == 0){
    myServo.write(90);
    curtainsClosed = 1;
  }
  delay(200);
}
