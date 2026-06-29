#include <DHT.h>
#include <Keypad.h>
#include <NanitLib.h>
#include <Servo.h>
#include <TM1637.h>
#include <TinyStepper.h>

#define GAS P1_1

#define STEPPER1 P5_1
#define STEPPER2 P5_2
#define STEPPER3 P5_3
#define STEPPER4 P5_4

#define SERVO P4_2
#define LINE P4_4

#define RLED P3_2
#define GLED P3_3
#define BLED P3_4

#define PIR P9_3
#define BUZZER P9_4

#define KEYPAD_ROWS 4
#define KEYPAD_COLS 4

#define SOUND P6_3
#define PHOTO P6_2
#define TEMP P1_2

#define CLK P2_1
#define DATA P2_2
#define TRIG P2_3
#define ECHO P2_4
#define HALF_STEPS 4096

byte keyboardRows[KEYPAD_ROWS]{P7_1, P7_2, P7_3, static_cast<byte>(P7_4)};
byte keyboardCols[KEYPAD_COLS]{P10_1, static_cast<byte>(P10_2),
                               static_cast<byte>(P10_3), P10_4};
char myKeyMap[KEYPAD_ROWS][KEYPAD_COLS]{{'1', '2', '3', 'A'},
                                        {'4', '5', '6', 'B'},
                                        {'7', '8', '9', 'C'},
                                        {'*', '0', '#', 'D'}};

Keypad myKeyboard(makeKeymap(myKeyMap), keyboardRows, keyboardCols, KEYPAD_ROWS,
                  KEYPAD_COLS);

String passwd{"1233"}; // dummy password for testing
String passwdInput{};
constexpr char enterPassText[]{"Enter pass:\n"};
constexpr char validPass[]{"Access granted.\n"};

Servo myServo;
DHT d11(TEMP, DHT11);
TM1637 tm(CLK, DATA);
TinyStepper stepper(HALF_STEPS, STEPPER1, STEPPER2, STEPPER3, STEPPER4);

// forward declarations
void initWindowTask();
void windowTask();
void initSecurityTask();
void securityTask();
void pirHandler();
void setPirInterval(unsigned long int ms);
void initGarageTask();
void garageTask();
void initFan();
void fanTask();
void initSensorsTask();
void sensorsTask();
void displayTask();
int getDistance();
void ledOn(byte rLed, byte gLed, byte bLed);
void outputText(const char *text, uint color = ST7735_WHITE, int xx = 10,
                int yy = 10);

// forward declarations end

bool isGateOpened = false; // to keep track of distance
byte promptShown =
    0; // to show prompt to user to enter password, don't show it at startup

int prevLine = 1; // line sensor at boot up shouldn't close window

float prevTempVal{}, prevHumVal{};
float tempVal{}, humVal{};
int photoVal, soundVal{}, gasVal{};
int prevGasVal{}, prevPhotoVal{}, prevSoundVal{};
bool isLocked{false};
unsigned long int prevTime{};
bool pirTimerSet{false};

void setup() {
  Nanit_Base_Start();
  tft.fillScreen(ST7735_WHITE);
  Serial.begin(9600);
  initSensorsTask();
  initWindowTask();
  initSecurityTask();
  initGarageTask();
  initFan();
  prevTime = millis();
}

void loop() {
  if (!isLocked) {
    setPirInterval(1 * 60000);
    sensorsTask();
    windowTask();
    garageTask();
    fanTask();
    displayTask();
  } else {
    securityTask();
  }

  delay(100);
}

void initWindowTask() {
  pinMode(LINE, INPUT);
  myServo.attach(SERVO);
  myServo.write(0);
}

void ledOn(byte rLed, byte gLed, byte bLed) {
  digitalWrite(RLED, rLed);
  digitalWrite(GLED, gLed);
  digitalWrite(BLED, bLed);
}

int getDistance() {
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  int cm = pulseIn(ECHO, HIGH) / 58;
  return cm;
}

void windowTask() {
  const byte line = digitalRead(LINE);
  Serial.print("Line: ");
  Serial.println(line);
  if (prevLine != line) {
    Serial.print("line: ");
    Serial.println(line);
    if (line == 1) {
      myServo.write(0);
    } else if (line == 0) {
      myServo.write(90);
    }
    prevLine = line;
    delay(1500); // give time for motor to move
  }
}

void initSecurityTask() {
  pinMode(RLED, OUTPUT);
  pinMode(GLED, OUTPUT);
  pinMode(BLED, OUTPUT);
  ledOn(0, 1, 0);
  pinMode(PIR, INPUT);
}

