#include "rule3bot.h"

#include "prng.h"

void Rule3Bot::FindNextMove(Board* board, Observer* o) {
  int max_streaks[7] = {0};
  Observer::State s;

  /* Rule 1: Take immediate win if can. */
  for (int column = 0; column < 7; ++column) {
    int row;
    if (!board->Add(column, my_disc_, &row)) 
      continue;  // This column must be full.
    max_streaks[column] = board->FindMaxStreakAt(row, column);
    board->UnAdd(column);
    if (max_streaks[column] >= 4) {
      s.kind = PlayerBot::Observer::kMoveDone;
      s.column = column;
      o->Observe(&s);
      return;
    }
  }

  /* Rule 2: Defend immediate loss if can. */
  for (int column = 0; column < 7; ++column) {
    int row;
    if (board->Add(column, opponent_disc_, &row)) {
      auto this_streak_count = board->FindMaxStreakAt(row, column);
      board->UnAdd(column);
      if (this_streak_count >= 4) {
        s.kind = PlayerBot::Observer::kMoveDone;
        s.column = column;
        o->Observe(&s);
        return;
      }
    }
  }

  /* Rule 3: Take the maximum streak, picking randomly if multiple. */
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
    s.kind = PlayerBot::Observer::kNoMovePossible;
    o->Observe(&s);
    return;
  }

  int index = prng_->Roll(max_max_streak_count);
  for (int i = 0; i < 7; ++i) {
    if (max_streaks[i] != max_max_streak)
      continue;
    if (index == 0) {
      s.kind = PlayerBot::Observer::kMoveDone;
      s.column = i;
      o->Observe(&s);
      return;
    }
    --index;
  }

  // Not expected to be reached.
  return;
}
