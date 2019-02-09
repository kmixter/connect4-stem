#ifndef _R2D2BOT_H
#define _R2D2BOT_H

#include "playerbot.h"

class PRNG;

class R2D2Bot : public PlayerBot {
 public:
  inline R2D2Bot(CellContents c, PRNG* prng) : PlayerBot("R2D2", c, prng) {}
  void FindNextMove(Board* board, Observer* o) override;
};

#endif  // _R2D2BOT_H