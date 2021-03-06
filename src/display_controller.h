#ifndef _DISPLAY_CONTROLLER_H
#define _DISPLAY_CONTROLLER_H

#include <stdint.h>

class LiquidCrystal_I2C;

class Board;

class DisplayController {
 public:
  DisplayController() : current_(nullptr), lcd_(nullptr) {}

  void Initialize(LiquidCrystal_I2C* lcd);
  void Show(const char* s, bool with_board = false);
  void UpdateBoardBitmap(uint8_t* bitmap);
 private:
  char* current_;
  LiquidCrystal_I2C* lcd_;
};

#endif  // _DISPLAY_CONTROLLER_H