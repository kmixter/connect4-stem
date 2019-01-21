#include <Arduino.h>
#include <Adafruit_MotorShield.h>
#include <Servo.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include "input_manager.h"

LiquidCrystal_I2C g_lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display
Adafruit_MotorShield g_motors;
const int kStepperStepsPerRevolution = 200;
const int kStepperMotorPort = 2;
Adafruit_StepperMotor* g_stepper;
Servo g_servo;
const int kServoOutput = 10;
const int kServoLoadingAngle = 0;
const int kServoUnloadingAngle = 80;
const int kStepsBetweenColumns = 170;
const int kDropTimeoutMs = 1000;
const int kErrorTimeoutMs = 5000;

typedef void (*StateHandler)();
StateHandler g_state = nullptr;

InputManager g_input;

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

void setup()
{
  g_lcd.init();                      // initialize the lcd 
  g_lcd.init();
  g_lcd.backlight();

  // Start serial connection
  Serial.begin(9600);
  // Configure pin 12/13 for inputs from momentary
  // push bottons connected to 5V
  g_state = HandleInitialState;

  g_motors.begin();
  g_stepper = g_motors.getStepper(kStepperStepsPerRevolution, kStepperMotorPort);
  g_stepper->setSpeed(10);  // 10 RPM
  g_servo.attach(kServoOutput);
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
  Serial.print("Showing ");
  Serial.println(s);
  g_lcd.clear();
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
    g_lcd.setCursor((16 - (last_break - line_start)) / 2, l);
    for (int j = line_start; j < last_break; ++j)
      g_lcd.write(s[j]);
    i = last_break;
    if (s[i] == ' ')
      ++i;
  }
  if (current_) free(current_);
  current_ = strdup(s);
}

static void HandleInitialState() {
  g_display.Show("Do you want to play?");
  g_stepper->release();
  g_servo.write(kServoLoadingAngle);
  InputEvent e;
  if (!g_input.Get(&e))
    return;
  if (e.IsKeyUp(kYesButtonKey))
    g_state = HandleStartEasyState;
  else if (e.IsKeyUp(kNoButtonKey))
    g_state = HandleAskDiagnosticsState;
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

static void HandleAskDiagnosticsState() {
  g_display.Show("Should I run a test?");
  InputEvent e;
  if (!g_input.Get(&e))
    return;
  if (e.IsKeyUp(kYesButtonKey))
    g_state = HandleDiagnosticsPrecondState;
  if (e.IsKeyUp(kNoButtonKey))
    g_state = HandleInitialState;
}

static int s_total_steps_backward;
static unsigned long s_millis_start_error_display;

static void HandleDiagnosticsPrecondState() {
  g_display.Show("Is hopper full and board empty?");
  InputEvent e;
  if (!g_input.Get(&e))
    return;
  if (e.IsKeyUp(kYesButtonKey)) {
    s_total_steps_backward = 0;
    g_state = HandleDiagnosticsHomingBackwardState;
  }
}

static void HandleDiagnosticsHomingBackwardState() {
  g_display.Show("Running test!");
  if (g_input.GetCurrentState() & (1 << kHomeSwitchKey)) {
    g_state = HandleDiagnosticsHomingForwardState;
    Serial.println("Hit home switch, moving forward");
    return;
  }
  g_stepper->step(2, BACKWARD, SINGLE);
  s_total_steps_backward += 2;
  if (s_total_steps_backward > 8 * kStepsBetweenColumns) {
    g_display.Show("Not moving? Stuck? Didn't find home.");
    s_millis_start_error_display = millis();
    g_state = HandleDiagnosticErrorTimeoutState;
  }
}

static int s_current_column_wait;
static unsigned long s_millis_start_wait;

static void HandleDiagnosticsHomingForwardState() {
  g_stepper->step(20, FORWARD, SINGLE);
  g_servo.write(kServoUnloadingAngle);
  g_state = HandleWaitForDiscDrop;
  s_millis_start_wait = millis();
  s_current_column_wait = 0;
}

static int s_steps_left;
static void HandleWaitForDiscDrop() {
  InputEvent e;
  g_stepper->release();
  if (millis() - s_millis_start_wait > kDropTimeoutMs) {
    g_display.Show("Disc never fell. Empty? Stuck?");
    s_millis_start_error_display = millis();
    g_state = HandleDiagnosticErrorTimeoutState;
    return;
  }
  bool found = false;
  InputKey this_key(InputKey(int(kColumn0Key) + s_current_column_wait));
  while (g_input.Get(&e)) {
    Serial.print("Considering ");
    Serial.println(e.key);
    if (e.IsKeyDown(this_key)) {
      // Ignore down event for when disc first goes into column before drop.
      continue;
    } else if (e.IsKeyUp(this_key)) {
      Serial.println("Found this key up");
      found = true;
    }
    else if (e.key == kHomeSwitchKey) {
      // Ignore home microswitch pushes enqueued from when we seeked
      // home position.
      continue;
    } else {
      Serial.print("Unexpected key pressed = ");
      Serial.println(e.key);
    }
  }

  if (!found) {
    return;
  }

  g_servo.write(kServoLoadingAngle);
  if (s_current_column_wait < 6) {
    ++s_current_column_wait;
    Serial.print("Going to next column ");
    Serial.println(s_current_column_wait);

    g_state = HandleDiagnosticsMovingNextColumnState;
    s_steps_left = kStepsBetweenColumns;
  } else {
    Serial.println("Success!");
    g_state = HandleInitialState;
  }
}

static void HandleDiagnosticErrorTimeoutState() {
  if (millis() - s_millis_start_error_display > kErrorTimeoutMs)
    g_state = HandleInitialState;
}

static void HandleDiagnosticsMovingNextColumnState() {
  g_stepper->step(1, FORWARD, SINGLE);
  InputEvent e;
  if (g_input.Get(&e)) {
    Serial.print("Unexpected key pressed = ");
    Serial.println(e.key);
  }
  --s_steps_left;
  if (s_steps_left) return;

  g_servo.write(kServoUnloadingAngle);
  g_state = HandleWaitForDiscDrop;
  s_millis_start_wait = millis();
}

void loop()
{
  g_input.Poll();
  g_state();
}
