#ifndef _MAXBOT_H
#define _MAXBOT_H

#include "playerbot.h"

class MaxBot : public PlayerBot {
 public:
  static const int kMaxLookahead = 8;
  MaxBot(CellContents disc, int lookahead, PRNG* prng) 
    : PlayerBot("Superbot", disc, prng), lookahead_(lookahead) {}

  void FindNextMove(Board* board, Observer* o) override;

  int ComputeHeuristic(Board* b) const;

 private:
  bool FindBestMove(Board* b, Observer* o, CellContents disc,
                    int lookahead, int* out_column, int* out_value);
  int lookahead_;
  uint8_t moves_[kMaxLookahead];
  PlayerBot::Observer::State state_;
};

#endif  // _MAXBOT_H