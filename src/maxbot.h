#ifndef _MAXBOT_H
#define _MAXBOT_H

#include "playerbot.h"

class MaxBot : public PlayerBot {
 public:
  MaxBot(CellContents disc, int lookahead, PRNG* prng) 
    : PlayerBot("Max", disc, prng), lookahead_(lookahead) {}

  void FindNextMove(Board* board, Observer* o) override;

  int ComputeHeuristic(Board* b) const;

 private:
  bool FindBestMove(Board* b, Observer* o, CellContents disc,
                    int lookahead, int* out_column, int* out_value);
  int lookahead_;
  PlayerBot::Observer::State state_;
};

#endif  // _MAXBOT_H