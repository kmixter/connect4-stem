#ifndef _INPUT_MANAGER_H
#define _INPUT_MANAGER_H

#ifndef TESTING
#include <Arduino.h>
#else
#define A8 100
#define A9 100
#endif

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
  bool IsKeyDown(InputKey k) const { return kind == kKeyDown && key == k; }
  bool IsKeyUp(InputKey k) const { return kind == kKeyUp && key == k; }
};

class InputManager {
 public:
  static const int kQueueSize = 32;

  InputManager();

  int GetCurrentState() const;
  void Poll();
  bool Get(InputEvent* e) {
    return Dequeue(e);
  }
  void Flush();

  // Get an input event of this key.
  bool GetFiltered(InputKey k, InputEvent* e);

  // Return current top of queue without dequeuing.
  bool Peek(InputEvent* e) const;

 private:
  static const int kNoButton = 8;
  static const int kYesButton = 9;
  static const int kAnalogColumn0 = A9;
  static const int kAnalogHomeSwitch = A8;

  void Enqueue(const InputEvent& e);
  bool Dequeue(InputEvent* e);

  int last_state_;
  int last_enqueued_;
  InputEvent queue_[kQueueSize];
};

#ifdef TESTING
extern int _testing_current_state;
#endif

#endif  // _INPUT_MANAGER_H