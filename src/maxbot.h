#ifndef _MAXBOT_H
#define _MAXBOT_H

#include "playerbot.h"

class MaxBot : public PlayerBot {
 public:
  static const int kMaxLookahead = 11;
  static const int kStreakWeight[4];
  MaxBot(CellContents disc, int lookahead, PRNG* prng,
         bool use_alphabeta)
    : PlayerBot("Superbot", disc, prng), lookahead_(lookahead),
      use_alphabeta_(use_alphabeta) {}

  void FindNextMove(Board* board, Observer* o) override;

  int ComputeHeuristic(Board* b) const;

 protected:
  bool FindBestMove(Board* b, Observer* o, CellContents disc,
                    int lookahead, int max_alternative,
                    int min_alternative, int* out_column, int* out_value);
  int lookahead_;
  bool use_alphabeta_;
  uint8_t moves_[kMaxLookahead];
  bool interrupted_ = false;
  PlayerBot::Observer::State state_;
};

class MaxBotConstantEvals : public MaxBot {
 public:
  MaxBotConstantEvals(CellContents disc, int max_evals, PRNG* prng,
                      bool use_alphabeta)
    : MaxBot(disc, 1, prng, use_alphabeta), max_evals_(max_evals) {}

  void FindNextMove(Board* board, Observer* o) override;

  int ComputeDepth(Board* b);

 protected:
  int max_evals_;
};

#endif  // _MAXBOT_H