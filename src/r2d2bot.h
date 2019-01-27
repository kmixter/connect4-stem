#ifndef _R2D2BOT_H
#define _R2D2BOT_H

#include "playerbot.h"

class R2D2Bot : public PlayerBot {
 public:
  inline R2D2Bot(CellContents c) : PlayerBot("R2D2", c) {}
  bool FindNextMove(Board* board, int* column) override;
};

#endif  // _R2D2BOT_H