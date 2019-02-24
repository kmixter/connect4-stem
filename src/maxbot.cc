#include "maxbot.h"

#include "prng.h"

// There are 69 4-streaks within a 7x6 connect4 board. Make sure that
// all streaks of 3 (which have highest weight below) does not overflow
// max heuristic. Also note that the depth will be subtracted from the
// max heuristic so that we favor moves that result in inevitable win/loss
// sooner (avoid taunting or delaying inevitable).
static const int kMaxHeuristic = 20000;
const int MaxBot::kStreakWeight[] = { 1, 10, 140, 0 };

// H(color) = all possible available wins for that color that involve
// at least one piece for that color on the board. Heuristic returned
// is H(red) - H(yellow).
 __attribute__((optimize("O2")))
int MaxBot::ComputeHeuristic(Board* b) const {
  int value = 0;
  for (int row = 0; row < 6; ++row) {
    for (int col = 0; col < 7; ++col) {
      for (int dr = 0; dr < 2; ++dr) {
        for (int dc = -1; dc < 2; ++dc) {
          if (dr == 0 && dc != 1) continue;
          // For each cell in the grid, for each of the 4 canonical directions:
          int yellow_count = 0;
          int red_count = 0;
          int r = row, c = col;
          //printf("(%d,%d) (%d,%d): ", row, col, dr, dc);
          int dist = 0;
          while (true) {
            //printf("%c", b->contents_[r][c] == kRedDisc ? 'R' : (b->contents_[r][c] == kYellowDisc ? 'Y' : '_'));
            if (b->contents_[r][c] == kRedDisc) {
              ++red_count;
              if (yellow_count) break;
            } else if (b->contents_[r][c] == kYellowDisc) {
              ++yellow_count;
              if (red_count) break;
            }
            ++dist;
            if (dist == 4) break;
            r += dr;
            c += dc;
            if (r < 0 || r > 5 || c < 0 || c > 6)
              break;
          }
          if (dist == 4) {
            if ((red_count > 0) != (yellow_count > 0)) {
              if (yellow_count)
                value -= kStreakWeight[yellow_count - 1];
              else
                value += kStreakWeight[red_count - 1];
            }
          }
          //printf(" (dist%d) -> %d\n", dist, value);
        }
      }    
    }
  }
  return value;
}

class UnAdder {
 public:
  UnAdder(Board* b, int column) : b_(b), column_(column) {}
  ~UnAdder() {
    b_->UnAdd(column_);
  }

 private:
  Board* b_;
  int column_;
};

bool MaxBot::FindBestMove(Board* b, Observer* o, CellContents disc,
                          int lookahead, int max_alternative,
                          int min_alternative, int* out_column, int* out_value) {
  *out_value = 0;
  *out_column = 0;
  bool one_considered = false;
  bool is_min = disc == kYellowDisc;

  uint8_t* move_pointer = &state_.moves[lookahead_ - lookahead];
  --lookahead;

  for (int col = 0; col < 7; ++col) {
    int row;
    int value;
    *move_pointer = col;
    if (!b->Add(col, disc, &row))
      continue;
    UnAdder unadder(b, col);

    //printf("%d: Trying column %d, disc %d:\n", lookahead, col, disc);
    //printf("%s\n", b->ToString().c_str());
    if (b->FindMaxStreakAt(row, col) >= 4) {
      // This player just won, do not bother recursing.
      *out_value = (disc == kRedDisc ? 1 : -1) *
        (kMaxHeuristic - (lookahead_ - lookahead));
      *out_column = col;
      state_.kind = PlayerBot::Observer::kHeuristicDone;
      state_.depth = lookahead_ - lookahead;
      state_.heuristic = *out_value;
      if (!o->Observe(&state_)) {
        return false;
      }
      return true;
    }

    if (lookahead > 0) {
      int opponent_col;
      if (!FindBestMove(b, o, b->GetOpposite(disc), lookahead,
                        max_alternative, min_alternative, &opponent_col,
                        &value)) {
        // No available move, aka tie reached. So call this value 0. 
        value = 0;
      }
    } else {
      // lookahead = 0
      value = ComputeHeuristic(b);
      state_.kind = PlayerBot::Observer::kHeuristicDone;
      //printf("Heuristic is %d\n", value);
      state_.depth = lookahead_ - lookahead;
      state_.heuristic = value;
      if (!o->Observe(&state_)) {
        interrupted_ = true;
        return false;
      }
    }

    //if (lookahead >= 0) printf("%d: value %d\n", lookahead, value);
    if (!one_considered ||
        (is_min && value < *out_value) ||
        (!is_min && value > *out_value)) {
      one_considered = true;
      *out_value = value;
      *out_column = col;
      //if (lookahead >= 0) printf("%d: new *out c%d, %d\n", lookahead, col, value);
    }

    if (use_alphabeta_) {
      bool prune = false;
      if (is_min) {
        if (value < min_alternative) {
          //printf("%d: min_alternative was %d now %d\n", lookahead,
          //       min_alternative, value);
          min_alternative = value;
        }
        // If this option results in a value less than what
        // maxifying player already has as an alternative,
        // prune this tree.
        prune = value < max_alternative;
      } else {
        if (value > max_alternative) {
          //printf("%d: max_alternative was %d now %d\n", lookahead,
          //       max_alternative, value);
          max_alternative = value;
        }
        // If this option results in a value greater than what
        // minifying player already has as an alternative,
        // prune this tree.
        prune = value > min_alternative;
      }
      //printf("!# alternative range: (%d,%d)\n", min_alternative, max_alternative);
      if (prune) {
        state_.kind = PlayerBot::Observer::kAlphaBetaPruneDone;
        state_.heuristic = value;
        if (!o->Observe(&state_)) {
          // Interruption during this observation, like any
          // other during this recursion
          interrupted_ = true;
          return false;
        }
        return true;
      }
    }
  }
  return one_considered;
}

void MaxBot::FindNextMove(Board* b, Observer* o) {
  int value;
  int column;
  if (lookahead_ < 1 || lookahead_ > kMaxLookahead)
    return;
  state_.moves = moves_;
  state_.depth = lookahead_;
  interrupted_ = false;
  int max_alternative = -kMaxHeuristic;  // aka alpha
  int min_alternative = kMaxHeuristic;   // aka beta
  if (!FindBestMove(b, o, my_disc_, lookahead_, max_alternative,
                    min_alternative, &column, &value)) {
    if (!interrupted_) {
      state_.kind = PlayerBot::Observer::kNoMovePossible;
      o->Observe(&state_);
    }
  } else {
    state_.kind = PlayerBot::Observer::kMoveDone;
    state_.column = column;
    state_.heuristic = value;
    o->Observe(&state_);
  }
}

int MaxBotConstantEvals::ComputeDepth(Board* b) {
  int branch_factor = 0;
  for (int i = 0; i < 7; ++i) {
    if (b->Add(i, kRedDisc)) {
      b->UnAdd(i);
      ++branch_factor;
    }
  }

  int predicted_evals = 1;
  int depth;
  for (depth = 1; depth <= kMaxLookahead; ++depth) {
    predicted_evals *= branch_factor;
    if (predicted_evals > max_evals_)
      break;
  }

  return depth - 1;
}

void MaxBotConstantEvals::FindNextMove(Board* b, Observer* o) {
  lookahead_ = ComputeDepth(b);
  MaxBot::FindNextMove(b, o);
}