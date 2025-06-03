#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display1(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Adafruit_SSD1306 display2(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);


const int button1Pin = 2;
const int button2Pin = 3;
const int startPausePin = 4;
const int addMinutePin = 5;
const int potPin = A0;

bool player1Turn = true;
bool running = false;
bool configuring = true;
bool isPaused = true;

unsigned long lastMillis;
unsigned long player1Time = 0;
unsigned long player2Time = 0;
int configMinutes = 5;

bool lastStartPauseState = HIGH;
bool lastAddMinState = HIGH;

void setup() {
  Serial.begin(9600);

  pinMode(button1Pin, INPUT_PULLUP);
  pinMode(button2Pin, INPUT_PULLUP);
  pinMode(startPausePin, INPUT_PULLUP);
  pinMode(addMinutePin, INPUT_PULLUP);
  
  if (!display1.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED 1 falhou"));
    while (true);
  }
  if (!display2.begin(SSD1306_SWITCHCAPVCC, 0x3D)) {
    Serial.println(F("OLED 2 falhou"));
    while (true);
  }

  display1.clearDisplay();
  display2.clearDisplay();
  display1.setTextSize(2);
  display2.setTextSize(2);
  display1.setTextColor(SSD1306_WHITE);
  display2.setTextColor(SSD1306_WHITE);
  updateDisplays();
}

void loop() {
  if (configuring) {
    int potValue = analogRead(potPin);
    if (potValue < 341) configMinutes = 5;
    else if (potValue < 682) configMinutes = 10;
    else configMinutes = 30;

    display1.clearDisplay();
    display1.setCursor(0, 10);
    display1.print("Tempo:");
    display1.setCursor(0, 30);
    display1.print(configMinutes);
    display1.print(" min");
    display1.display();

    display2.clearDisplay();
    display2.setCursor(0, 10);
    display2.print("Aperte Start");
    display2.display();
  }

  bool currentStartPause = digitalRead(startPausePin);
  if (currentStartPause == LOW && lastStartPauseState == HIGH) {
    if (configuring) {
      player1Time = configMinutes * 60 * 1000UL;
      player2Time = player1Time;
      configuring = false;
      running = true;
      isPaused = false;
      lastMillis = millis();
    } else {
      isPaused = !isPaused;
      lastMillis = millis();
    }
    delay(300);
  }
  lastStartPauseState = currentStartPause;

  bool currentAddMin = digitalRead(addMinutePin);
  if (currentAddMin == LOW && lastAddMinState == HIGH && running && !isPaused) {
    if (player1Turn) player1Time += 60000UL;
    else player2Time += 60000UL;
    delay(300);
  }
  lastAddMinState = currentAddMin;

  if (digitalRead(button1Pin) == LOW && player1Turn && running && !isPaused) {
    player1Turn = false;
    lastMillis = millis();
    delay(300);
  }

  if (digitalRead(button2Pin) == LOW && !player1Turn && running && !isPaused) {
    player1Turn = true;
    lastMillis = millis();
    delay(300);
  }

  if (running && !isPaused) {
    unsigned long now = millis();
    unsigned long elapsed = now - lastMillis;

    if (player1Turn) {
      player1Time = (elapsed < player1Time) ? player1Time - elapsed : 0;
    } else {
      player2Time = (elapsed < player2Time) ? player2Time - elapsed : 0;
    }

    lastMillis = now;
    updateDisplays();

    if (player1Time == 0 || player2Time == 0) {
      running = false;
    }
  }
}

void updateDisplays() {
  display1.clearDisplay();
  display1.setCursor(0, 10);
  display1.print("P1 ");
  printTime(display1, player1Time);
  if (player1Turn) display1.print(" *");
  display1.display();

  display2.clearDisplay();
  display2.setCursor(0, 10);
  display2.print("P2 ");
  printTime(display2, player2Time);
  if (!player1Turn) display2.print(" *");
  display2.display();
}

void printTime(Adafruit_SSD1306 &disp, unsigned long ms) {
  unsigned long seconds = ms / 1000;
  unsigned long minutes = seconds / 60;
  seconds = seconds % 60;

  if (minutes < 10) disp.print("0");
  disp.print(minutes);
  disp.print(":");
  if (seconds < 10) disp.print("0");
  disp.print(seconds);
}
