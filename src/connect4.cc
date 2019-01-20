#include <Arduino.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include "input_manager.h"

LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display

typedef void (*StateHandler)();
StateHandler g_state = nullptr;

InputManager g_input;

static void HandleInitialState();
static void HandleStartEasyState();
static void HandleStartMediumState();
static void HandleStartHardState();
static void HandleGameStartState();

void setup()
{
  lcd.init();                      // initialize the lcd 
  lcd.init();
  lcd.backlight();

  // Start serial connection
  Serial.begin(9600);
  // Configure pin 12/13 for inputs from momentary
  // push bottons connected to 5V
  g_state = HandleInitialState;
}

// display_controller.h

class DisplayController {
 public:
  DisplayController() : current_(nullptr) {}
  void Show(const char* s);

  char* current_;
};

DisplayController g_display;

void DisplayController::Show(const char* s) {
  int i = 0;
  if (current_ && strcmp(s, current_) == 0)
    return;
  lcd.clear();
  for (int l = 0; l < 2; ++l) {
    int line_len = 0;
    int line_start = i;
    int last_break = -1;
    for (; s[i] && line_len < 16; ++i) {
      if (s[i] == ' ') {
        last_break = i;
      }
      ++line_len;
    }
    if (s[i] == '\0')
      last_break = i;
    else if (last_break < 0)
      last_break = i;
    lcd.setCursor((16 - (last_break - line_start)) / 2, l);
    for (int j = line_start; j < last_break; ++j)
      lcd.write(s[j]);
    i = last_break;
    if (s[i] == ' ')
      ++i;
  }
  if (current_) free(current_);
  current_ = strdup(s);
}

static void HandleInitialState() {
  g_display.Show("Do you want to play?");
  InputEvent e;
  if (!g_input.Get(&e))
    return;
  if (e.IsKeyUp(kYesButtonKey))
    g_state = HandleStartEasyState;
}

static void HandleStartEasyState() {
  g_display.Show("Do you want to go easy?");
  InputEvent e;
  if (!g_input.Get(&e))
    return;
  if (e.IsKeyUp(kYesButtonKey))
    g_state = HandleGameStartState;
  if (e.IsKeyUp(kNoButtonKey))
    g_state = HandleStartMediumState;
}

static void HandleStartMediumState() {
  g_display.Show("Do you want to go medium?");
  InputEvent e;
  if (!g_input.Get(&e))
    return;
  if (e.IsKeyUp(kYesButtonKey))
    g_state = HandleGameStartState;
  if (e.IsKeyUp(kNoButtonKey))
    g_state = HandleStartHardState;
}

static void HandleStartHardState() {
  g_display.Show("Do you want to go hard?");
  InputEvent e;
  if (!g_input.Get(&e))
    return;
  if (e.IsKeyUp(kYesButtonKey))
    g_state = HandleGameStartState;
  if (e.IsKeyUp(kNoButtonKey))
    g_state = HandleInitialState;
}

static void HandleGameStartState() {
  g_display.Show("Let's start. I'll go first!");  
}

void loop()
{
  g_input.Poll();
  g_state();
}
