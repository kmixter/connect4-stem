#include "roombabot.h"

#include "prng.h"

void RoombaBot::FindNextMove(Board* b, Observer* o) {
  bool valid_moves[7];
  PlayerBot::Observer::State s;
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

  if (!valid_move_count) {
    o->Observe(&s);
    return;
  }

  int valid_index = prng_->Roll(valid_move_count);
  for (int i = 0; i < 7; ++i) {
    if (valid_moves[i]) {
      if (!valid_index) {
        s.kind = PlayerBot::Observer::kMoveDone;
        s.column = i;
        o->Observe(&s);
        return;
      }
      --valid_index;
    }
  }

  // Should not reach.
}
