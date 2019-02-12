#include "display_controller.h"

#include <string.h>

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

#include "board.h"

void DisplayController::Initialize(LiquidCrystal_I2C* lcd) {
  lcd_ = lcd;
  lcd_->init(); 
  lcd_->backlight();
}

void DisplayController::Show(const char* s, bool with_board) {
  int i = 0;
  if (current_ && strcmp(s, current_) == 0)
    return;
  Serial.print("Showing ");
  if (with_board)
    Serial.print("(with board) ");
  Serial.println(s);
  lcd_->clear();
  int max_length = with_board ? 12 : 16;
  for (int l = 0; l < 2; ++l) {
    int line_len = 0;
    int line_start = i;
    int last_break = -1;
    for (; s[i] && line_len < max_length; ++i) {
      if (s[i] == ' ') {
        last_break = i;
      }
      ++line_len;
    }
    if (s[i] == '\0')
      last_break = i;
    else if (last_break < 0)
      last_break = i;
    if (!with_board)
      lcd_->setCursor((16 - (last_break - line_start)) / 2, l);
    else
      lcd_->setCursor(16 - (last_break - line_start), l);
    for (int j = line_start; j < last_break; ++j)
      lcd_->write(s[j]);
    i = last_break;
    if (s[i] == ' ')
      ++i;
  }

  if (with_board) {
    lcd_->setCursor(0, 0);
    for (int i = 4; i < 8; ++i) lcd_->write(i);
    lcd_->setCursor(0, 1);
    for (int i = 0; i < 4; ++i) lcd_->write(i);
  }

  if (current_) free(current_);
  current_ = strdup(s);
}

void DisplayController::UpdateBoardBitmap(uint8_t* bitmap) {
  for (int i = 0; i < 8; ++i) {
    lcd_->createChar(i, bitmap + i * 8);
  }
}