#include <Arduino.h>
#include <Adafruit_MotorShield.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

#include "display_controller.h"
#include "dropper_controller.h"
#include "input_manager.h"
#include "prng.h"
#include "maxbot.h"
#include "rule3bot.h"
#include "randombot.h"

LiquidCrystal_I2C g_lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display
const int kShowMessageTimeoutMs = 5000;

typedef void (*StateHandler)();
StateHandler g_state = nullptr;

InputManager g_input;
DropperController g_dropper;

const unsigned long kUserSwitchColumnTimeoutMs = 2000;
static const char kExitGameEarly[] = "End game early?";
static const char kYourTurn[] = "Your turn!";

DisplayController g_display;
char g_string[64];

SmallPRNG g_prng(0);
RandomBot g_randombot(kRedDisc, &g_prng);
Rule3Bot g_rule3bot(kRedDisc, &g_prng);
MaxBot g_maxbot(kRedDisc, 4, &g_prng);

void setup()
{
  // Start serial connection
  Serial.begin(115200);

  g_display.Initialize(&g_lcd);
  g_dropper.Initialize(&g_input);
}

enum YesNoResponse {
  kYesNoResponseNo,
  kYesNoResponseYes,
  kYesNoResponseBothButtons
};

YesNoResponse AskYesNo(const char* question, bool blink = false,
                       bool allow_both_buttons = false) {
  g_display.Show(question);
  InputEvent e;
  unsigned long last_ms = millis();
  const int kBlinkIntervalMs = 500;
  bool display_on = true;
  bool yes_down = false, no_down = false, both_down = false;

  while (true) {
    yield();
    if (blink && millis() - last_ms > kBlinkIntervalMs) {
      display_on = !display_on;
      if (display_on)
        g_lcd.backlight();
      else
        g_lcd.noBacklight();
      last_ms = millis();
    }
    if (!g_input.Get(&e))
      continue;
    if (e.IsKeyUp(kYesButtonKey)) {
      g_lcd.backlight();
      yes_down = false;
      if (!(allow_both_buttons && both_down))
        return kYesNoResponseYes;
    }
    if (e.IsKeyUp(kNoButtonKey)) {
      g_lcd.backlight();
      no_down = false;
      if (!(allow_both_buttons && both_down))
        return kYesNoResponseNo;
    }
    if (allow_both_buttons && both_down && !yes_down && !no_down) {
      return kYesNoResponseBothButtons;
    }
    if (e.kind == kKeyDown) {
      if (e.IsKeyDown(kYesButtonKey))
        yes_down = true;
      if (e.IsKeyDown(kNoButtonKey))
        no_down = true;
      if (yes_down && no_down)
        both_down = true;
      continue;
    }
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
  int bot_row;
  InputEvent e;
  SimpleObserver o;
  bot->FindNextMove(b, &o);
  if (!o.success || !b->Add(o.column, disc, &bot_row)) {
    ShowMessage("Bot failed.");
    return false;
  }

  bool successful_move = false;

  while (!successful_move) {
    strcpy(g_string, bot->GetName());
    strcat(g_string, " picks ");
    strcat(g_string, b->GetCellLocator(bot_row, o.column));
    g_display.Show(g_string);

    if (!g_dropper.MoveToColumn(o.column) ||
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
  g_display.Show(kYourTurn);

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
        g_display.Show(kYourTurn);
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
  AskYesNo("It's a draw! Good game! Continue?", true);
}

void ShowUserWins(Board* b) {
  strcpy(g_string, "Good job, you win! ");
  strcat(g_string, b->GetWinLocator());
  strcat(g_string, " Continue?");
  AskYesNo(g_string, true);
}

void ShowBotWins(PlayerBot* bot, Board* b) {
  strcpy(g_string, bot->GetName());
  strcat(g_string, " won!! ");
  strcat(g_string, b->GetWinLocator());
  strcat(g_string, " Continue?");
  AskYesNo(g_string, true);
}

void RunGame(PlayerBot* bot) {
  if (bot == nullptr) {
    ShowMessage("This bot isn't ready");
    return;
  }
  strcpy(g_string, "You're against ");
  strcat(g_string, bot->GetName());
  strcat(g_string, ". Ready?");
  if (!AskYesNo(g_string)) {
    return;
  }
  if (!AskYesNo("Is board empty and latched?")) {
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
        ShowBotWins(bot, &b);
      return;
    }

    if (!HandleUserTurn(&b, kYellowDisc))
      return;

    if (b.IsTerminal(&is_draw)) {
      if (is_draw)
        ShowDraw();
      else
        ShowUserWins(&b);
      return;
    }
  }
}

void RunTest() {
  g_display.Show("Here it goes...");
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
    YesNoResponse yn = AskYesNo("Do you want to play?", false, true);
    if (yn == kYesNoResponseYes) {
      uint32_t seed = micros();
      g_prng.SetSeed(seed);
      Serial.print("Setting PRNG seed to ");
      Serial.println(seed);
      if (AskYesNo("Do you want to go easy?")) {
        RunGame(&g_randombot);
      } else if (AskYesNo("Do you want to go medium?")) {
        RunGame(&g_rule3bot);
      } else if (AskYesNo("Do you want to go hard?")) {
        RunGame(&g_maxbot);
      }
    } else if (yn == kYesNoResponseNo) {
      ShowMessage("Sorry, I don't know any jokes.");
    } else if (yn == kYesNoResponseBothButtons) {
      if (AskYesNo("Run dropper test?"))
        RunTest();
    }
  }
}

void yield() {
  g_input.Poll();
  if (serialEventRun) serialEventRun();
}
