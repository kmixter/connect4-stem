#ifndef _Rule3BOT_H
#define _Rule3BOT_H

#include "playerbot.h"

class PRNG;

class Rule3Bot : public PlayerBot {
 public:
  inline Rule3Bot(CellContents c, PRNG* prng) : PlayerBot("Trifoil", c, prng) {}
  void FindNextMove(Board* board, Observer* o) override;
};

#endif  // _Rule3BOT_H