#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display

typedef void (*StateHandler)();
StateHandler g_state = nullptr;

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

// input_manager.h

enum EventKind {
  kKeyDown = 0,
  kKeyUp = 1
};

enum InputKey {
  kNoButtonKey,
  kYesButtonKey,
  kHomeSwitchKey,
  kColumn0Key,
  kColumn1Key,
  kColumn2Key,
  kColumn3Key,
  kColumn4Key,
  kColumn5Key,
  kColumn6Key,
  kKeyMax
};

struct InputEvent {
  EventKind kind : 4;
  InputKey key : 4;
  InputEvent(EventKind e = kKeyDown, InputKey k = kNoButtonKey) : kind(e), key(k) {}
};

class InputManager {
 public:
  InputManager();

  int GetCurrentState() const;
  void Poll();
  bool Get(InputEvent* e) {
    return Dequeue(e);
  }

 private:
  static const int kNoButton = 8;
  static const int kYesButton = 9;
  static const int kAnalogColumn0 = A9;
  static const int kAnalogHomeSwitch = A8;
  static const int kQueueSize = 32;

  bool Enqueue(const InputEvent& e);
  bool Dequeue(InputEvent* e);

  int last_state_;
  int last_enqueued_;
  InputEvent queue_[kQueueSize];
};

InputManager::InputManager() : last_state_(0), last_enqueued_(-1) {
  pinMode(kYesButton, INPUT_PULLUP);
  pinMode(kNoButton, INPUT_PULLUP);
  pinMode(kAnalogHomeSwitch, INPUT_PULLUP);
  for (int i = 0; i < 7; ++i)
    pinMode(kAnalogColumn0 + i, INPUT);
}

void InputManager::Poll() {
  int current_state = GetCurrentState();
  int diffs = current_state ^ last_state_;
  for (int i = 0; i < kKeyMax; ++i) {
    if (diffs & (1 << i)) {
      if (current_state & (1 << i))
        Enqueue(InputEvent(kKeyDown, InputKey(i)));
      else
        Enqueue(InputEvent(kKeyUp, InputKey(i)));
    }
  }
}

int InputManager::GetCurrentState() const {
  int result =
      ((!digitalRead(kNoButton)) << kNoButtonKey) |
      ((!digitalRead(kYesButton)) << kYesButtonKey) |
      ((analogRead(kAnalogHomeSwitch) < 512) << kHomeSwitchKey);
  for (int c = 0; c < 7; ++c) {
    result |= (analogRead(kAnalogColumn0 + c) < 512) <<
                   (kColumn0Key + c);
  }
  return result;
}

bool InputManager::Enqueue(const InputEvent& e) {
  if (last_enqueued_ + 1 >= kQueueSize)
    return false;
  queue_[++last_enqueued_] = e;
  return true;
}

bool InputManager::Dequeue(InputEvent* e) {
  if (last_enqueued_ < 0)
    return false;
  *e = queue_[0];
  memmove(&queue_[0], &queue_[1], sizeof(InputEvent[kQueueSize-1]));
  --last_enqueued_;
  return true;
}

InputManager g_input;

// display_controller.h

class DisplayController {
 public:
  DisplayController() {}
  void Show(const char* s);
};

DisplayController g_display;

void DisplayController::Show(const char* s) {
  int i = 0;
  for (int l = 0; l < 2; ++l) {
    int line_len = 0;
    int line_start = i;
    int last_break = -1;
    for (; s[i] && line_len < 16; ++i) {
      if (s[i] == ' ')
        last_break = i;
      ++line_len;
    }
    if (last_break < 0)
      last_break = i;
    lcd.setCursor((16 - (last_break - line_start)) / 2, l);
    for (int j = line_start; j < last_break; ++j)
      lcd.write(s[j]);
    i = last_break;
    if (s[i] == ' ')
      ++i;
  }
}

void HandleInitialState() {
  g_display.Show("Do you want to play?");
#if 0
  InputEvent e;
  if (g_input.Get(&e)) {
    Serial.println("Got a press.");
    Serial.println(e.kind);
    Serial.println(e.key);
  }
#endif
}

void loop()
{
  g_input.Poll();
  g_state();
}
