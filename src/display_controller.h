#ifndef _DISPLAY_CONTROLLER_H
#define _DISPLAY_CONTROLLER_H

class LiquidCrystal_I2C;


class DisplayController {
 public:
  DisplayController() : current_(nullptr), lcd_(nullptr) {}

  void Initialize(LiquidCrystal_I2C* lcd);
  void Show(const char* s);

 private:
  char* current_;
  LiquidCrystal_I2C* lcd_;
};

#endif  // _DISPLAY_CONTROLLER_H