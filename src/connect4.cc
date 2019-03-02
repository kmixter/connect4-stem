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
const int kShowMessageTimeoutMs = 2500;

typedef void (*StateHandler)();
StateHandler g_state = nullptr;

InputManager g_input;
DropperController g_dropper;

const unsigned long kUserSwitchColumnTimeoutMs = 2000;
static const char kExitGameEarly[] = "End game early?";

DisplayController g_display;
char g_string[64];

SmallPRNG g_prng(0);
RandomBot g_randombot(kRedDisc, &g_prng);
Rule3Bot g_rule3bot(kRedDisc, &g_prng);
MaxBotConstantEvals g_maxbot(kRedDisc, 2500, &g_prng, true);

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

void WaitForUp(const InputEvent& down) {
  InputEvent e;
  while (true) {
    yield();
    if (g_input.Get(&e)) {
      if (e.IsKeyUp(down.key))
        return;
    }
  }
}

void DumpBoardToSerial(Board* b, const char* desc) {
  Serial.println(desc);
  for (int row = 5; row >= 0; --row) {
    for (int col = 0; col < 7; ++col) {
      switch (b->Get(row, col)) {
        case kRedDisc:
          Serial.print("R ");
          break;
        case kYellowDisc:
          Serial.print("Y ");
          break;
        default:
          Serial.print("_ ");
          break;
      }
    }
    Serial.println("");
  }
}

void ShowMessage(const char* msg) {
  g_display.Show(msg);
  delay(kShowMessageTimeoutMs);
  g_input.Flush();
}

struct UserMove {
  UserMove() : column(-1), row(-1), selection_ms(0), done_showing(false) {}

  bool IsSet() const {
    return column >= 0;
  }

  bool IsSettled() const {
    return millis() - selection_ms >= kUserSwitchColumnTimeoutMs;
  }

  void Reset() {
    column = -1;
    row = -1;
    selection_ms = 0;
    done_showing = false;
  }

  int column;
  int row;
  unsigned long selection_ms;
  bool done_showing;
};

void UndoUserMove(Board* b, UserMove* user_move) {
  if (!user_move->IsSet())
    return;
  b->UnAdd(user_move->column);
  user_move->Reset();
}

void ShowUserMove(PlayerBot* bot, Board* b, UserMove* user_move) {
  if (user_move->IsSet() && !user_move->done_showing) {
    strcpy(g_string, "You moved to ");
    strcat(g_string, Board::GetCellLocator(user_move->row, user_move->column));
    g_display.Show(g_string, true);
    if (user_move->IsSettled())
      user_move->done_showing = true;
  } else {
    strcpy(g_string, bot->GetName());
    strcat(g_string, " is thinking");
    g_display.Show(g_string, true);
    g_display.UpdateBoardBitmap(b->GetBitmap());
  }
}

enum BotResult {
  kBotMoved,
  kBotTurnUserDoOver,
  kBotSignalExit,
  kBotContinue
};

class InterruptableObserver : public SimpleObserver {
 public:
  InterruptableObserver() {}

  bool Observe(PlayerBot::Observer::State* s) {
    InputEvent e;
    yield();
    if (g_input.Peek(&e)) {
      Serial.println("Interruption during observer");
      return false;
    }
    if (s->kind == kHeuristicDone) {
      ++heuristics_computed;
      if (heuristics_computed % 200 == 1) {
        Serial.println("200 more heuristics");
      }
    } else if (s->kind == kMoveDone) {
      heuristic = s->heuristic;
      return SimpleObserver::Observe(s);
    }
    return true;
  }
  void Reset() {
    heuristics_computed = 0;
  }

  int heuristic = 0;
  int heuristics_computed = 0;
};