void securityTask() {

  if (!promptShown) {
    tft.fillScreen(ST7735_WHITE);

    tone(BUZZER, 700, 1000);

    ledOn(0, 0, 1);
    outputText(enterPassText);
    promptShown = 1;
  }
  char key = myKeyboard.getKey();
  if (key) {
    Serial.println(key);
    tft.print("*");
    if (key == '*') {
      // Serial.println("Input cleared");
      passwdInput = "";
      tft.fillScreen(ST7735_WHITE);
      outputText(enterPassText);
    } else if (key == '#') {
      if (passwdInput == passwd) {
        // Serial.println("Access allowed.");
        outputText(validPass, ST7735_GREEN);
        ledOn(0, 1, 0);
        tone(BUZZER, 600, 200);
        delay(400);
        tone(BUZZER, 600, 200);
        promptShown = 0;
        tft.fillScreen(ST7735_WHITE);

        // refresh indicators
        prevGasVal = 0;
        prevHumVal = 0;
        prevTempVal = 0;
        prevPhotoVal = 0;
        prevSoundVal = 0;

        isLocked = false;
        prevTime = millis();
        pirTimerSet = false;
      } else {
        // Serial.println("Access denied.\n");
        tone(BUZZER, 300, 500);
        tft.fillScreen(ST7735_WHITE);
        outputText(enterPassText);
      }
      passwdInput = "";
    } else {
      passwdInput += key;
    }
  }
}

void outputText(const char *text, uint color, int xx, int yy) {
  tft.fillRect(xx, yy, tft.width(), 15, color);
  tft.setCursor(xx, yy);
  tft.print(text);
}

void initGarageTask() {
  pinMode(CLK, OUTPUT);
  pinMode(DATA, OUTPUT);
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  stepper.Enable();
  // stepper.Move(15); // initial position of the barrier
  tm.begin();
  tm.setBrightness(4);
  tm.display("stop");

  digitalWrite(TRIG, 0);
}

void garageTask() {
  int cm{getDistance()};

  if (cm < 5 && !isGateOpened) {
    tm.clearScreen();
    delay(10);
    tm.display("go");
    stepper.Move(90);
    isGateOpened = !isGateOpened;
    delay(1500);
  } else if (cm > 5 && isGateOpened) {
    tm.clearScreen();
    delay(10);
    tm.display("stop");
    stepper.Move(-90);
    isGateOpened = !isGateOpened;
    delay(1500);
  }
}

void initFan() {
  pinMode(GAS, INPUT);
  pinMode(MOTOR_ENABLE, OUTPUT);
  analogWrite(MOTOR_ENABLE, 140);
  pinMode(MOTOR1_A, OUTPUT);
  pinMode(MOTOR1_B, OUTPUT);
  digitalWrite(MOTOR1_A, 0);
  digitalWrite(MOTOR1_B, 0);
}

void fanTask() {
  if (tempVal > 35) {
    digitalWrite(MOTOR1_A, HIGH);
  } else {
    digitalWrite(MOTOR1_A, LOW);
  }
}

void initSensorsTask() {
  pinMode(SOUND, INPUT);
  pinMode(PHOTO, INPUT);
  pinMode(TEMP, INPUT);
  d11.begin();
}

void sensorsTask() {
  tempVal = d11.readTemperature();
  humVal = d11.readHumidity();
  gasVal = analogRead(GAS);
  soundVal = analogRead(SOUND);
  photoVal = analogRead(PHOTO);
}

void displayTask() {
  if (prevTempVal != tempVal) {
    String tempText = "Temp: " + String(tempVal, 2);
    outputText(tempText.c_str(), ST77XX_WHITE, 10, 10);
    prevTempVal = tempVal;
  }
  if (prevHumVal != humVal) {
    String humText = "Hum: " + String(humVal, 2);
    outputText(humText.c_str(), ST77XX_WHITE, 10, 30);
    prevHumVal = humVal;
  }
  if (prevGasVal != gasVal) {
    String gasText = "CO: " + String(gasVal);
    outputText(gasText.c_str(), ST77XX_WHITE, 10, 50);
    prevGasVal = gasVal;
  }

  if (prevSoundVal != soundVal) {
    String soundText = "Sound: " + String(soundVal);
    outputText(soundText.c_str(), ST77XX_WHITE, 10, 70);
    prevSoundVal = soundVal;
  }

  if (prevPhotoVal != photoVal) {
    String photoText = "Ph: " + String(photoVal);
    outputText(photoText.c_str(), ST77XX_WHITE, 10, 90);
    prevPhotoVal = photoVal;
  }
}

void pirHandler() {
  detachInterrupt(digitalPinToInterrupt(PIR));
  isLocked = true;
}

void setPirInterval(unsigned long int ms) {
  if ((millis() - prevTime > ms) && !pirTimerSet) {
    attachInterrupt(digitalPinToInterrupt(PIR), pirHandler, RISING);
    pirTimerSet = true;
  }
}