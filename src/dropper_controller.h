#ifndef _DROPPER_CONTROLLER_H
#define _DROPPER_CONTROLLER_H

#include <Adafruit_MotorShield.h>
#include <Servo.h>

class InputManager;

class DropperController {
 public:
 	DropperController();

 	void Initialize(InputManager* input);

  // Reset internal state after a malfunction.
  void Reset();

 	// Move truck to given column,
 	bool MoveToColumn(int column);

 	// Move truck to given exact step.
 	bool MoveToStep(int step);

 	// Drop a disc and wait until it has fallen.
 	bool DropAndWait();

 	// Gets the column the truck is currently at, or -1 if unknown or misaligned.
	int GetColumn() const;

	// Gets the closest column the truck is currently at.
	int GetClosestColumn() const;

 private:
  Adafruit_MotorShield motors_;
  Adafruit_StepperMotor* stepper_;
  Servo servo_;
  InputManager* input_;

  // Current step number, or -1 if unknown
  int current_step_;
};

#endif  // _DROPPER_CONTROLLER_H