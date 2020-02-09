#include <Arduino.h>
#include <LiquidCrystal.h>
#include <Keypad.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, A3, A2, A1, A0);


const byte numRows = 4;
const byte numCols = 4;
char keymap[numRows][numCols] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[numRows] = {9,8,7,6}; //Rows 0 to 3
byte colPins[numCols] = {5,4,3,2}; //Columns 0 to 3

Keypad keypad = Keypad(makeKeymap(keymap), rowPins, colPins, numRows, numCols);

typedef struct Context {
  unsigned long sceneStart;
  bool isBeeping;
} Context;

typedef enum Scene {
  NONE = -1,
  INIT,
  SELECT_MODE,
  COUNTDOWN_BUTTON,
  BOOM,
} Scene;

Scene scene = INIT;


Scene loop_flash(Context* ctx) {
  if (millis() - ctx->sceneStart > 2000) {
    return SELECT_MODE; 
  }
  return NONE;
};




void setup_flash() {
  Serial.begin(9600);
  Serial.println("enter flash scene");


  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("ArduBomb, v0.1");
};


void setup_select_mode() {
  Serial.println("enter select mode");
  lcd.setCursor(0, 0);
  lcd.print("Select mode");
  lcd.setCursor(0, 1);
  lcd.print("A: Btn, B: Code");
}

Scene loop_select_mode(Context* ctx) {
  if (keypad.getKey() == 'A') {
    return COUNTDOWN_BUTTON;
  }
  return NONE;
}

void setup_countdown_button() {
  Serial.println("entering countdown button");
  lcd.setCursor(0, 0);
  lcd.print("push buttons to defuse the bomb");
  //lcd.autoscroll();
}

Scene loop_countdown_button(Context* ctx) {
  lcd.setCursor(0, 1);
  unsigned int timeRemaining = 60 - (millis() - ctx->sceneStart) / 1000;
  bool doBeep = !(timeRemaining % (timeRemaining / 5));
  if (ctx->isBeeping != doBeep) {
    Serial.println(doBeep);
    if (doBeep) {
      tone(10, 50);
    } else {
      noTone(10);
    }
    ctx->isBeeping = doBeep;
    
  }
    
  lcd.print(timeRemaining);
  if (timeRemaining <= 0) {
    return BOOM;
  }
  return NONE;
}

void setup_boom() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Boom, you are dead");
  lcd.setCursor(0, 1);
  lcd.print("Press any Button");
}

Scene loop_boom(Context* ctx) {
  if (keypad.getKey() != NO_KEY) {
    return INIT;
  }
  return NONE;
}

void (*scene_setup[])() = {
  setup_flash,
  setup_select_mode,
  setup_countdown_button,
  setup_boom,
};

Scene (*scene_loop[])(Context*) = {
  loop_flash,
  loop_select_mode,
  loop_countdown_button,
  loop_boom,
};

void loadScene(Scene _scene, Context* ctx) {
  Serial.println("load scene");
  Serial.println(_scene);
  lcd.clear();
  ctx->sceneStart = millis();
  scene = _scene;
  scene_setup[scene]();
}

Context ctx = { };

void setup() {
  loadScene(INIT, &ctx);
}

void loop() {
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  Scene newScene = scene_loop[scene](&ctx);
  if (newScene != NONE) {
    Serial.println("switch");
    Serial.println(newScene);
    loadScene(newScene, &ctx);
  }
  //EasyBuzzer.update();
  /*lcd.setCursor(0, 1);
  // print the number of seconds since reset:
  lcd.print(millis() / 1000);

  digitalWrite(13, millis() / 1000 % 2);

  const char keypressed = myKeypad.getKey();

  lcd.setCursor(0, 0);
  if (keypressed == NO_KEY) {
    lcd.print(" ");
  } else {
    EasyBuzzer.singleBeep(440, 100);
    lcd.print(keypressed);
    Serial.println(keypressed);
  }
  EasyBuzzer.update();*/
}
