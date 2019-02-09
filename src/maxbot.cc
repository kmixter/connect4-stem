#include "maxbot.h"

#include "prng.h"

static const int kMaxHeuristic = 10000;

// H(color) = all possible available wins for that color that involve
// at least one piece for that color on the board. Heuristic returned
// is H(red) - H(yellow).
int MaxBot::ComputeHeuristic(Board* b) const {
  int value = 0;
  for (int row = 0; row < 6; ++row) {
    for (int col = 0; col < 7; ++col) {
      for (int dr = 0; dr < 2; ++dr) {
        for (int dc = -1; dc < 2; ++dc) {
          if (dr == 0 && dc != 1) continue;
          // For each cell in the grid, for each of the 4 canonical directions:
          bool any_yellow = false;
          bool any_red = false;
          int r = row, c = col;
          //printf("(%d,%d) (%d,%d): ", row, col, dr, dc);
          int dist = 0;
          while (true) {
            //printf("%c", b->contents_[r][c] == kRedDisc ? 'R' : (b->contents_[r][c] == kYellowDisc ? 'Y' : '_'));
            if (b->contents_[r][c] == kRedDisc) {
              any_red = true;
              if (any_yellow) break;
            } else if (b->contents_[r][c] == kYellowDisc) {
              any_yellow = true;
              if (any_red) break;
            }
            ++dist;
            if (dist == 4) break;
            r += dr;
            c += dc;
            if (r < 0 || r > 5 || c < 0 || c > 6)
              break;
          }
          if (dist == 4) {
            if (any_yellow ^ any_red) {
              if (any_yellow)
                --value;
              else
                ++value;
            }
          }
          //printf(" (dist%d) -> %d\n", dist, value);
        }
      }    
    }
  }
  return value;
}

bool MaxBot::FindBestMove(Board* b, Observer* o, CellContents disc,
                          int lookahead, int* out_column, int* out_value) {
  int extreme_value = 0;
  int extreme_column = 0;
  bool one_considered = false;
  bool is_min = disc == kYellowDisc;

  for (int col = 0; col < 7; ++col) {
    int row;
    int value;
    if (!b->Add(col, disc, &row))
      continue;
    //if (lookahead >= 0) {
    //  printf("%d: Trying column %d, disc %d:\n", lookahead, col, disc);
    //  printf("%s\n", b->ToString().c_str());
    //}
    bool just_won = b->FindMaxStreakAt(row, col) >= 4;
    if (just_won) {
      // This player just won, do not bother recursing.
      value = disc == kRedDisc ? kMaxHeuristic : -kMaxHeuristic;
    } else if (lookahead > 0) {
      int opponent_col;
      if (!FindBestMove(b, o, b->GetOpposite(disc), lookahead - 1,
                        &opponent_col, &value)) {
        // No available move, aka tie reached. So call this value 0. 
        value = 0;
      }
    } else {
      // lookahead = 0
      value = ComputeHeuristic(b);
    }
    b->UnAdd(col);
    //if (lookahead >= 0) printf("%d: value %d\n", lookahead, value);
    if (!one_considered ||
        (is_min && value < extreme_value) ||
        (!is_min && value > extreme_value)) {
      one_considered = true;
      extreme_value = value;
      extreme_column = col;
      //if (lookahead >= 0) printf("%d: new extreme c%d, %d\n", lookahead, col, value);
    }
    // If this player can win by this move, no other move can be better.
    if (just_won)
      break;
  }
  if (!one_considered) return false;

  *out_column = extreme_column;
  *out_value = extreme_value;
  return true;
}

void MaxBot::FindNextMove(Board* b, Observer* o) {
  int value;
  int column;
  if (!FindBestMove(b, o, my_disc_, lookahead_, &column, &value)) {
    state_.kind = PlayerBot::Observer::kNoMovePossible;
    o->Observe(&state_);
  } else {
    state_.kind = PlayerBot::Observer::kMoveDone;
    state_.column = column;
    state_.heuristic = value;
    o->Observe(&state_);
  }
}