BotResult HandleKeyDuringBotTurn(Board* b, InputEvent* e,
                                 UserMove* user_move) {
  switch (e->key) {
    case kYesButtonKey:
    case kNoButtonKey: {
      if (e->kind == kKeyDown)
        WaitForUp(*e);
      if (user_move->IsSet()) {
        strcpy(g_string, "Was ");
        strcat(g_string, b->GetCellLocator(user_move->row, user_move->column));
        strcat(g_string, " your last move?");
        if (!AskYesNo(g_string)) {
          UndoUserMove(b, user_move);
          g_display.Show("Realigning, please wait...");
          if (!g_dropper.MoveToColumn(g_dropper.GetClosestColumn())) {
            ShowMessage("Sorry, too many mistakes.");
            return kBotSignalExit;
          }
          return kBotTurnUserDoOver;
        }
      } else {
        if (AskYesNo(g_string)) {

        }
      }
      if (AskYesNo(kExitGameEarly)) {
        return kBotSignalExit;
      }
      return kBotContinue;
    }

    case kHomeSwitchKey: {
      ShowMessage("Home key pushed. Failed.");
      return kBotSignalExit;
    }

    case kColumn0Key:
    case kColumn1Key:
    case kColumn2Key:
    case kColumn3Key:
    case kColumn4Key:
    case kColumn5Key:
    case kColumn6Key: {
      if (e->kind == kKeyUp) {
        // We have already somehow consumed the up event for the column,
        // which is a problem because we need the user handler to consume
        // this. Fail out. Hopefully this won't happen.
        Serial.println("Up event for column");
        break;
      }
      if (user_move->IsSet()) {
        // User moves while the bot is thinking/moving. Let's assume they aren't
        // cheating and they moved their disc from where we thought
        // they last put it.
        UndoUserMove(b, user_move);
        return kBotTurnUserDoOver;
      }
      break;
    }
    case kKeyMax: // avoid warning.
      break;
  }
  ShowMessage("I am confused! Ending game.");
  return kBotSignalExit;
}

BotResult HandleBotTurn(Board* b, PlayerBot* bot, CellContents disc,
                        UserMove* user_move) {
  int bot_row;
  InputEvent e;
  InterruptableObserver o;

  while (true) {
    o.Reset();
    ShowUserMove(bot, b, user_move);
    g_display.UpdateBoardBitmap(b->GetBitmap());
    unsigned long bot_start_ms = millis();
    DumpBoardToSerial(b, "Bot board");
    bot->FindNextMove(b, &o);
    if (o.success) {
      Serial.print("Bot took ");
      Serial.print(millis() - bot_start_ms);
      Serial.print("ms and computed ");
      Serial.print(o.heuristics_computed);
      Serial.print(" heuristics, final ");
      Serial.println(o.heuristic);
      break;
    }
    if (!g_input.Peek(&e)) {
      ShowMessage("Bot failed.");
      return kBotSignalExit;
    }
    Serial.println("Handling PlayerBot interrupt");
    while (g_input.Get(&e)) {
      BotResult r = HandleKeyDuringBotTurn(b, &e, user_move);
      if (r != kBotContinue)
        return r;
    }
  }

  if (!b->Add(o.column, disc, &bot_row)) {
    ShowMessage("Bot failed.");
    return kBotSignalExit;
  }
  // Now remove the chip until we really have placed it.
  b->UnAdd(o.column);

  // Continue waiting for the user's move to settle (enough time to pass).
  // We promise to give the user a settle time to drop their disc and
  // move their hand away before we move the dropper. If we are running a
  // slow bot, enough time will have already passed.
  while (!user_move->IsSettled()) {
    yield();
    if (g_input.Peek(&e)) {
      BotResult r = HandleKeyDuringBotTurn(b, &e, user_move);
      if (r != kBotContinue)
        return r;
    }
  }

  bool successful_move = false;

  while (!successful_move) {
    strcpy(g_string, bot->GetName());
    strcat(g_string, " picks ");
    strcat(g_string, b->GetCellLocator(bot_row, o.column));
    g_display.UpdateBoardBitmap(b->GetBitmap());
    g_display.Show(g_string, true);

    if (!g_dropper.MoveToColumn(o.column) || !g_dropper.DropAndWait()) {
      if (g_input.Get(&e)) {
        BotResult r = HandleKeyDuringBotTurn(b, &e, user_move);
        if (r != kBotContinue)
          return r;
      } else {
        if (AskYesNo("Refill the hopper. Ready?")) {
          g_dropper.Reset();
        } else {
          ShowMessage("Quitting.");
          return kBotSignalExit;
        }
      }
      continue;
    }
    Serial.print(millis());
    Serial.println(", drop finished");
    successful_move = true;
  }

  if (!b->Add(o.column, disc)) {
    ShowMessage("Bot failed post");
    return kBotSignalExit;
  }
  return kBotMoved;
}

