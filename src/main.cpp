#include <Arduino.h>
#include <LiquidCrystal.h>
#include <Keypad.h>
#include <Wire.h>
#include <OneButton.h>

#include "debug.h"
#include "scene.h"

// Configuration

const uint32_t flashTime = 500;
const uint32_t countdownTime = 120000;
const uint16_t beepToneFrequency = 440 * 12;
const uint8_t beeperPin = 12;
const uint8_t blinkLedPin = 11;
const uint8_t lcdRows = 2;
const uint8_t lcdColumns = 16;

const byte _numRows = 4;
const byte _numCols = 4;
char _keymap[_numRows][_numCols] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
// Johnny uno
//const byte const rowPins[numRows] = {9,8,7,6}; //Rows 0 to 3
//const byte const colPins[numCols] = {5,4,3,2}; //Columns 0 to 3
// Takeru
byte _rowPins[_numRows] = {36, 34, 32, 30}; //Rows 0 to 3
byte _colPins[_numCols] = {28, 26, 24, 22}; //Columns 0 to 3

// Global Variables. Urgs.

char lineBuffer[lcdColumns + 1] = "\0";
uint32_t sceneStart;
bool isBeeping;
//LiquidCrystal lcd(12, 11, A3, A2, A1, A0); // Johnnys Uno
LiquidCrystal lcd(2, 3, 4, 5, 6, 7, 8); // Takeru Dev board
Keypad keypad(makeKeymap(_keymap), _rowPins, _colPins, _numRows, _numCols);

OneButton tilt = OneButton(10, true);

Scene scene = NONE;

Scene loop_flash() {
  if (millis() - sceneStart > flashTime) {
    return SELECT_MODE; 
  }
  return NONE;
};

void setup_flash() {
#ifdef DEBUG
  Serial.begin(9600);
#endif
  MSG("enter flash scene");

  // set up the LCD's number of columns and rows:
  lcd.begin(lcdColumns, lcdRows);
  lcd.print("ArduBomb, v0.1");
};

void setup_select_mode() {
  MSG("enter select mode");

  lcd.setCursor(0, 0);
  lcd.print("Select mode");
  lcd.setCursor(0, 1);
  lcd.print("A: Btn, B: Code");
}

Scene loop_select_mode() {
  if (keypad.getKey() == 'A') {
    return COUNTDOWN_BUTTON;
  }
  return NONE;
}

void setup_countdown_button() {
  MSG("entering countdown button");

  lcd.setCursor(0, 0);
  lcd.print("Push both buttons...");
  tilt.setPressTicks(2000);
}

Scene loop_countdown_button() {
  const int32_t timeRemaining = countdownTime - (millis() - sceneStart);
  const uint32_t frequency = (timeRemaining / 1000 / 12 + 1) * 200;
  const float dutyCycle = 0.9;
  const bool doBeep = (timeRemaining % frequency) / (float)frequency > dutyCycle;

  tilt.tick();
  MSG(tilt.isLongPressed());
  

  if (isBeeping != doBeep) {
    if (doBeep) {
      digitalWrite(blinkLedPin, 1);
      tone(beeperPin, beepToneFrequency);
    } else {
      digitalWrite(blinkLedPin, 0);
      noTone(beeperPin);
    }
    isBeeping = doBeep;
  }

  sprintf(lineBuffer, "%03ld", timeRemaining / 1000 + 1);
  lcd.setCursor(0, 1);
  lcd.print(lineBuffer);

  if (timeRemaining <= 0) {
    return BOOM;
  }
  return NONE;
}

void setup_boom() {
  lcd.setCursor(0, 0);
  lcd.print("Boom, you are dead");
  lcd.setCursor(0, 1);
  lcd.print("Press any Button");
}

Scene loop_boom() {
  if (keypad.getKey() != NO_KEY) {
    return FLASH;
  }
  return NONE;
}

void (*scene_setup[])() = {
  setup_flash,
  setup_select_mode,
  setup_countdown_button,
  setup_boom,
};

Scene (*scene_loop[])() = {
  loop_flash,
  loop_select_mode,
  loop_countdown_button,
  loop_boom,
};

void loadScene(Scene _scene) {
  MSG("load scene %d");
  MSG(_scene);
  
  lcd.clear();
  sceneStart = millis();
  scene = _scene;
  scene_setup[scene]();
}

void setup() {
  loadScene(FLASH);
}

void loop() {
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  Scene newScene = scene_loop[scene]();
  if (newScene != NONE) {
    loadScene(newScene);
  }
}