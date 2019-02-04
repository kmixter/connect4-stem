#include "r2d2bot.h"

#include "prng.h"

bool R2D2Bot::FindNextMove(Board* board, int* out_column) {
  int max_streaks[7] = {0};
  for (auto column = 0; column < 7; ++column) {
    int row;
    if (!board->Add(column, my_disc_, &row)) 
      continue;  // This column must be full.
    max_streaks[column] = board->FindMaxStreakAt(row, column);
    board->UnAdd(column);
    if (max_streaks[column] >= 4) {
      *out_column = column;
      return true;
    }

    if (board->Add(column, opponent_disc_)) {
      auto this_streak_count = board->FindMaxStreakAt(row, column);
      board->UnAdd(column);
      if (this_streak_count >= 4) {
        *out_column = column;
        return true;
      }
    }
  }
  int max_max_streak = 0;
  int max_max_streak_count = 0;
  for (int i = 0; i < 7; ++i) {
    if (max_streaks[i] == 0)
      continue;
    if (max_streaks[i] > max_max_streak) {
      max_max_streak_count = 1;
      max_max_streak = max_streaks[i];
    } else if (max_streaks[i] == max_max_streak) {
      ++max_max_streak_count;
    }
  }
  if (!max_max_streak_count) {
    // We were unable to find a single column that we could add to.
    return false;
  }

  int index = prng_->Roll(max_max_streak_count);
  for (int i = 0; i < 7; ++i) {
    if (max_streaks[i] != max_max_streak)
      continue;
    if (index == 0) {
      *out_column = i;
      return true;
    }
    --index;
  }

  // Not expected to be reached.
  return false;
}