void ShowUserPrompt(Board* b) {
  if (b->GetCount() == 0) {
    g_display.Show("You go first.");
  } else {
    g_display.Show("Your turn!", true);
  }
  g_display.UpdateBoardBitmap(b->GetBitmap());
}

bool HandleUserTurn(Board* b, PlayerBot* bot, CellContents disc, UserMove* user_move) {
  InputEvent e;
  DumpBoardToSerial(b, "Your board");
  ShowUserPrompt(b);
  user_move->Reset();

  while (!user_move->IsSet()) {
    yield();
    if (!g_input.Get(&e))
      continue;
    if (e.kind == kKeyDown) {
      int column = e.key - int(kColumn0Key);
      Serial.println("Showing immediate board update");
      if (column >=0 && column <= kColumn6Key && b->Add(column, disc)) {
        g_display.UpdateBoardBitmap(b->GetBitmap());
        b->UnAdd(column);
      }
      continue;
    }

    switch (e.key) {
      case kYesButtonKey:
      case kNoButtonKey:
      case kHomeSwitchKey:
        if (AskYesNo(kExitGameEarly))
          return false;
        ShowUserPrompt(b);
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
        if (!b->Add(user_column, disc, &user_row))
          continue;

        b->UnAdd(user_column);
        user_move->column = user_column;
        user_move->row = user_row;
        user_move->selection_ms = millis();
        g_display.UpdateBoardBitmap(b->GetBitmap());
        ShowUserMove(bot, b, user_move);
        break;
      }

      case kKeyMax:  // avoid warning
        break;
    }
  }

  b->Add(user_move->column, disc);
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
  if (!AskYesNo("Is board empty and latched?")) {
    ShowMessage("Empty the board first.");
    return;
  }

  bool robot_turn = AskYesNo("Can I go first?");

  Board b;
  bool is_draw;
  UserMove user_move;

  while (true) {
    if (robot_turn) {
      BotResult bot_result = HandleBotTurn(&b, bot, kRedDisc, &user_move);

      if (bot_result == kBotSignalExit)
        break;

      if (b.IsTerminal(&is_draw)) {
        if (is_draw)
          ShowDraw();
        else
          ShowBotWins(bot, &b);
        return;
      }

    } else {
      if (!HandleUserTurn(&b, bot, kYellowDisc, &user_move))
        return;

      if (b.IsTerminal(&is_draw)) {
        if (is_draw)
          ShowDraw();
        else
          ShowUserWins(&b);
        return;
      }
    }
    robot_turn = !robot_turn;
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

  struct QuestionBot {
    const char* question;
    PlayerBot* bot;
  } question_bot[] = {
    {
      "Do you want to go easy?",
      &g_randombot
    }, {
      "Do you want to go medium?",
      &g_rule3bot
    }, {
      "Do you want to go hard?",
      &g_maxbot
    }
  };
  int question = 0;
  int num_questions = sizeof(question_bot) / sizeof(question_bot[0]);

  while (true) {
    YesNoResponse yn = AskYesNo("Do you want to play?", false, true);
    if (yn == kYesNoResponseYes) {
      uint32_t seed = micros();
      g_prng.SetSeed(seed);
      Serial.print("Setting PRNG seed to ");
      Serial.println(seed);
      int start_question = question;
      do {
        if (AskYesNo(question_bot[question].question)) {
          RunGame(question_bot[question].bot);
          break;
        } else {
          question = (question + 1) % num_questions;
        }
      } while (question != start_question);
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
