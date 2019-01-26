#include <Arduino.h>
#include <Adafruit_MotorShield.h>
#include <Servo.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

#include "display_controller.h"
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

DisplayController g_display;

void setup()
{
  g_display.Initialize(&g_lcd);

  // Start serial connection
  Serial.begin(115200);
  // Configure pin 12/13 for inputs from momentary
  // push bottons connected to 5V
  g_state = HandleInitialState;

  g_motors.begin();
  g_stepper = g_motors.getStepper(kStepperStepsPerRevolution, kStepperMotorPort);
  g_stepper->setSpeed(10);  // 10 RPM
  g_servo.attach(kServoOutput);
}

static void HandleInitialState() {}
static int s_total_steps_backward;
static unsigned long s_millis_start_error_display;

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

int main(void) {
  init();
  setup();

  while (true) {
    g_stepper->release();
    g_servo.write(kServoLoadingAngle);

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
      s_total_steps_backward = 0;
      g_state = HandleDiagnosticsHomingBackwardState;
      while (g_state != HandleInitialState) {
        g_state();
        yield();
      }
    }
  }
}

void yield() {
  g_input.Poll();
  if (serialEventRun) serialEventRun();
}
