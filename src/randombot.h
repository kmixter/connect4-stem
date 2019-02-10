#ifndef _ROOMBABOT_H
#define _ROOMBABOT_H

#include "playerbot.h"

class RandomBot : public PlayerBot {
 public:
  RandomBot(CellContents disc, PRNG* prng) : PlayerBot("Cupcake", disc, prng) {}

  void FindNextMove(Board* board, Observer* o) override;
};

#endif  // _ROOMBABOT_H10
