#include "dropper_controller.h"

#include <Arduino.h>

#include "input_manager.h"

const int kStepperStepsPerRevolution = 200;
const int kStepperMotorPort = 2;
const int kServoOutput = 10;
const int kServoLoadingAngle = 0;
const int kServoUnloadingAngle = 80;
const int kStepsAtColumn0 = 20;
const int kStepsBetweenColumns = 170;
const int kDropTimeoutMs = 1000;
const int kErrorTimeoutMs = 5000;
const int kDropDebounceTimeoutMs = 200;
const int kStepMax = kStepsAtColumn0 + 7 * kStepsBetweenColumns;

DropperController::DropperController() : current_step_(-1), input_(nullptr) {
}

void DropperController::Reset() {
	current_step_ = -1;
	servo_.write(kServoLoadingAngle);
}

void DropperController::Initialize(InputManager* input) {
	input_ = input;

  motors_.begin();

  stepper_ = motors_.getStepper(kStepperStepsPerRevolution, kStepperMotorPort);
  stepper_->setSpeed(10);  // 10 RPM
  stepper_->release();

  servo_.attach(kServoOutput);
  Reset();
}

bool DropperController::MoveToStep(int target_step) {
	if (target_step < kStepsAtColumn0 || target_step > kStepMax) {
		return false;
	}

	if (current_step_ < 0) {
		// First we need to determine our current location.
		Serial.println("Finding home");
		int steps_left = kStepMax + kStepsAtColumn0;

		while (steps_left > 0) {
			yield();
			InputEvent e;
			if (input_->Peek(&e)) {
				if (e.IsKeyDown(kHomeSwitchKey)) {
					Serial.println("Hit home switch, found 0");
			  	current_step_ = 0;
			  	input_->Get(&e);
			  	break;
			  } else {
			  	Serial.print("Interrupted by key during homing, ");
			  	Serial.println(e.key);
			  	return false;
			  }
			}
		  stepper_->step(2, BACKWARD, SINGLE);
		  steps_left -= 2;
		}

		if (current_step_ < 0) {
			Serial.println("Couldn't find home.");
			return false;
		}
  }

  Serial.print("Stepping to ");
  Serial.println(target_step);

  while (current_step_ != target_step) {
  	yield();
  	if (current_step_ < target_step) {
		  stepper_->step(1, FORWARD, SINGLE);
		  ++current_step_;
  	} else {
			stepper_->step(1, BACKWARD, SINGLE);
		  --current_step_;
		}
	  InputEvent e;
	  while (input_->Peek(&e)) {
	  	if (!e.IsKeyUp(kHomeSwitchKey)) {
		    Serial.print("Interrupted by key during move, ");
		    Serial.println(e.key);
		    return false;
		  }
		  input_->Get(&e);
	  }
	}

	Serial.print("Moved to ");
	Serial.println(current_step_);
	return true;
}

int DropperController::GetColumn() const {
	if (current_step_ < kStepsAtColumn0)
		return -1;
	if ((current_step_ - kStepsAtColumn0) % kStepsBetweenColumns != 0)
		return -1;
	return (current_step_ - kStepsAtColumn0) / kStepsBetweenColumns;
}

bool DropperController::DropAndWait() {
	int this_column = GetColumn();
	if (this_column < 0) {
		Serial.println("Cannot drop when not aligned");
		return false;
	}
  servo_.write(kServoUnloadingAngle);
  stepper_->release();

  unsigned long start_ms = millis();
  InputKey this_key(InputKey(int(kColumn0Key) + this_column));
	InputEvent e;
  bool found = false;

  while (!found && millis() - start_ms < kDropTimeoutMs) {
  	yield();
		while (input_->GetFiltered(this_key, &e)) {
	    if (e.kind == kKeyUp) {
	      Serial.println("Disc passed interrupter");
	      found = true;
	    }
	  }
  }

  // Within a short period of time after dropping the disc, we may
  // get a second enter/exit. Wait for this to pass and consume
  // that event.
  delay(kDropDebounceTimeoutMs);
  while (input_->GetFiltered(this_key, &e)) {
  	Serial.println("Consumed interrupter bounce");
  }

  servo_.write(kServoLoadingAngle);

  return found;
}

bool DropperController::MoveToColumn(int column) {
	if (column < 0 || column > 6)
		return false;
	return MoveToStep(kStepsAtColumn0 + kStepsBetweenColumns * column);
}
