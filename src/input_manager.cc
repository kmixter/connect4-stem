#include "input_manager.h"

#include <string.h>

#ifdef TESTING
int _testing_current_state;
#endif

InputManager::InputManager() : last_state_(0), last_enqueued_(-1) {
#ifndef TESTING
  pinMode(kYesButton, INPUT_PULLUP);
  pinMode(kNoButton, INPUT_PULLUP);
  pinMode(kAnalogHomeSwitch, INPUT_PULLUP);
  for (int i = 0; i < 7; ++i)
    pinMode(kAnalogColumn0 + i, INPUT);
#endif  // TESTING
}

void InputManager::Poll() {
  int current_state = GetCurrentState();
  int diffs = current_state ^ last_state_;
  for (int i = 0; i < kKeyMax; ++i) {
    if (diffs & (1 << i)) {
      if (current_state & (1 << i)) {
        Enqueue(InputEvent(kKeyDown, InputKey(i)));
      } else {
        Enqueue(InputEvent(kKeyUp, InputKey(i)));
      }
    }
  }
  last_state_ = current_state;
}

int InputManager::GetCurrentState() const {
#ifndef TESTING
  int result =
      ((!digitalRead(kNoButton)) << kNoButtonKey) |
      ((!digitalRead(kYesButton)) << kYesButtonKey) |
      ((analogRead(kAnalogHomeSwitch) > 512) << kHomeSwitchKey);
  for (int c = 0; c < 7; ++c) {
    result |= (analogRead(kAnalogColumn0 + c) < 512) <<
                   (kColumn0Key + c);
  }
  return result;
#else
  return _testing_current_state;
#endif
}

void InputManager::Enqueue(const InputEvent& e) {
  if (last_enqueued_ + 1 >= kQueueSize)
    return;
  queue_[++last_enqueued_] = e;
}

bool InputManager::Dequeue(InputEvent* e) {
  if (last_enqueued_ < 0)
    return false;
  *e = queue_[0];
  memmove(&queue_[0], &queue_[1], sizeof(InputEvent[kQueueSize-1]));
  --last_enqueued_;
  return true;
}

bool InputManager::Peek(InputEvent* e) const {
  if (last_enqueued_ < 0)
    return false;
  *e = queue_[0];
  return true;  
}

bool InputManager::GetFiltered(InputKey k, InputEvent* e) {
  int i;
  for (i = 0; i <= last_enqueued_; ++i) {
    if (queue_[i].key == k)
      break;
  }
  if (i > last_enqueued_)
    return false;
  *e = queue_[i];
  memmove(&queue_[i], &queue_[i + 1], sizeof(InputEvent[kQueueSize - i - 1]));
  --last_enqueued_;
  return true;
}