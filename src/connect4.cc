#include <Arduino.h>
#include <Adafruit_MotorShield.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

#include "display_controller.h"
#include "dropper_controller.h"
#include "input_manager.h"

LiquidCrystal_I2C g_lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display
const int kShowMessageTimeoutMs = 5000;

typedef void (*StateHandler)();
StateHandler g_state = nullptr;

InputManager g_input;
DropperController g_dropper;

static void HandleInitialState();
static void HandleStartEasyState();
static void HandleStartMediumState();
static void HandleStartHardState();
static void HandleGameStartState();
static void HandleAskDiagnosticsState();
static void HandleDiagnosticsPrecondState();
static void HandleDiagnosticsHomingForwardState();
static void HandleDiagnosticsHomingBackwardState();
static void HandleWaitForDiscDrop();
static void HandleDiagnosticsMovingNextColumnState();
static void HandleDiagnosticErrorTimeoutState();

DisplayController g_display;

void setup()
{
  // Start serial connection
  Serial.begin(115200);

  g_display.Initialize(&g_lcd);
  g_dropper.Initialize(&g_input);
}


bool AskYesNo(const char* question) {
  g_display.Show(question);
  InputEvent e;
  while (true) {
    yield();
    if (!g_input.Get(&e))
      continue;
    if (e.IsKeyUp(kYesButtonKey))
      return true;
    if (e.IsKeyUp(kNoButtonKey))
      return false;
    if (e.kind == kKeyDown)
      continue;
    Serial.print("Ignoring key ");
    Serial.println(e.key);
  }
}

void StartGame() {
  g_display.Show("Let's start. I'll go first!");
  while(true) {
    yield();
  }
}

void WaitForUp(InputEvent down) {
  InputEvent e;
  while (true) {
    yield();
    if (g_input.Get(&e)) {
      if (e.IsKeyUp(down.key))
        return;
    }
  }
}

void ShowMessage(const char* msg) {
  g_display.Show(msg);
  delay(kShowMessageTimeoutMs);
}

void RunTest() {
  for (int i = 0; i < 7; ++i) {
    InputEvent e;
    if (!g_dropper.MoveToColumn(i)) {
      if (!g_input.Get(&e)) {
        ShowMessage("Failed");
        return;
      }
      if (e.IsKeyDown(kYesButtonKey) ||
          e.IsKeyDown(kNoButtonKey)) {
        WaitForUp(e);;
        if (AskYesNo("Exit test early?")) {
          return;
        }
        --i;
        continue;
      } else {
        Serial.print("Ignoring ");
        Serial.println(e.key);
      }
    }
    if (!g_dropper.DropAndWait()) {
      ShowMessage("Disc never fell. Empty? Stuck?");
      return;
    }
  }
  ShowMessage("Test passed!");
}

int main(void) {
  init();
  setup();

  while (true) {
   if (AskYesNo("Do you want to play?")) {
      if (AskYesNo("Do you want to go easy?")) {
        StartGame();
      } else if (AskYesNo("Do you want to go medium?")) {
        StartGame();
      } else if (AskYesNo("Do you want to go hard?")) {
        StartGame();
      }
    } else if (AskYesNo("Do you want to run a test?")) {
      while (!AskYesNo("Is hopper full, board empty?"));
      RunTest();
    }
  }
}

void yield() {
  g_input.Poll();
  if (serialEventRun) serialEventRun();
}
