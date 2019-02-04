#include "roombabot.h"

#include "prng.h"

bool RoombaBot::FindNextMove(Board* b, int* column) {
  bool valid_moves[7];
  int valid_move_count = 0;
  for (int i = 0; i < 7; ++i) {
    if (b->Add(i, my_disc())) {
      b->UnAdd(i);
      valid_moves[i] = true;
      ++valid_move_count;
    } else {
      valid_moves[i] = false;
    }
  }

  if (!valid_move_count)
    return false;

  int valid_index = prng_->Roll(valid_move_count);
  for (int i = 0; i < 7; ++i) {
    if (valid_moves[i]) {
      if (!valid_index) {
        *column = i;
        return true;
      }
      --valid_index;
    }
  }

  // Should not reach.
  return false;
}
