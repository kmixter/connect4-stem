#include "display_controller.h"

#include <string.h>

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

void DisplayController::Initialize(LiquidCrystal_I2C* lcd) {
  lcd_ = lcd;
  lcd_->init(); 
  lcd_->backlight();
}

void DisplayController::Show(const char* s) {
  int i = 0;
  if (current_ && strcmp(s, current_) == 0)
    return;
  Serial.print("Showing ");
  Serial.println(s);
  lcd_->clear();
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
    lcd_->setCursor((16 - (last_break - line_start)) / 2, l);
    for (int j = line_start; j < last_break; ++j)
      lcd_->write(s[j]);
    i = last_break;
    if (s[i] == ' ')
      ++i;
  }
  if (current_) free(current_);
  current_ = strdup(s);
}

