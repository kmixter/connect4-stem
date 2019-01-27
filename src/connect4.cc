#include <Arduino.h>
#include <Adafruit_MotorShield.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

#include "display_controller.h"
#include "dropper_controller.h"
#include "input_manager.h"
#include "r2d2bot.h"

LiquidCrystal_I2C g_lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display
const int kShowMessageTimeoutMs = 5000;

typedef void (*StateHandler)();
StateHandler g_state = nullptr;

InputManager g_input;
DropperController g_dropper;

const unsigned long kUserSwitchColumnTimeoutMs = 2000;
static const char kExitGameEarly[] = "Exit game early?";

DisplayController g_display;

R2D2Bot g_r2d2bot(kRedDisc);

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

bool HandleBotTurn(Board* b, PlayerBot* bot, CellContents disc) {
  int bot_column, bot_row;
  InputEvent e;
  if (!bot->FindNextMove(b, &bot_column) ||
      !b->Add(bot_column, disc, &bot_row)) {
    ShowMessage("Bot failed. You win.");
    return false;
  }
  char display[64] = {0};
  strcpy(display, bot->GetName());
  strcat(display, " picks ");
  strcat(display, b->GetCellLocator(bot_row, bot_column));
  g_display.Show(display);

  do {
    yield();
    if (!g_dropper.MoveToColumn(bot_column) ||
        !g_dropper.DropAndWait()) {
      if (!g_input.Get(&e)) {
        if (e.IsKeyDown(kYesButtonKey) ||
            e.IsKeyDown(kNoButtonKey)) {
          WaitForUp(e);
          if (AskYesNo(kExitGameEarly)) {
            return false;
          }
          continue;
        } else if (e.key == kHomeSwitchKey) {
          ShowMessage("Home key pushed. Failed.");
          return false;
        } else {
          ShowMessage("You cheated. Bot quits!");
          return false;
        }
      } else {
        ShowMessage("Cannot drop. Failed.");
        return false;
      }
    }
  } while(false);

  return true;
}

bool HandleUserTurn(Board* b, CellContents disc) {
  char display[64] = {0};
  InputEvent e;
  unsigned long last_selection_ms = 0;
  int last_selection_column = -1;

  while (last_selection_column == -1 ||
         millis() - last_selection_ms < kUserSwitchColumnTimeoutMs) {
    yield();
    if (!g_input.Get(&e) || e.kind == kKeyDown) {
      continue;
    }

    switch (e.key) {
      case kYesButtonKey:
      case kNoButtonKey:
        if (AskYesNo(kExitGameEarly))
          return false;
        break;

      case kColumn0Key:
      case kColumn1Key:
      case kColumn2Key:
      case kColumn3Key:
      case kColumn4Key:
      case kColumn5Key:
      case kColumn6Key: {
        int user_row;
        int user_column = e.key - kColumn0Key;
        if (!b->Add(user_column, disc, &user_row)) {
          strcpy(display, "Column X already full");
          display[7] = '1' + user_column;
          continue;
        }
        b->UnAdd(user_column);
        last_selection_column = user_column;
        last_selection_ms = millis();
        strcpy(display, "You pick ");
        strcat(display, b->GetCellLocator(user_row, user_column));
        g_display.Show(display);
        break;
      }
    }
  }

  b->Add(last_selection_column, disc);
  return true;
}

void ShowDraw() {
  ShowMessage("It's a draw! Good game!");
}

void ShowUserWins() {
  ShowMessage("What an upset, you win!");
}

void ShowBotWins(PlayerBot* bot) {
  char display[64] = {0};
  strcpy(display, bot->GetName());
  strcat(display, " won!!!");
  ShowMessage(display);
}

void RunGame(PlayerBot* bot) {
  if (bot == nullptr) {
    ShowMessage("This bot isn't ready");
    return;
  }
  while (!AskYesNo("Is hopper full, board empty?")) {
    ShowMessage("Empty the board and fill the hopper");
    return;
  }

  Board b;
  bool is_draw;

  while (true) {
    if (!HandleBotTurn(&b, bot, kRedDisc))
      return;

    if (b.IsTerminal(&is_draw)) {
      if (is_draw)
        ShowDraw();
      else
        ShowBotWins(bot);
      return;
    }

    g_display.Show("Your turn!");

    if (!HandleUserTurn(&b, kYellowDisc))
      return;

    if (b.IsTerminal(&is_draw)) {
      if (is_draw)
        ShowDraw();
      else
        ShowUserWins();
      return;
    }
  }
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
        WaitForUp(e);
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
      if (AskYesNo("Do you want to play Roomba?")) {
        RunGame(nullptr);
      } else if (AskYesNo("Do you want to play R2D2?")) {
        RunGame(&g_r2d2bot);
      } else if (AskYesNo("Do you want to play?")) {
        RunGame(nullptr);
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
