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
char g_string[64];

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

  bool successful_move = false;

  while (!successful_move) {
    strcpy(g_string, bot->GetName());
    strcat(g_string, " picks ");
    strcat(g_string, b->GetCellLocator(bot_row, bot_column));
    g_display.Show(g_string);

    if (!g_dropper.MoveToColumn(bot_column) ||
        !g_dropper.DropAndWait()) {
      if (g_input.Get(&e)) {
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
        if (AskYesNo("Please fill hopper. OK?")) {
          g_dropper.Reset();
          continue;
        } else {
          ShowMessage("Quitting.");
          return false;
        }
      }
    }
    Serial.print(millis());
    Serial.println(", drop finished");
    successful_move = true;
  }

  return true;
}

bool HandleUserTurn(Board* b, CellContents disc) {
  InputEvent e;
  unsigned long last_selection_ms = 0;
  int last_selection_column = -1;

  while (last_selection_column == -1 ||
         millis() - last_selection_ms < kUserSwitchColumnTimeoutMs) {
    yield();
    if (!g_input.Get(&e) || e.kind == kKeyDown) {
      continue;
    }
    Serial.print(millis());
    Serial.println(", user touched");

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
          strcpy(g_string, "Column X already full");
          g_string[7] = '1' + user_column;
          continue;
        }
        b->UnAdd(user_column);
        last_selection_column = user_column;
        last_selection_ms = millis();
        strcpy(g_string, "You pick ");
        strcat(g_string, b->GetCellLocator(user_row, user_column));
        g_display.Show(g_string);
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
  strcpy(g_string, bot->GetName());
  strcat(g_string, " won!!!");
  ShowMessage(g_string);
}

void RunGame(PlayerBot* bot) {
  if (bot == nullptr) {
    ShowMessage("This bot isn't ready");
    return;
  }
  strcpy(g_string, "You're against ");
  strcat(g_string, bot->GetName());
  strcat(g_string, ". Empty?");
  if (!AskYesNo(g_string)) {
    ShowMessage("Empty the board first.");
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
      if (AskYesNo("Do you want to go easy?")) {
        RunGame(nullptr);
      } else if (AskYesNo("Do you want to go medium?")) {
        RunGame(&g_r2d2bot);
      } else if (AskYesNo("Do you want to go hard?")) {
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
