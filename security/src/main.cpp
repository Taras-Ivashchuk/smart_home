#include <Keypad.h>
#include <NanitLib.h>

#define ROWS 4
#define COLS 4
#define BUZ_PIN P4_2
#define RED_PIN P3_2
#define GREEN_PIN P3_3
#define BLUE_PIN P3_4
#define PIR_PIN P5_1

char hexKeys[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'},
};

byte rowPins[ROWS] = {P6_1, P6_2, P6_3, P6_4};
byte colPins[COLS] = {P7_1, P7_2, P7_3, static_cast<byte>(P7_4)};
byte mode = 0;
byte promptShown = 0;

Keypad customKeypad = Keypad(makeKeymap(hexKeys), rowPins, colPins, ROWS, COLS);
String passwd{"1234"}; // dummy password for testing
String passwdInput{};
constexpr char enterPassText[]{"Enter pass:\n"};
constexpr char validPass[]{"Access granted.\n"};
constexpr char invalidPass[]{"Wrong password!\n"};

void outputPrompt(const char *text, uint color = ST7735_WHITE);
void pirHandler();

void setup() {
  Nanit_Base_Start();
  pinMode(BUZ_PIN, OUTPUT);
  Serial.begin(9600);
  attachInterrupt(digitalPinToInterrupt(PIR_PIN), pirHandler, RISING);
}

void loop() {

  if (mode) {
    if (!promptShown) {
      outputPrompt(enterPassText);
      promptShown = 1;
    }
    char key = customKeypad.getKey();
    if (key) {
#if DEBUG
      Serial.println(key);
#endif
      tft.print("*");
      if (key == '*') {
        Serial.println("Input cleared");
        passwdInput = "";
        outputPrompt(enterPassText);
      } else if (key == '#') {
        if (passwdInput == passwd) {
          Serial.println("Access allowed.");
          outputPrompt(validPass, ST7735_GREEN);
          digitalWrite(GREEN_PIN, HIGH);
          tone(BUZ_PIN, 600, 200);
          delay(400);
          tone(BUZ_PIN, 600, 200);
          delay(2000);
          digitalWrite(GREEN_PIN, LOW);
          mode = 0;
          Nanit_Base_Start();
          attachInterrupt(digitalPinToInterrupt(PIR_PIN), pirHandler, RISING);
        } else {
          Serial.println("Access denied.\n");
          digitalWrite(RED_PIN, HIGH);
          outputPrompt(invalidPass, ST7735_RED);
          tone(BUZ_PIN, 300, 500);
          delay(2000);
          digitalWrite(RED_PIN, LOW);
          outputPrompt(enterPassText);
        }
        passwdInput = "";
      } else {
        passwdInput += key;
      }
    }
  }
  delay(100);
}

void outputPrompt(const char *text, uint color) {
  tft.fillScreen(color);
  tft.setCursor(10, 10);
  tft.print(text);
}

void pirHandler() {
  detachInterrupt(digitalPinToInterrupt(PIR_PIN));
  Serial.println("Motion detected");
  mode = 1;
  promptShown = 0;
}