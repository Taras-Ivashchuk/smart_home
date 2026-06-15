#include <NanitLib.h>

#define PHOTORES_PIN P6_3
#define RELAY_PIN P6_1

void setup() {
  Nanit_Base_Start();
  pinMode(PHOTORES_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  Serial.begin(9600);
  tft.fillScreen(ST77XX_WHITE);
}

void loop() {
  // clear the display
  tft.fillRect(70, 10, 70, 15, ST77XX_WHITE);
  int phVal = analogRead(PHOTORES_PIN);
#if DEBUG
  Serial.print("photo res: ");
  Serial.println(phVal);
#endif
  tft.setCursor(10, 10);
  tft.print("PhVal: ");
  tft.println(phVal);
  // set display brightness inverted to photoresistor value
  int brVal = map(phVal, 0, 1023, 0, 255);
  analogWrite(TFT_BL, brVal);
  if (phVal > 500)
    digitalWrite(RELAY_PIN, HIGH);
  else
    digitalWrite(RELAY_PIN, LOW);
  delay(500);
}
